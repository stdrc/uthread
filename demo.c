#include <stdio.h>

#include "uthread.h"

void func1(void *arg);
void func2(void *arg);
void func3(void *arg);

void func1(void *arg) {
    int n = (int)arg;
    uthread_create(func2, (void *)uthread_get_tid());
    for (int i = 0; i < n; i++) {
        printf("Hello, I'm thread %d, my arg is %d\n", uthread_get_tid(), n);
        uthread_yield();
    }
}

void func2(void *arg) {
    printf("func2 from thread %d!\n", (int)arg);
    uthread_yield();
    printf("func2 from thread %d again!\n", (int)arg);
    uthread_create(func3, (void *)uthread_get_tid());
}

void func3(void *arg) {
    printf("func3 from thread %d!\n", (int)arg);
    uthread_exit();
    printf("never reach here\n");
}

int main() {
    uthread_manager_init();
    for (int i = 0; i < 32; i++) {
        uthread_create(func1, (void *)(i + 1));
    }
    uthread_manager_start();
    return 0;
}
