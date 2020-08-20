#include <stdio.h>

#include "thread.h"

void func(void *arg);
void func2(void *arg);

void func(void *arg) {
    int n = (int)arg;
    thread_create(func2, (void *)thread_gettid());
    for (int i = 0; i < n; i++) {
        printf("Hello, I'm thread %d, my arg is %d\n", thread_gettid(), n);
        thread_yield();
    }
}

void func2(void *arg) {
    if ((int)arg > 10) thread_exit();
    printf("func2 from thread %d!\n", (int)arg);
    thread_yield();
    printf("func2 from thread %d again!\n", (int)arg);
    thread_create(func2, (void *)thread_gettid());
}

int main() {
    thread_manager_init();
    for (int i = 0; i < 30; i++) {
        thread_create(func, (void *)(i + 1));
    }
    thread_manager_start();
    return 0;
}
