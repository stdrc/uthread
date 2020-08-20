#include "thread.h"

#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

#include "jmp_buf.h"
#include "list.h"
#include "logging.h"

#define STACK_ALIGNMENT (0x10)
#define STACK_ALLOC_SIZE (0x1000)

struct thread {
    int tid;
    thread_func *func;
    void *arg;
    void *stack_base;
    size_t stack_size;
    struct list_node ready_node;
    jmp_buf context;
};

static struct thread *current_thread = NULL;
static struct list_node ready_list;

static jmp_buf manager_context;
static void *mem_to_free = NULL;

void thread_manager_init() {
    list_init(&ready_list);
}

#define MGR_START 0
#define MGR_RESCHED 1
#define MGR_FREE_MEM 2

void thread_manager_start() {
    debug("Manager started\n");
    int svc = setjmp(manager_context);
    debug("Manager service #%d\n", svc);
    switch (svc) {
    case MGR_START:
    case MGR_RESCHED: {
        debug("Manager reschedule\n");
        if (current_thread) {
            list_append(&ready_list, &current_thread->ready_node);
        }
        if (!list_empty(&ready_list)) {
            current_thread = container_of(ready_list.next, struct thread, ready_node);
            list_del(&current_thread->ready_node);
            debug("Thread %d selected\n", current_thread->tid);
            longjmp(current_thread->context, 1);
        }
        break;
    }
    case MGR_FREE_MEM: {
        debug("Manager free mem\n");
        if (mem_to_free) {
            free(mem_to_free);
            mem_to_free = NULL;
        }
        longjmp(current_thread->context, 1);
        break;
    }
    default:
        longjmp(current_thread->context, 1);
    }
    debug("Manager finished\n");
}

static inline void thread_manager_call(int svc) {
    if (!current_thread || !setjmp(current_thread->context)) {
        longjmp(manager_context, svc);
    }
}

static void thread_entry() {
    assert(current_thread != NULL);
    debug("Thread %d scheduled first time\n", current_thread->tid);
    current_thread->func(current_thread->arg);
    thread_exit();
}

static int next_tid = 1;

int thread_create(thread_func *func, void *arg) {
    // create tcb
    void *stack_base;
    if (posix_memalign(&stack_base, STACK_ALIGNMENT, STACK_ALLOC_SIZE)) {
        fatal(1, "failed to allocate memory");
    }
    void *stack_top = (void *)stack_base + STACK_ALLOC_SIZE - sizeof(struct thread);
    struct thread *thread = (struct thread *)stack_top;
    thread->tid = __sync_fetch_and_add(&next_tid, 1);
    thread->func = func;
    thread->arg = arg;
    thread->stack_base = stack_base;
    thread->stack_size = stack_top - stack_base;
    list_init(&thread->ready_node);
    list_append(&ready_list, &thread->ready_node);
    debug("Thread %d, stack base: %p, stack size: %zu\n", thread->tid, thread->stack_base, thread->stack_size);

    if (setjmp(thread->context)) fatal(1, "shouldn't reach here");
    // set stack and pc
    stack_top -= 0x10 - 0x8; // spare some space for safety
    jmp_buf_set_bp(thread->context, (uint64_t)stack_top);
    jmp_buf_set_sp(thread->context, (uint64_t)stack_top);
    jmp_buf_set_pc(thread->context, (uint64_t)thread_entry);

    debug("Thread %d created\n", thread->tid);
    return thread->tid;
}

void thread_yield() {
    assert(current_thread != NULL);
    thread_manager_call(MGR_RESCHED);
}

void thread_exit() {
    assert(current_thread != NULL);
    thread_free(current_thread->stack_base);
    current_thread = NULL;
    thread_manager_call(MGR_RESCHED);
}

void thread_free(void *ptr) {
    assert(current_thread != NULL);
    mem_to_free = ptr;
    thread_manager_call(MGR_FREE_MEM);
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
