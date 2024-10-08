// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COREINIT_ALLOCATOR_H
#define COREINIT_ALLOCATOR_H

#include <stdint.h>

#include <wut.h>
#include <coreinit/memheap.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct MEMAllocatorFunctions MEMAllocatorFunctions;


typedef struct MEMAllocator {
    MEMAllocatorFunctions* funcs;
    MEMHeapHandle          heap;
    uint32_t               arg1;
    uint32_t               arg2;
} MEMAllocator;
WUT_CHECK_SIZE(MEMAllocator, 16);



typedef void* (*MEMAllocatorAlloc)(MEMAllocator* allocator, uint32_t size);
typedef void (*MEMAllocatorFree)(MEMAllocator* allocator, void* ptr);


struct MEMAllocatorFunctions {
    MEMAllocatorAlloc alloc;
    MEMAllocatorFree  free;
};
WUT_CHECK_SIZE(MEMAllocatorFunctions, 8);


void* MEMAllocFromAllocator(MEMAllocator* allocator, uint32_t size);
void MEMFreeToAllocator(MEMAllocator* allocator, void* ptr);


void MEMInitAllocatorForExpHeap(MEMAllocator* allocator,
                                MEMHeapHandle heap,
                                uint32_t alignment);


void MEMInitAllocatorForFrmHeap(MEMAllocator* allocator,
                                MEMHeapHandle heap,
                                uint32_t alignment);


void MEMInitAllocatorForUnitHeap(MEMAllocator* allocator, MEMHeapHandle heap);


void MEMInitAllocatorForDefaultHeap(MEMAllocator* allocator);


void MEMInitAllocatorForBlockHeap(MEMAllocator* allocator,
                                  MEMHeapHandle heap,
                                  uint32_t alignment);

#ifdef __cplusplus
}
#endif

#endif
