// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GX2_PERF_H
#define GX2_PERF_H

#include <stdint.h>
#include <wut.h>

#include "coreinit_allocator.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef enum GX2PerfCollectionMethod {
    GX2_PERF_COLLECT_NONE            = 0,
    GX2_PERF_COLLECT_TAGS_ACCUMULATE = 1,
    GX2_PERF_COLLECT_TAGS_INDIVIDUAL = 2,
} GX2PerfCollectionMethod;


typedef enum GX2PerfType {
    GX2_PERF_TYPE_GPU_METRIC = 0,
    GX2_PERF_TYPE_GPU_STAT   = 1,
    GX2_PERF_TYPE_MEM_STAT   = 2,
} GX2PerfType;


typedef enum GX2PerfMetric {
    GX2_PERF_U64_TIME                     = 0x00,
    GX2_PERF_U64_GPU_TIME                 = 0x01,
    GX2_PERF_F32_GPU_BUSY                 = 0x02,
    GX2_PERF_F32_SHADER_BUSY              = 0x03,
    GX2_PERF_U64_REUSED_INDICES_VS        = 0x04,
    GX2_PERF_F32_SHADER_BUSY_VS           = 0x05,
    GX2_PERF_F32_SHADER_BUSY_GS           = 0x06,
    GX2_PERF_F32_SHADER_BUSY_PS           = 0x07,
    GX2_PERF_F32_ALU_BUSY                 = 0x08,
    GX2_PERF_F32_TEX_BUSY                 = 0x09,
    GX2_PERF_U64_VS_VERTICES_IN           = 0x0a,
    GX2_PERF_F32_VS_TEX_INST_COUNT        = 0x0b,
    GX2_PERF_F32_VS_TEX_BUSY              = 0x0c,
    GX2_PERF_F32_VS_ALU_INST_COUNT        = 0x0d,
    GX2_PERF_F32_VS_ALU_BUSY              = 0x0e,
    GX2_PERF_F32_VS_ALU_EFFICIENCY        = 0x0f,
    GX2_PERF_F32_VS_ALU_TEX_RATIO         = 0x10,
    GX2_PERF_F32_GS_TEX_INST_COUNT        = 0x11,
    GX2_PERF_F32_GS_TEX_BUSY              = 0x12,
    GX2_PERF_F32_GS_ALU_INST_COUNT        = 0x13,
    GX2_PERF_F32_GS_ALU_BUSY              = 0x14,
    GX2_PERF_F32_GS_ALU_EFFICIENCY        = 0x15,
    GX2_PERF_F32_GS_ALU_TEX_RATIO         = 0x16,
    GX2_PERF_F32_PRIMITIVE_ASSEMBLY_BUSY  = 0x17,
    GX2_PERF_U64_PRIMITIVES_IN            = 0x18,
    GX2_PERF_F32_PA_STALLED_ON_RASTERIZER = 0x19,
    GX2_PERF_F32_INTERP_BUSY              = 0x1a,
    GX2_PERF_U64_PS_PIXELS_IN             = 0x1b,
    GX2_PERF_F32_PS_TEX_INST_COUNT        = 0x1c,
    GX2_PERF_F32_PS_TEX_BUSY              = 0x1d,
    GX2_PERF_F32_PS_ALU_INST_COUNT        = 0x1e,
    GX2_PERF_F32_PS_ALU_BUSY              = 0x1f,
    GX2_PERF_F32_PS_ALU_EFFICIENCY        = 0x20,
    GX2_PERF_F32_PS_ALU_TEX_RATIO         = 0x21,
    GX2_PERF_U64_PS_PIXELS_OUT            = 0x22,
    GX2_PERF_F32_PS_EXPORT_STALLS         = 0x23,
    GX2_PERF_F32_TEX_UNIT_BUSY            = 0x24,
    GX2_PERF_U64_TEXEL_FETCH_COUNT        = 0x25,
    GX2_PERF_F32_TEX_CACHE_STALLED        = 0x26,
    GX2_PERF_F32_TEX_MISS_RATE            = 0x27,
    GX2_PERF_U64_TEX_MEM_BYTES_READ       = 0x28,
    GX2_PERF_F32_DEPTH_STENCIL_TEST_BUSY  = 0x29,
    GX2_PERF_F32_HIZ_TRIVIAL_ACCEPT       = 0x2a,
    GX2_PERF_F32_HIZ_REJECT               = 0x2b,
    GX2_PERF_U64_PRE_Z_SAMPLES_PASSING    = 0x2c,
    GX2_PERF_U64_PRE_Z_SAMPLES_FAILING_S  = 0x2d,
    GX2_PERF_U64_PRE_Z_SAMPLES_FAILING_Z  = 0x2e,
    GX2_PERF_U64_POST_Z_SAMPLES_PASSING   = 0x2f,
    GX2_PERF_U64_POST_Z_SAMPLES_FAILING_S = 0x30,
    GX2_PERF_U64_POST_Z_SAMPLES_FAILING_Z = 0x31,
    GX2_PERF_F32_Z_UNIT_STALLED           = 0x32,
    GX2_PERF_U64_PIXELS_AT_CB             = 0x33,
    GX2_PERF_U64_PIXELS_CB_MEM_WRITTEN    = 0x34,
    GX2_PERF_U64_IA_VERTICES              = 0x35,
    GX2_PERF_U64_IA_PRIMITIVES            = 0x36,
    GX2_PERF_U64_VS_INVOCATIONS           = 0x37,
    GX2_PERF_U64_GS_INVOCATIONS           = 0x38,
    GX2_PERF_U64_GS_PRIMITIVES            = 0x39,
    GX2_PERF_U64_C_INVOCATIONS            = 0x3a,
    GX2_PERF_U64_C_PRIMITIVES             = 0x3b,
    GX2_PERF_U64_PS_INVOCATIONS           = 0x3c,
    GX2_PERF_U64_PA_INPUT_PRIM            = 0x3d,
} GX2PerfMetric;


