#pragma once

void uthread_manager_init();
void uthread_manager_start();

typedef void uthread_func_t(void *arg);
int uthread_create(uthread_func_t *func, void *arg);
void uthread_yield();
void uthread_exit();
void uthread_free(void *ptr);
int uthread_get_tid();
