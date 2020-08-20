#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define container_of(ptr, type, field) ((type *)((uintptr_t)(ptr) - (uintptr_t)(&(((type *)(0))->field))))

#define container_of_safe(ptr, type, field)             \
    ({                                                  \
        __typeof__(ptr) __ptr = (ptr);                  \
        type *__obj = container_of(__ptr, type, field); \
        (__ptr ? __obj : NULL);                         \
    })

struct list_node {
    struct list_node *prev;
    struct list_node *next;
};

static inline void list_init(struct list_node *head) {
    head->next = head->prev = head;
}

static inline bool list_empty(struct list_node *head) {
    return (head->prev == head && head->next == head);
}

static inline void list_add(struct list_node *head, struct list_node *node) {
    node->next = head->next;
    node->prev = head;
    head->next->prev = node;
    head->next = node;
}

static inline void list_append(struct list_node *head, struct list_node *node) {
    struct list_node *tail = head->prev;
    return list_add(tail, node);
}

static inline void list_del(struct list_node *node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    list_init(node);
}

#define for_each_in_list(elem, type, field, head)                                    \
    for (elem = container_of((head)->next, type, field); &((elem)->field) != (head); \
         elem = container_of(((elem)->field).next, type, field))

#define next_container_of_safe(obj, type, field)                                \
    ({                                                                          \
        __typeof__(obj) __obj = (obj);                                          \
        (__obj ? container_of_safe(((__obj)->field).next, type, field) : NULL); \
    })

#define __for_each_in_list_safe(elem, tmp, type, field, head)                                             \
    for (elem = container_of((head)->next, type, field), tmp = next_container_of_safe(elem, type, field); \
         &((elem)->field) != (head);                                                                      \
         elem = tmp, tmp = next_container_of_safe(tmp, type, field))

#define for_each_in_list_safe(elem, tmp, field, head) __for_each_in_list_safe(elem, tmp, __typeof__(*elem), field, head)
