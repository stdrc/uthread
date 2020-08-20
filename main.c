#include <stdio.h>

#include "thread.h"

void func(void *arg) {
    printf("Hello, I'm thread %d, my arg is %d\n", thread_gettid(), (int)arg);
    thread_yield();
    printf("Hello, I'm thread %d, my arg is %d\n", thread_gettid(), (int)arg);
}

void func2(void *arg) {
    printf("Hello, I'm thread %d, my arg is %d\n", thread_gettid(), (int)arg);
    // thread_create("Task 3", task0, (void *)3);
    // thread_yield();
    // thread_yield();
    // thread_yield();
    thread_yield();
    printf("Hello, I'm thread %d, my arg is %d\n", thread_gettid(), (int)arg);
}

int main() {
    thread_manager_init();
    thread_create("Task 1", func, (void *)1);
    thread_create("Task 2", func, (void *)2);
    thread_manager_start();
    return 0;
}
