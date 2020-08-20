#pragma once

#include <setjmp.h>
#include <stdint.h>

struct context {
    jmp_buf _env;
};

#define save_context(ctx) setjmp((ctx)._env)
#define restore_context(ctx, ret_val) longjmp((ctx)._env, (ret_val))

#define context_set_bp(ctx, bp)                                                             \
    do {                                                                                    \
        asm volatile("xorq %%gs:0x38, %0\n\t" : "=g"(((uint64_t *)ctx._env)[1]) : "0"(bp)); \
    } while (0)
#define context_set_sp(ctx, sp)                                                             \
    do {                                                                                    \
        asm volatile("xorq %%gs:0x38, %0\n\t" : "=g"(((uint64_t *)ctx._env)[2]) : "0"(sp)); \
    } while (0)
#define context_set_pc(ctx, pc)                                                             \
    do {                                                                                    \
        asm volatile("xorq %%gs:0x38, %0\n\t" : "=g"(((uint64_t *)ctx._env)[7]) : "0"(pc)); \
    } while (0)
