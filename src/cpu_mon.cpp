/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2025  Daniel K. O.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*
 * CPU Monitoring
 *
 * In this file we take advantage of the leftover "CafeOS Shell" functions left behind
 * inside retail coreinit.
 *
 * For ARM utilization we use the bspRead() function.
 */

#include <atomic>
#include <cstdio>
#include <cstring>
#include <new>

#include <coreinit/bsp.h>
#include <coreinit/debug.h>
#include <coreinit/ios.h>

#include "cpu_mon.hpp"

#include "cfg.hpp"
#include "utils.hpp"


using std::uint32_t;
using std::size_t;


#define AVOID_ALLOCATIONS

// Define this if you want more accurate ARM CPU usage, but might reduce frame rate.
//#define SYNCHRONOUS_ARM_STAT

namespace my {

    struct BSPReadRequest {
        char     entity[32];
        uint32_t instance;
        char     attribute[32];
        size_t   size;

        BSPReadRequest(const char* entity_,
                       std::uint32_t instance_,
                       const char* attribute_,
                       std::size_t size_)
            noexcept
        {
            // std::memset(this, 0, sizeof *this);
            std::strncpy(entity, entity_, sizeof entity);
            instance = instance_;
            std::strncpy(attribute, attribute_, sizeof attribute);
            size = size_;
        }
    };
    static_assert(sizeof(BSPReadRequest) == 0x48);


    struct BSPReadResponse {
        char data[512];
    };


    enum BSPCommand {
        BSP_CMD_READ = 5,
    };


    auto bsp_handle_ptr = reinterpret_cast<const int*>(0x1004f920);


    /*
     * Note: instance argument seems to be 32 bit argument, not 8 bit.
     * This version is more efficient than the coreinit bspRead().
     */
    BSPError
    bspRead(const char* entity,
            uint32_t instance,
            const char* attribute,
            size_t size,
            void* output)
    {
        if (size > sizeof BSPReadResponse::data)
            return BSP_ERROR_RESPONSE_TOO_LARGE;

        alignas(0x20)
        BSPReadResponse response;

        alignas(0x20)
        BSPReadRequest request{
            entity,
            instance,
            attribute,
            size
        };

        auto status = IOS_Ioctl(*bsp_handle_ptr,
                                BSP_CMD_READ,
                                &request, sizeof request,
                                response.data, size);
        if (status < 0) {
            OSReport("bspRead(): IOS_Ioctl() returned %d\n", status);
            return BSP_ERROR_IOS_ERROR;
        }
        std::memcpy(output, response.data, size);
        return BSP_ERROR_OK;
    }


    using BSPReadAsyncCallbackFn = void (*)(BSPError, void*);


    struct BSPReadAsyncContext {
        alignas(0x20)
        BSPReadResponse response;

        alignas(0x20)
        BSPReadRequest request;

        void* output;
        BSPReadAsyncCallbackFn callback;
        void* context;

        BSPReadAsyncContext(const char* entity_,
                            uint32_t instance_,
                            const char* attribute_,
                            size_t size_,
                            void* output_,
                            BSPReadAsyncCallbackFn callback_,
                            void* context_)
            noexcept:
            // intentionally don't initialize response
            request{entity_, instance_, attribute_, size_},
            output{output_},
            callback{callback_},
            context{context_}
        {}
    };


    static
    void
    bspReadAsyncCompleted(IOSError status, void* context)
    {
        auto read_ctx = reinterpret_cast<BSPReadAsyncContext*>(context);
        BSPError bsp_status = status < 0 ? BSP_ERROR_IOS_ERROR : BSP_ERROR_OK;
        if (!bsp_status)
            memcpy(read_ctx->output,
                   read_ctx->response.data,
                   read_ctx->request.size);
        read_ctx->callback(bsp_status, read_ctx->context);
#ifndef AVOID_ALLOCATIONS
        delete read_ctx;
#endif
    }


#ifdef AVOID_ALLOCATIONS
    alignas(BSPReadAsyncContext)
    char read_async_buf[sizeof(BSPReadAsyncContext)] ;
#endif

