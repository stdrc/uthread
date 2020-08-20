#include "thread.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "jmp_buf.h"
#include "list.h"
#include "logging.h"

#define STACK_ALIGNMENT (sizeof(void *) * 2)
#define STACK_SIZE 512

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

static struct thread *current_thread = NULL;
static struct list_node ready_list;

#define MGR_START 0
#define MGR_RESCHED 1
#define MGR_EXIT_THREAD 2
#define MGR_FREE_MEM 3

static jmp_buf manager_context;
static void *mem_to_free = NULL;

void thread_manager_init() {
    list_init(&ready_list);
}

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
    case MGR_EXIT_THREAD: {
        debug("Manager exit thread\n");
        free(current_thread->stack_base);
        free(current_thread);
        current_thread = NULL;
        longjmp(manager_context, MGR_RESCHED);
        break;
    }
    case MGR_FREE_MEM: {
        debug("Manager free mem\n");
        if (mem_to_free) {
            free(mem_to_free);
            mem_to_free = NULL;
        }
        break;
    }
    }
    debug("Manager finished\n");
}

static inline void thread_manager_call(int svc) {
    if (!current_thread) fatal(1, "no thread is running\n");
    if (!setjmp(current_thread->context)) {
        longjmp(manager_context, svc);
    }
}

static int next_tid = 1;

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
        // new thread scheduled first time
        assert(current_thread != NULL);
        debug("Thread %d scheduled first time\n", current_thread->tid);
        current_thread->func(current_thread->arg);
        thread_exit();
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
    thread_manager_call(MGR_RESCHED);
}

void thread_exit() {
    thread_manager_call(MGR_EXIT_THREAD);
}

void thread_free(void *ptr) {
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
