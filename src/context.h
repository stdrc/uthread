#pragma once

#include <setjmp.h>
#include <stdint.h>

typedef struct {
    jmp_buf _env;
} context_t[1];

#define save_context(ctx) setjmp((ctx)[0]._env)
#define restore_context(ctx, ret_val) longjmp((ctx)[0]._env, (ret_val))

static inline void context_set_bp(context_t ctx, uint64_t bp) {
    asm volatile("xorq %%gs:0x38, %0\n\t" : "=g"(((uint64_t *)(ctx)[0]._env)[1]) : "0"(bp));
}

static inline void context_set_sp(context_t ctx, uint64_t sp) {
    asm volatile("xorq %%gs:0x38, %0\n\t" : "=g"(((uint64_t *)(ctx)[0]._env)[2]) : "0"(sp));
}

static inline void context_set_pc(context_t ctx, uint64_t pc) {
    asm volatile("xorq %%gs:0x38, %0\n\t" : "=g"(((uint64_t *)(ctx)[0]._env)[7]) : "0"(pc));
}