// TODO: there are a lot of these, this list is incomplete
typedef enum GX2StatId {
    GX2_STAT_CP_CP_COUNT       = 0x0000,
    GX2_STAT_CP_RBIU_FIFO_FULL = 0x0001,
    // ...
    GX2_STAT_PIPELINE          = 0xf000,
} GX2StatId;



typedef uint32_t GX2PerfTag;

typedef const char* (*GX2PerfTagToStringFunction)(GX2PerfTag tag);


typedef union GX2MetricResult {
    uint64_t u64Result;
    float    f32Result;
} GX2MetricResult;


#define GX2_PERF_DATA_ALIGNMENT 0x40

typedef struct WUT_ALIGNAS(GX2_PERF_DATA_ALIGNMENT) GX2PerfData {
    uint8_t reserved[0x8a0];
} GX2PerfData;



typedef enum GX2PerfMetricType {
    GX2_PERF_METRIC_TYPE_U64 = 0,
    GX2_PERF_METRIC_TYPE_F32 = 1,
} GX2PerfMetricType;


GX2PerfMetricType GX2GetPerfMetricType(GX2PerfMetric metric);


// init/free
void GX2PerfInit(GX2PerfData* perfData,
                 uint32_t maxTags,
                 MEMAllocator* pAllocator);
void GX2PerfFree(GX2PerfData* perfData);


// collection method
void GX2PerfSetCollectionMethod(GX2PerfData* perfData, GX2PerfCollectionMethod method);
GX2PerfCollectionMethod GX2PerfGetCollectionMethod(const GX2PerfData* perfData);


// metrics
void GX2PerfMetricsClear(GX2PerfData* perfData);
BOOL GX2PerfMetricEnable(GX2PerfData* perfData,
                         GX2PerfType type,
                         uint32_t id);
BOOL GX2PerfMetricIsEnabled(const GX2PerfData* perfData,
                            GX2PerfType type,
                            uint32_t id);
BOOL GX2PerfMetricGetEnabled(const GX2PerfData* perfData,
                             uint32_t idx,
                             GX2PerfType* type,
                             uint32_t* id);


// tags
void GX2PerfTagEnable(GX2PerfData* perfData,
                      GX2PerfTag tag,
                      BOOL enable);
void GX2PerfTagEnableAll(GX2PerfData* perfData);
void GX2PerfTagDisableAll(GX2PerfData* perfData);
BOOL GX2PerfTagIsEnabled(const GX2PerfData* perfData, GX2PerfTag tag);


// start/end frame
void GX2PerfFrameStart(GX2PerfData* perfData);
void GX2PerfFrameEnd(GX2PerfData* perfData);


// start/end tag
uint32_t GX2PerfGetNumPasses(const GX2PerfData* perfData);
void GX2PerfPassStart(GX2PerfData* perfData);
void GX2PerfPassEnd(GX2PerfData* perfData);


// start/end tag
void GX2PerfTagStart(GX2PerfData* perfData, GX2PerfTag tag);
void GX2PerfTagEnd(GX2PerfData* perfData, GX2PerfTag tag);


// results
BOOL GX2PerfGetResultByFrame(const GX2PerfData* perfData,
                             GX2PerfType type,
                             uint32_t id,
                             GX2MetricResult* result);
BOOL GX2PerfGetResultByTagId(const GX2PerfData* perfData,
                             GX2PerfType type,
                             uint32_t id,
                             uint32_t tag,
                             uint32_t number,
                             GX2MetricResult* result);
BOOL GX2PerfGetResultByTagSequence(const GX2PerfData* perfData,
                                   GX2PerfType type,
                                   uint32_t id,
                                   uint32_t sequence,
                                   uint32_t* tag,
                                   uint32_t* number,
                                   uint32_t* depth,
                                   GX2MetricResult* result);


// printing
void GX2PerfPrintFrameResults(const GX2PerfData* perfData);
void GX2PerfPrintTagResults(const GX2PerfData* perfData, GX2PerfTagToStringFunction func);


// pass coherence
void GX2PerfSetPassCoherEnable(GX2PerfData* perfData, BOOL enable);
BOOL GX2PerfGetPassCoherEnable(const GX2PerfData* perfData);


#ifdef __cplusplus
}
#endif


#endif
