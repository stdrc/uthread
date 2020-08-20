#pragma once

#include <setjmp.h>
#include <stdint.h>

struct jmp_buf_vals {
    uint64_t bp;
    uint64_t sp;
    uint64_t pc;
};

static inline struct jmp_buf_vals jmp_buf_parse(jmp_buf buf) {
    // platform dependent
    uint64_t *p = (uint64_t *)buf;
    struct jmp_buf_vals ret;
    asm volatile("xorq %%gs:0x38, %0\n\t" : "=g"(ret.bp) : "0"(p[1]));
    asm volatile("xorq %%gs:0x38, %0\n\t" : "=g"(ret.sp) : "0"(p[2]));
    asm volatile("xorq %%gs:0x38, %0\n\t" : "=g"(ret.pc) : "0"(p[7]));
    return ret;
}

static inline void jmp_buf_overwrite(jmp_buf buf, struct jmp_buf_vals vals) {
    // platform dependent
    uint64_t *p = (uint64_t *)buf;
    asm volatile("xorq %%gs:0x38, %0\n\t" : "=g"(p[1]) : "0"(vals.bp));
    asm volatile("xorq %%gs:0x38, %0\n\t" : "=g"(p[2]) : "0"(vals.sp));
    asm volatile("xorq %%gs:0x38, %0\n\t" : "=g"(p[7]) : "0"(vals.pc));
}
