#pragma once

#include <setjmp.h>
#include <stdint.h>

static inline void jmp_buf_set_bp(jmp_buf buf, uint64_t bp) {
    asm volatile("xorq %%gs:0x38, %0\n\t" : "=g"(((uint64_t *)buf)[1]) : "0"(bp));
}

static inline void jmp_buf_set_sp(jmp_buf buf, uint64_t sp) {
    asm volatile("xorq %%gs:0x38, %0\n\t" : "=g"(((uint64_t *)buf)[2]) : "0"(sp));
}

static inline void jmp_buf_set_pc(jmp_buf buf, uint64_t pc) {
    asm volatile("xorq %%gs:0x38, %0\n\t" : "=g"(((uint64_t *)buf)[7]) : "0"(pc));
}
