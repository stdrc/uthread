#include "thread.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "list.h"

#ifdef DEBUG
#define debug(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define debug(fmt, ...)
#endif

#define STACK_ALIGNMENT (sizeof(void *) * 2)
#define STACK_SIZE 1024

struct thread {
    int tid;
    char name[MAX_THREAD_NAME];
    thread_func *func;
    void *arg;
    jmp_buf context;
    void *stack_base;
    size_t stack_size;
    struct list_node ready_node;
};

static int next_tid = 1;
static struct thread *current_thread = NULL;
static jmp_buf scheduler_context;
static struct list_node ready_list;

#pragma region Handle jmp_buf

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

#pragma endregion

#pragma region Scheduler

void thread_scheduler_init() {
    list_init(&ready_list);
}

void thread_scheduler_run() {
    debug("Scheduler started\n");
    setjmp(scheduler_context);
    debug("Scheduler reschedule\n");
    if (current_thread) {
        list_append(&ready_list, &current_thread->ready_node);
    }
    if (!list_empty(&ready_list)) {
        current_thread = container_of(ready_list.next, struct thread, ready_node);
        list_del(&current_thread->ready_node);
        debug("Thread %d selected\n", current_thread->tid);
        longjmp(current_thread->context, 1);
    }
}

#pragma endregion

static void thread_fire() {
    // new thread scheduled first time
    assert(current_thread != NULL);
    debug("Thread %d scheduled first time\n", current_thread->tid);
    current_thread->func(current_thread->arg);
    thread_exit();
}

void thread_create(const char *name, thread_func *func, void *arg) {
    // create tcb
    struct thread *thread = (struct thread *)malloc(sizeof(struct thread));
    thread->tid = __sync_fetch_and_add(&next_tid, 1);
    strncpy(thread->name, name, MAX_THREAD_NAME);
    thread->func = func;
    thread->arg = arg;
    list_init(&thread->ready_node);
    list_append(&ready_list, &thread->ready_node);

    if (setjmp(thread->context)) {
        thread_fire();
        // won't come back here
    }

    // prepare stack
    thread->stack_size = STACK_SIZE;
    posix_memalign(&thread->stack_base, STACK_ALIGNMENT, thread->stack_size);
    debug("Thread %d, stack base: %p, stack size: %lu\n", thread->tid, thread->stack_base, thread->stack_size);

    // modify jmp_buf
    struct jmp_buf_vals buf_vals = jmp_buf_parse(thread->context);
    buf_vals.sp = buf_vals.bp = (uint64_t)(thread->stack_base + thread->stack_size);
    jmp_buf_overwrite(thread->context, buf_vals);

    debug("Thread %d created\n", thread->tid);
}

void thread_yield() {
    if (!setjmp(current_thread->context)) {
        longjmp(scheduler_context, 1);
    }
}

void thread_exit() {
    if (current_thread) {
        free(current_thread->stack_base);
        free(current_thread);
        current_thread = NULL;
    }
    longjmp(scheduler_context, 1);
}

int thread_gettid() {
    return current_thread ? current_thread->tid : 0;
}

void __thread_debug_print_ready_list() {
    struct thread *thread;
    for_each_in_list(thread, struct thread, ready_node, &ready_list) {
        printf("Thread %d, func = %p, arg = %p\n", thread->tid, thread->func, thread->arg);
    }
}
