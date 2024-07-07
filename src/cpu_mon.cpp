// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdint>
#include <optional>

#include "cpu_mon.hpp"



#define PM_ARG_MMCR0 (1u << 0u)
#define PM_ARG_MMCR1 (1u << 1u)
#define PM_ARG_PMC1  (1u << 2u)
#define PM_ARG_PMC2  (1u << 3u)
#define PM_ARG_PMC3  (1u << 4u)
#define PM_ARG_PMC4  (1u << 5u)

// These are written to MMCR0
#define PM_PMC1_CPU_CYCLES              (0b0000001u << 6u)
#define PM_PMC1_INSTR_COMPLETED         (0b0000010u << 6u)
#define PM_PMC1_INSTR_DISPATCHED        (0b0000100u << 6u)
#define PM_PMC1_EIEIO_COMPLETED         (0b0000101u << 6u)
#define PM_PMC1_ITLB_SEARCH_CYCLES      (0b0000110u << 6u)
#define PM_PMC1_L2_HITS                 (0b0000111u << 6u)
#define PM_PMC1_UNRESOLVED_BRANCHES     (0b0001011u << 6u)
#define PM_PMC1_UNRESOLVED_STALL_CYCLES (0b0001100u << 6u)

#define PM_PMC2_CPU_CYCLES              (0b000001u)
#define PM_PMC2_INSTR_COMPLETED         (0b000010u)
#define PM_PMC2_INSTR_DISPATCHED        (0b000100u)
#define PM_PMC2_L1_ICACHE_MISSES        (0b000101u)
#define PM_PMC2_ITLB_MISSES             (0b000110u)
#define PM_PMC2_L2_IMISSES              (0b000111u)
#define PM_PMC2_PRED_BRANCHES_NOT_TAKEN (0b001000u)
#define PM_PMC2_LOADS_STORES            (0b001011u)
#define PM_PMC2_L1_INSTR_MISS_CYCLES    (0b001111u)

// These are written to MMCR1
#define PM_PMC3_CPU_CYCLES          (0b00001u << 27u)
#define PM_PMC3_INSTR_COMPLETED     (0b00010u << 27u)
#define PM_PMC3_INSTR_DISPATCHED    (0b00100u << 27u)
#define PM_PMC3_DCACHE_MISSES       (0b00101u << 27u)
#define PM_PMC3_L2_DMISSES          (0b00111u << 27u)
#define PM_PMC3_PRED_BRANCHES_TAKEN (0b01000u << 27u)
#define PM_PMC3_FPU_OPS             (0b01011u << 27u)
#define PM_PMC3_L1_DATA_MISS_CYCLES (0b01111u << 27u)

#define PM_PMC4_CPU_CYCLES       (0b00001u << 22u)
#define PM_PMC4_INSTR_COMPLETED  (0b00010u << 22u)
#define PM_PMC4_INSTR_DISPATCHED (0b00100u << 22u)
#define PM_PMC4_MISPRED_BRANCHES (0b01000u << 22u)
#define PM_PMC4_INT_OPS          (0b01101u << 22u)



extern "C"
void OSSetPerformanceMonitor(uint32_t arg_mask,
                             uint32_t mmcr0,
                             uint32_t mmcr1,
                             uint32_t pmc1,
                             uint32_t pmc2,
                             uint32_t pmc3,
                             uint32_t pmc4);


// convenience C++ wrapper
void
set_performance_monitor(std::optional<std::uint32_t> mmcr0,
                        std::optional<std::uint32_t> mmcr1,
                        std::optional<std::uint32_t> pmc1 = {},
                        std::optional<std::uint32_t> pmc2 = {},
                        std::optional<std::uint32_t> pmc3 = {},
                        std::optional<std::uint32_t> pmc4 = {})
{
    std::uint32_t mask = 0;
    if (mmcr0)
        mask |= PM_ARG_MMCR0;
    if (mmcr1)
        mask |= PM_ARG_MMCR1;

    if (pmc1)
        mask |= PM_ARG_PMC1;
    if (pmc2)
        mask |= PM_ARG_PMC2;
    if (pmc3)
        mask |= PM_ARG_PMC3;
    if (pmc4)
        mask |= PM_ARG_PMC4;

    OSSetPerformanceMonitor(mask,
                            mmcr0.value_or(0),
                            mmcr1.value_or(0),
                            pmc1.value_or(0),
                            pmc2.value_or(0),
                            pmc3.value_or(0),
                            pmc4.value_or(0));
}


std::uint32_t
read_upmc1()
{
    std::uint32_t result;
    asm("mfupmc1 %[result]"
        : [result] "=r"(result));
    return result;
}


std::uint32_t
read_upmc2()
{
    std::uint32_t result;
    asm("mfupmc2 %[result]"
        : [result] "=r"(result));
    return result;
}


std::uint32_t
read_upmc3()
{
    std::uint32_t result;
    asm("mfupmc3 %[result]"
        : [result] "=r"(result));
    return result;
}


std::uint32_t
read_upmc4()
{
    std::uint32_t result;
    asm("mfupmc4 %[result]"
        : [result] "=r"(result));
    return result;
}



namespace cpu_mon {

    void
    initialize()
    {}


    void
    finalize()
    {
        set_performance_monitor(0, 0);
    }


}
