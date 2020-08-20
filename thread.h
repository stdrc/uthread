#pragma once

#include <setjmp.h>

#include "list.h"

void thread_manager_init();
void thread_manager_start();

typedef void thread_func(void *arg);
int thread_create(thread_func *func, void *arg);
void thread_yield();
void thread_exit();
void thread_free(void *ptr);
int thread_gettid();

void __thread_debug_print_ready_list();
