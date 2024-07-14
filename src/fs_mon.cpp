// SPDX-License-Identifier: GPL-3.0-or-later

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <new>

#include <coreinit/filesystem.h>
#include <coreinit/filesystem_fsa.h>
#include <coreinit/messagequeue.h>

#include <wups.h>

#include "fs_mon.hpp"
#include "logging.hpp"


namespace fs_mon {

    std::atomic_uint bytes_read = 0;
    std::atomic_uint shim_bytes_read = 0;


    void
    initialize()
    {
        bytes_read = 0;
        shim_bytes_read = 0;
    }


    void
    finalize()
    {}


    const char*
    get_report(float dt)
    {
        static char buf[64];

        const unsigned read = std::atomic_exchange(&bytes_read, 0u);
        const float read_rate = read / (1024.0f * 1024.0f) / dt;

        const unsigned shim_read = std::atomic_exchange(&shim_bytes_read, 0u);
        const float shim_read_rate = shim_read / (1024.0f * 1024.0f) / dt;


        std::snprintf(buf, sizeof buf,
                      "read %.1f MiB/s | shim_read %.1f MiB/s",
                      read_rate,
                      shim_read_rate);
        return buf;
    }


} // namespace fs_mon


DECL_FUNCTION(FSStatus, FSReadFile,
              FSCmdBlock *block,
              uint8_t *buffer,
              uint32_t size,
              uint32_t count,
              FSFileHandle handle,
              uint32_t unk1,
              FSErrorFlag errorMask)
{
    FSStatus result = real_FSReadFile(block, buffer, size, count, handle, unk1, errorMask);
    if (result == FS_STATUS_OK)
        fs_mon::bytes_read += size * count;
    return result;
}


DECL_FUNCTION(FSStatus, FSReadFileAsync,
              FSClient* client,
              FSCmdBlock* block,
              std::uint8_t* buffer,
              std::uint32_t size,
              std::uint32_t count,
              FSFileHandle handle,
              std::uint32_t unk1,
              FSErrorFlag errorMask,
              FSAsyncData* data)
{
    // sloppy measuring, we assume the read will succeed
    fs_mon::bytes_read += size * count;
    return real_FSReadFileAsync(client, block,
                                buffer, size, count,
                                handle, unk1, errorMask,
                                data);
}


DECL_FUNCTION(FSStatus, FSReadFileWithPos,
              FSClient* client,
              FSCmdBlock* block,
              uint8_t* buffer,
              uint32_t size,
              uint32_t count,
              uint32_t pos,
              FSFileHandle handle,
              uint32_t unk1,
              FSErrorFlag errorMask)
{
    FSStatus result = real_FSReadFileWithPos(client, block,
                                             buffer, size, count, pos,
                                             handle, unk1, errorMask);
    if (result == FS_STATUS_OK)
        fs_mon::bytes_read += size * count;
    return result;
}


DECL_FUNCTION(FSStatus, FSReadFileWithPosAsync,
              FSClient* client,
              FSCmdBlock* block,
              uint8_t* buffer,
              uint32_t size,
              uint32_t count,
              uint32_t pos,
              FSFileHandle handle,
              uint32_t unk1,
              FSErrorFlag errorMask,
              FSAsyncData* asyncData)
{
    // sloppy measuring, we assume the read will succeed
    fs_mon::bytes_read += size * count;
    return real_FSReadFileWithPosAsync(client, block,
                                       buffer, size, count, pos,
                                       handle, unk1, errorMask, asyncData);
}


DECL_FUNCTION(FSError, FSAReadFile,
              FSAClientHandle client,
              void* buffer,
              uint32_t size,
              uint32_t count,
              FSAFileHandle handle,
              uint32_t flags)
{
    FSError result = real_FSAReadFile(client, buffer, size, count, handle, flags);
    if (result == FS_ERROR_OK)
        fs_mon::bytes_read += size * count;
    return result;
}


DECL_FUNCTION(FSError, FSAReadFileWithPos,
              FSAClientHandle client,
              void* buffer,
              uint32_t size,
              uint32_t count,
              uint32_t pos,
              FSAFileHandle handle,
              uint32_t flags)
{
    FSError result = real_FSAReadFileWithPos(client, buffer, size, count, pos, handle, flags);
    if (result == FS_ERROR_OK)
        fs_mon::bytes_read += size * count;
    return result;
}


DECL_FUNCTION(FSError, FSAReadFileAsync,
              FSAClientHandle client,
              void* buffer,
              uint32_t size,
              uint32_t count,
              FSAFileHandle handle,
              uint32_t flags,
              void* asyncData)
{
    // sloppy measuring, we assume the read will succeed
    fs_mon::bytes_read += size * count;
    return real_FSAReadFileAsync(client, buffer, size, count, handle, flags, asyncData);
}


DECL_FUNCTION(FSError, FSAReadFileWithPosAsync,
              FSAClientHandle client,
              void* buffer,
              uint32_t size,
              uint32_t count,
              uint32_t pos,
              FSAFileHandle handle,
              uint32_t flags,
              void* asyncData)
{
    // sloppy measuring, we assume the read will succeed
    fs_mon::bytes_read += size * count;
    return real_FSAReadFileWithPosAsync(client,
                                        buffer, size, count, pos,
                                        handle, flags, asyncData);
}


WUPS_MUST_REPLACE(FSReadFile,             WUPS_LOADER_LIBRARY_COREINIT, FSReadFile);
WUPS_MUST_REPLACE(FSReadFileAsync,        WUPS_LOADER_LIBRARY_COREINIT, FSReadFileAsync);
WUPS_MUST_REPLACE(FSReadFileWithPos,      WUPS_LOADER_LIBRARY_COREINIT, FSReadFileWithPos);
WUPS_MUST_REPLACE(FSReadFileWithPosAsync, WUPS_LOADER_LIBRARY_COREINIT, FSReadFileWithPosAsync);

WUPS_MUST_REPLACE(FSAReadFile,             WUPS_LOADER_LIBRARY_COREINIT, FSAReadFile);
WUPS_MUST_REPLACE(FSAReadFileAsync,        WUPS_LOADER_LIBRARY_COREINIT, FSAReadFileAsync);
WUPS_MUST_REPLACE(FSAReadFileWithPos,      WUPS_LOADER_LIBRARY_COREINIT, FSAReadFileWithPos);
WUPS_MUST_REPLACE(FSAReadFileWithPosAsync, WUPS_LOADER_LIBRARY_COREINIT, FSAReadFileWithPosAsync);



DECL_FUNCTION(FSError, fsaShimPrepareRequestReadFile,
              FSAShimBuffer* shim,
              IOSHandle client,
              uint8_t* buffer,
              uint32_t size,
              uint32_t count,
              FSAFilePosition pos,
              FSAFileHandle handle,
              FSAReadFlag flags)
{
    // sloppy measuring, we assume the read will succeed
    fs_mon::shim_bytes_read += size * count;
    return real_fsaShimPrepareRequestReadFile(shim, client,
                                              buffer, size, count, pos,
                                              handle, flags);
}


WUPS_MUST_REPLACE_PHYSICAL(fsaShimPrepareRequestReadFile,
                           (0x020436cc + 0x3001c400),
                           (0x020436cc - 0xfe3c00));
