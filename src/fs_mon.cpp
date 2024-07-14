// SPDX-License-Identifier: GPL-3.0-or-later

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <new>

#include <coreinit/filesystem_fsa.h>

#include <wups.h>

#include "fs_mon.hpp"
#include "logging.hpp"


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
                      "in %.1f MiB/s",
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
            fs_mon::bytes_read += shim->request.readFile.size * res;
            break;
        case FSA_COMMAND_RAW_READ:
            fs_mon::bytes_read += shim->request.rawRead.size * res;
            break;
        // case FSA_COMMAND_WRITE_FILE:
        //     fs_mon::bytes_written += shim->request.writeFile.size * res;
        //     break;
        // case FSA_COMMAND_RAW_WRITE:
        //     fs_mon::bytes_written += shim->request.rawWrite.size * res;
        //     break;
        }
    }


    struct ContextWrapper {
        IOSAsyncCallbackFn realCallback;
        void* realContext;
        FSAShimBuffer* shim;
    };


    void
    async_callback(IOSError result, void* context)
    {
        auto wrapper = static_cast<ContextWrapper*>(context);
        fs_mon::update_stats(wrapper->shim,
                             __FSAShimDecodeIosErrorToFsaStatus(wrapper->shim->clientHandle,
                                                                result));
        if (wrapper->realCallback)
            wrapper->realCallback(result, wrapper->realContext);

        delete wrapper;
    }


} // namespace fs_mon



DECL_FUNCTION(int, fsaShimSubmitRequest,
              FSAShimBuffer* shim,
              FSError emulatedError)
{
    auto res = real_fsaShimSubmitRequest(shim, emulatedError);
    fs_mon::update_stats(shim, res);
    return res;
}


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
            auto wrapper = new(std::nothrow) fs_mon::ContextWrapper{
                .realCallback = callback,
                .realContext = context,
                .shim = shim
            };
            if (!wrapper)
                break; // fall back to original callback and context

            auto result = real_fsaShimSubmitRequestAsync(shim, emulatedError,
                                                         fs_mon::async_callback, wrapper);
            if (result != FS_ERROR_OK) {
                delete wrapper;
                break; // fall back to original callback and context
            }

            return result;
        }
    }

    return real_fsaShimSubmitRequestAsync(shim, emulatedError, callback, context);
}


WUPS_MUST_REPLACE_PHYSICAL(fsaShimSubmitRequest,
                           (0x02042d90 + 0x3001c400),
                           (0x02042d90 - 0xfe3c00));

WUPS_MUST_REPLACE_PHYSICAL(fsaShimSubmitRequestAsync,
                           (0x02042e84 + 0x3001c400),
                           (0x02042e84 - 0xfe3c00));
