#include <setjmp.h>
#include <stdio.h>

#include "thread.h"

void task0(void *arg) {
    printf("Hello, I'm task0");
}

void task1(void *arg) {
    printf("Hello, I'm thread %d, my arg is %d\n", thread_gettid(), (int)arg);
    thread_yield();
    printf("Hello, I'm thread %d, my arg is %d\n", thread_gettid(), (int)arg);
}

void task2(void *arg) {
    printf("Hello, I'm thread %d, my arg is %d\n", thread_gettid(), (int)arg);
    // thread_create("Task 3", task0, (void *)3);
    thread_yield();
    printf("Hello, I'm thread %d, my arg is %d\n", thread_gettid(), (int)arg);
}

int main() {
    thread_scheduler_init();
    thread_create("Task 1", task1, (void *)1);
    thread_create("Task 2", task2, (void *)2);
    thread_scheduler_run();
    return 0;
}
