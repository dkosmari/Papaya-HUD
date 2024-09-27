/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2024  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*
 * Network Monitoring
 *
 * One of the simplest modules, we just hook into send and recv functions from nsysnet.
 */

#include <algorithm>
#include <atomic>
#include <cstdio>

#include <nsysnet/netconfig.h>
#include <sys/socket.h>         // struct sockaddr
#include <wups.h>

#include "net_mon.hpp"

#include "cfg.hpp"


using namespace std::literals;


namespace {

    std::string
    eth_speed_to_string(int spd)
    {
        return std::to_string(spd) + " Mbps";
    }

    std::string
    eth_duplex_to_string(int dup)
    {
        switch (dup) {
        case NET_CONF_ETH_CFG_DUPLEX_HALF:
            return "Half";
        case NET_CONF_ETH_CFG_DUPLEX_FULL:
            return "Full";
        default:
            return "?";
        }
    }


    std::string
    get_ssid(const NetConfWifiConfigData& cfg)
    {
        auto len = std::min<std::size_t>(cfg.ssidlength, sizeof cfg.ssid);
        return std::string(reinterpret_cast<const char*>(cfg.ssid), len);
    }


} // namespace


namespace net_mon {

    std::atomic_uint bytes_received = 0;
    std::atomic_uint bytes_sent = 0;


    void
    initialize()
    {
        reset();
    }


    void
    finalize()
    {}


    void
    reset()
    {
        bytes_received = 0;
        bytes_sent = 0;
    }


    const char*
    get_report(float dt)
    {
        std::string net_stat;
        if (cfg::net_cfg) {
            net_stat = "offline";
            int res;
            NetConfCfg cfg{};
            res = netconf_init();
            if (!res) {
                res = netconf_get_running(&cfg);
                if (!res) {
                    if (cfg.wl0.if_sate) {
                        net_stat = "wifi \""s
                            + get_ssid(cfg.wifi.config)
                            + "\""s;
                    } else if (cfg.eth0.if_sate) {
                        net_stat = "eth "s
                            + eth_speed_to_string(cfg.ethCfg.speed)
                            + " "s
                            + eth_duplex_to_string(cfg.ethCfg.duplex);
                    }
                }
            }
            netconf_close();
        }

        std::string speed_stat;

        if (cfg::net_bw) {

            const unsigned down = std::atomic_exchange(&bytes_received, 0u);
            const unsigned up = std::atomic_exchange(&bytes_sent, 0u);

            const float down_rate = down / 1024.0f / dt;
            const float up_rate = up / 1024.0f / dt;

            static char speed_buf[64];
            std::snprintf(speed_buf, sizeof speed_buf,
                          "↓ %.1f KiB/s "
                          "↑ %.1f KiB/s",
                          down_rate,
                          up_rate);
            speed_stat = speed_buf;
        }

        const char* sep = net_stat.empty() || speed_stat.empty()
                          ? ""
                          : " ";

        static char buf[128];

        std::snprintf(buf, sizeof buf,
                      "%s%s%s",
                      net_stat.c_str(),
                      sep,
                      speed_stat.c_str());
        return buf;
    }

} // namespace net_mon


DECL_FUNCTION(int, recv,
              int fd,
              void* buf,
              int len,
              int flags)
{
    int result = real_recv(fd, buf, len, flags);
    if (result != -1 && cfg::net_bw)
        net_mon::bytes_received += result;
    return result;
}


DECL_FUNCTION(int, recvfrom,
              int fd,
              void* buf,
              int len,
              int flags,
              struct sockaddr* src,
              int* src_len)
{
    int result = real_recvfrom(fd, buf, len, flags, src, src_len);
    if (result != -1 && cfg::net_bw)
        net_mon::bytes_received += result;
    return result;
}


DECL_FUNCTION(int, recvfrom_ex,
              int fd,
              void* buf,
              int len,
              int flags,
              struct sockaddr* src,
              int* src_len,
              void* msg,
              int msg_len)
{
    int result = real_recvfrom_ex(fd, buf, len, flags, src, src_len, msg, msg_len);
    if (result != -1 && cfg::net_bw)
        net_mon::bytes_received += result;
    return result;
}


DECL_FUNCTION(int, recvfrom_multi,
              int fd,
              int flags,
              void* buffs,
              int data_len,
              int data_count,
              struct timeval* timeout)
{
    int result = real_recvfrom_multi(fd, flags, buffs, data_len, data_count, timeout);
    if (result != -1 && cfg::net_bw)
        net_mon::bytes_received += result;
    return result;
}


DECL_FUNCTION(int, send,
              int fd,
              const void* buf,
              int len,
              int flags)
{
    int result = real_send(fd, buf, len, flags);
    if (result != -1 && cfg::net_bw)
        net_mon::bytes_sent += result;
    return result;
}


DECL_FUNCTION(int, sendto,
              int fd,
              const void* buf,
              int len,
              int flags,
              const struct sockaddr* dst,
              int dst_len)
{
    int result = real_sendto(fd, buf, len, flags, dst, dst_len);
    if (result != -1 && cfg::net_bw)
        net_mon::bytes_sent += result;
    return result;
}


DECL_FUNCTION(int, sendto_multi,
              int fd,
              const void *buf,
              int len,
              int flags,
              const struct sockaddr* dstv,
              int dstv_len)
{
    int result = real_sendto_multi(fd, buf, len, flags, dstv, dstv_len);
    if (result != -1 && cfg::net_bw)
        net_mon::bytes_sent += result;
    return result;
}


DECL_FUNCTION(int, sendto_multi_ex,
              int fd,
              int flags,
              void* buffs,
              int count)
{
    int result = real_sendto_multi_ex(fd, flags, buffs, count);
    if (result != -1 && cfg::net_bw)
        net_mon::bytes_sent += result;
    return result;
}


WUPS_MUST_REPLACE(recv,           WUPS_LOADER_LIBRARY_NSYSNET, recv);
WUPS_MUST_REPLACE(recvfrom,       WUPS_LOADER_LIBRARY_NSYSNET, recvfrom);
WUPS_MUST_REPLACE(recvfrom_ex,    WUPS_LOADER_LIBRARY_NSYSNET, recvfrom_ex);
WUPS_MUST_REPLACE(recvfrom_multi, WUPS_LOADER_LIBRARY_NSYSNET, recvfrom_multi);

WUPS_MUST_REPLACE(send,            WUPS_LOADER_LIBRARY_NSYSNET, send);
WUPS_MUST_REPLACE(sendto,          WUPS_LOADER_LIBRARY_NSYSNET, sendto);
WUPS_MUST_REPLACE(sendto_multi,    WUPS_LOADER_LIBRARY_NSYSNET, sendto_multi);
WUPS_MUST_REPLACE(sendto_multi_ex, WUPS_LOADER_LIBRARY_NSYSNET, sendto_multi_ex);
