/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2025  Daniel K. O.
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
#include "utils.hpp"


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


    void
    get_report(out_span& out,
               float dt)
    {
        const char* separator = "";

        out.append("net: ");

        if (cfg::net_cfg.value) {
            int res;
            NetConfCfg cfg{};
            res = netconf_init();
            if (!res) {
                res = netconf_get_running(&cfg);
                if (!res) {
                    if (cfg.wl0.if_state) {
                        out.append("wifi \"");
                        out.append(cfg.wifi.config.ssid,
                                   cfg.wifi.config.ssidlength);
                        out.append("\"");
                    } else if (cfg.eth0.if_state)
                        out.append("eth");
                    else // unlikely scenario when neither wl0 nor eth0 are enabled
                        out.append("offline");
                } else {
                    // when netconf_get_running() fails
                    out.append("offline");
                }
                netconf_close();
            } else {
                // when netconf_init() fails
                out.append("offline");
            }
            separator = " ";
        }

        if (cfg::net_bw.value) {
            using utils::format_bytes;

            float down_rate = std::atomic_exchange(&bytes_received, 0u) / dt;
            float up_rate = std::atomic_exchange(&bytes_sent, 0u) / dt;

            const char* symbol = "\u3000"; // blank space

            if (cfg::net_bw_combined.value) {
                if (down_rate > 0 && up_rate > 0)
                    symbol = "⇅";
                else if (down_rate > 0)
                    symbol = "↓";
                else if (up_rate > 0)
                    symbol = "↑";

                out.printf("%s%s", separator, symbol);
                format_bytes(out, down_rate + up_rate);
                out.append("/s");
            } else {
                out.append(separator);
                out.append("↓");
                format_bytes(out, down_rate);
                out.append("/s↑");
                format_bytes(out, up_rate);
                out.append("/s");
            }
        }

    }

} // namespace net_mon


DECL_FUNCTION(int, recv,
              int fd,
              void* buf,
              int len,
              int flags)
{
    int result = real_recv(fd, buf, len, flags);
    if (result != -1 && cfg::net_bw.value)
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
    if (result != -1 && cfg::net_bw.value)
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
    if (result != -1 && cfg::net_bw.value)
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
    if (result != -1 && cfg::net_bw.value)
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
    if (result != -1 && cfg::net_bw.value)
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
    if (result != -1 && cfg::net_bw.value)
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
    if (result != -1 && cfg::net_bw.value)
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
    if (result != -1 && cfg::net_bw.value)
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
