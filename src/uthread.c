#include "uthread.h"

#include <assert.h>
#include <stdio.h>

#include "context.h"
#include "utils/list.h"
#include "utils/logging.h"

#define STACK_ALIGNMENT (0x10)
#define STACK_ALLOC_SIZE (0x1000)

struct uthread {
    int tid;
    uthread_func_t *func;
    void *arg;
    void *stack_base;
    size_t stack_size;
    struct list_node ready_node;
    struct context context;
};

static struct uthread *current_thread = NULL;
static struct list_node ready_list;

static struct context manager_context;
static void *mem_to_free = NULL;

void uthread_manager_init() {
    list_init(&ready_list);
}

#define MGR_SVC_START 0
#define MGR_SVC_RESCHED 1
#define MGR_SVC_FREE_MEM 2

void uthread_manager_start() {
    debug("Manager started\n");
    int svc = save_context(manager_context);
    debug("Manager service #%d\n", svc);
    switch (svc) {
    case MGR_SVC_START:
    case MGR_SVC_RESCHED: {
        debug("Manager reschedule\n");
        if (current_thread) {
            list_append(&ready_list, &current_thread->ready_node);
        }
        if (!list_empty(&ready_list)) {
            current_thread = container_of(ready_list.next, struct uthread, ready_node);
            list_del(&current_thread->ready_node);
            debug("Thread %d selected\n", current_thread->tid);
            restore_context(current_thread->context, 1);
        }
        break;
    }
    case MGR_SVC_FREE_MEM: {
        debug("Manager free mem\n");
        if (mem_to_free) {
            free(mem_to_free);
            mem_to_free = NULL;
        }
        restore_context(current_thread->context, 1);
        break;
    }
    default:
        restore_context(current_thread->context, 1);
    }
    debug("Manager finished\n");
}

static inline void uthread_manager_call(int svc) {
    if (!current_thread || !save_context(current_thread->context)) {
        restore_context(manager_context, svc);
    }
}

static void uthread_entry() {
    assert(current_thread != NULL);
    debug("Thread %d scheduled first time\n", current_thread->tid);
    current_thread->func(current_thread->arg);
    uthread_exit();
}

static int next_tid = 1;

int uthread_create(uthread_func_t *func, void *arg) {
    // create tcb
    void *stack_base;
    if (posix_memalign(&stack_base, STACK_ALIGNMENT, STACK_ALLOC_SIZE)) {
        fatal(1, "failed to allocate memory");
    }
    void *stack_top = (void *)stack_base + STACK_ALLOC_SIZE - sizeof(struct uthread);
    struct uthread *thread = (struct uthread *)stack_top;
    thread->tid = __sync_fetch_and_add(&next_tid, 1);
    thread->func = func;
    thread->arg = arg;
    thread->stack_base = stack_base;
    thread->stack_size = stack_top - stack_base;
    list_init(&thread->ready_node);
    list_append(&ready_list, &thread->ready_node);
    debug("Thread %d, stack base: %p, stack size: %zu\n", thread->tid, thread->stack_base, thread->stack_size);

    if (save_context(thread->context)) fatal(1, "shouldn't reach here");
    // set stack and pc
    stack_top = stack_top - 0x10 - 0x8; // spare some space for safety
    context_set_bp(thread->context, (uint64_t)stack_top);
    context_set_sp(thread->context, (uint64_t)stack_top);
    context_set_pc(thread->context, (uint64_t)uthread_entry);

    debug("Thread %d created\n", thread->tid);
    return thread->tid;
}

void uthread_yield() {
    assert(current_thread != NULL);
    uthread_manager_call(MGR_SVC_RESCHED);
}

void uthread_exit() {
    assert(current_thread != NULL);
    uthread_free(current_thread->stack_base);
    current_thread = NULL;
    uthread_manager_call(MGR_SVC_RESCHED);
}

void uthread_free(void *ptr) {
    assert(current_thread != NULL);
    mem_to_free = ptr;
    uthread_manager_call(MGR_SVC_FREE_MEM);
}

int uthread_get_tid() {
    return current_thread ? current_thread->tid : 0;
}
