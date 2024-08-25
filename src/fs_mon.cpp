/*
 * Papaya-HUD - a HUD plugin for Aroma.
 *
 * Copyright (C) 2024  Daniel K. O.
 * Copyright (C) 2024  Maschell
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*
 * Filesystem Monitoring
 *
 * While coreinit does have vestigial functions to monitor I/O, they're disabled on retail
 * Wii U, and only return error status.
 *
 * So we do it in a low-level way, by hooking into two internal functions (credits to
 * Maschell for these), at the "shim" layer; all higher-level I/O functions are redirected
 * to those two functions.
 *
 * Note that we can't really track I/O that happens under the apps (e.g. kernel, IOSU). If
 * you try moving a game between NAND and USB from the system settings, this code can't
 * see the I/O happening.
 */

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <new>

#include <coreinit/filesystem_fsa.h>

#include <wups.h>

#include "fs_mon.hpp"


namespace fs_mon {

    std::atomic_uint bytes_read = 0;


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
        bytes_read = 0;
    }


    const char*
    get_report(float dt)
    {
        static char buf[64];

        unsigned read = std::atomic_exchange(&bytes_read, 0u);
        float read_rate = read / (1024.0f * 1024.0f) / dt;

        std::snprintf(buf, sizeof buf,
                      "RD: %.1f MiB/s",
                      read_rate);
        return buf;
    }


    // Code below was suggested by Maschell, with some modifications.

    void
    update_stats(FSAShimBuffer* shim, int res)
    {
        if (res < 0)
            return;

        // TODO: we can track more stuff in here, right now there isn't much screen space
        // in the HUD for it all.
        switch (shim->command) {
        case FSA_COMMAND_READ_FILE:
            bytes_read += shim->request.readFile.size * res;
            break;
        case FSA_COMMAND_RAW_READ:
            bytes_read += shim->request.rawRead.size * res;
            break;
        // case FSA_COMMAND_WRITE_FILE:
        //     bytes_written += shim->request.writeFile.size * res;
        //     break;
        // case FSA_COMMAND_RAW_WRITE:
        //     bytes_written += shim->request.rawWrite.size * res;
        //     break;
        }
    }


    struct ContextWrapper {
        IOSAsyncCallbackFn realCallback;
        void*              realContext;
        FSAShimBuffer*     shim;
    };


    void
    async_callback(IOSError result, void* context)
    {
        auto wrapper = static_cast<ContextWrapper*>(context);
        update_stats(wrapper->shim,
                     __FSAShimDecodeIosErrorToFsaStatus(wrapper->shim->clientHandle,
                                                        result));
        if (wrapper->realCallback)
            wrapper->realCallback(result, wrapper->realContext);

        delete wrapper;
    }


    DECL_FUNCTION(int, fsaShimSubmitRequest,
                  FSAShimBuffer* shim,
                  FSError emulatedError)
    {
        auto res = real_fsaShimSubmitRequest(shim, emulatedError);
        update_stats(shim, res);
        return res;
    }

    WUPS_MUST_REPLACE_PHYSICAL(fsaShimSubmitRequest,
                               (0x02042d90 + 0x3001c400),
                               (0x02042d90 - 0xfe3c00));


    DECL_FUNCTION(FSError, fsaShimSubmitRequestAsync,
                  FSAShimBuffer* shim,
                  FSError emulatedError,
                  IOSAsyncCallbackFn callback,
                  void* context)
    {
        switch (shim->command) {
        case FSA_COMMAND_READ_FILE:
        case FSA_COMMAND_RAW_READ:
            {
                auto wrapper = new(std::nothrow) ContextWrapper{
                    .realCallback = callback,
                    .realContext = context,
                    .shim = shim
                };
                if (!wrapper)
                    break; // fall back to original callback and context

                auto result = real_fsaShimSubmitRequestAsync(shim, emulatedError,
                                                             async_callback, wrapper);
                if (result != FS_ERROR_OK) {
                    delete wrapper;
                    break; // fall back to original callback and context
                }

                return result;
            }
        }

        return real_fsaShimSubmitRequestAsync(shim, emulatedError, callback, context);
    }

    WUPS_MUST_REPLACE_PHYSICAL(fsaShimSubmitRequestAsync,
                               (0x02042e84 + 0x3001c400),
                               (0x02042e84 - 0xfe3c00));

} // namespace fs_mon