    BSPError
    bspReadAsync(const char* entity,
                 uint32_t instance,
                 const char* attribute,
                 size_t size,
                 void* output,
                 BSPReadAsyncCallbackFn callback,
                 void* context)
    {
        if (size > 512)
            return BSP_ERROR_RESPONSE_TOO_LARGE;
#ifdef AVOID_ALLOCATIONS
        auto read_ctx = new(read_async_buf) BSPReadAsyncContext{
#else
        auto read_ctx = new(std::nothrow) BSPReadAsyncContext{
#endif
            entity, instance, attribute, size,
            output,
            callback,
            context
        };
        if (!read_ctx)
            return BSP_ERROR_IOS_ERROR;

        IOSError status = IOS_IoctlAsync(*bsp_handle_ptr,
                                         BSP_CMD_READ,
                                         &read_ctx->request,
                                         sizeof read_ctx->request,
                                         &read_ctx->response,
                                         size,
                                         bspReadAsyncCompleted,
                                         read_ctx);
        if (status < 0) {
#ifndef AVOID_ALLOCATIONS
            delete read_ctx;
#endif
            OSReport("bspReadAsync(): IOS_IoctlAsync() returned %d\n", status);
            return BSP_ERROR_IOS_ERROR;
        }
        return BSP_ERROR_OK;
    }
} // namespace my


namespace cpu_mon {

    using get_ppc_utilization_ptr = float (*)(unsigned);
    const get_ppc_utilization_ptr get_ppc_utilization =
        reinterpret_cast<get_ppc_utilization_ptr>(0x020298d4 - 0xfe3c00);

#ifdef SYNCHRONOUS_ARM_STAT

    // This is a synchronous call, will reduce frame rate under high IOS activity.
    float
    get_arm_utilization()
    {
        float result = 0;
        std::uint32_t val = 0;
        // Seems to be a value between 0 and 1000.
        auto err = my::bspRead("Sys", 0, "cpuUtil", sizeof val, &val);
        if (!err)
            result = val / 10.0;

        return result;
    }

#else

    alignas(0x20)
    std::atomic_uint last_arm_value = 0xffffffff;

    uint32_t arm_result = 0; // used by the async read

    alignas(0x20)
    std::atomic_bool read_pending = false;


    void
    got_arm_utilization(BSPError error, void*)
    {
        if (!error)
            last_arm_value.store(arm_result, std::memory_order::relaxed);
        else
            OSReport("bspReadAsync resulted in error failed\n");
        read_pending.store(false, std::memory_order::release);
    }


    float
    get_arm_utilization()
    {
        if (!read_pending.load(std::memory_order::acquire)) {
            // request new async read
            read_pending.store(true, std::memory_order::release);
            auto err = my::bspReadAsync("Sys", 0, "cpuUtil",
                                        sizeof arm_result, &arm_result,
                                        got_arm_utilization,
                                        nullptr);
            if (err)
                read_pending.store(false, std::memory_order::release);
        }

        float result = 0;
        unsigned val = last_arm_value.load(std::memory_order::relaxed);
        if (val != 0xffffffff)
            result = val / 10.0; // Seems to be a value between 0 and 1000.

        return result;
    }

#endif


    void
    initialize()
    {}


    void
    finalize()
    {}


    void
    reset()
    {}


    void
    get_report(out_span& out,
               float)
    {
        float c0 = get_ppc_utilization(0);
        float c1 = get_ppc_utilization(1);
        float c2 = get_ppc_utilization(2);

        if (cfg::cpu_busy_percent.value) {

            out.printf("PPC0: %2.1f%%  PPC1: %2.1f%%  PPC2: %2.1f%%", c0, c1, c2);
            if (cfg::cpu_busy_arm.value) {
                float c3 = get_arm_utilization();
                out.printf("  ARM: %2.1f%%", c3);
            }

        } else {

            using utils::percent_to_bar;
            out.printf("CPU: %s %s %s",
                       percent_to_bar(c0),
                       percent_to_bar(c1),
                       percent_to_bar(c2));
            if (cfg::cpu_busy_arm.value) {
                float c3 = get_arm_utilization();
                out.printf(" %s", percent_to_bar(c3));
            }

        }
    }

} // namespace cpu_mon
