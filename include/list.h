#include <spinlock.h>
#include <stddef.h>

#ifndef LIST_H
#define LIST_H

#define DEFINE_LIST(nType)      \
struct nType;                   \
typedef struct nType##list {    \
    struct nType * head;        \
    struct nType * tail;        \
    spin_lock_t lock;           \
    unsigned long size;         \
} nType##_list_t;

#define DEFINE_LINK(nType)      \
struct nType * next##nType;     \
struct nType * prev##nType;     \
nType##_list_t * container;

#define INITIALISE_LIST(list)           \
    list.head = list.tail = NULL;       \
    list.size = 0;                      \
    spin_init(&list.lock);              


#define IMPLEMENT_LIST(nType)                                                   \
void append_##nType##_list(nType##_list_t * list, struct nType * node) {        \
    spin_lock(&list->lock);                                                     \
    if (list->tail == NULL) {                                                   \
        list->head = node;                                                      \
        list->tail = node;                                                      \
        node->prev##nType = NULL;                                               \
    } else {                                                                    \
        list->tail->next##nType = node;                                         \
        node->prev##nType = list->tail;                                         \
        list->tail = node;                                                      \
    }                                                                           \
    node->next##nType = NULL;                                                   \
    list->size++;                                                               \
    node->container = list;                                                     \
    spin_unlock(&list->lock);                                                   \
}                                                                               \
                                                                                \
void push_##nType##_list(nType##_list_t * list, struct nType * node) {          \
    spin_lock(&list->lock);                                                     \
    node->next##nType = list->head;                                             \
    node->prev##nType = NULL;                                                   \
    list->head = node;                                                          \
    list->size++;                                                               \
    if (list->tail == NULL) {                                                   \
        list->tail = node;                                                      \
    }                                                                           \
    node->container = list;                                                     \
    spin_unlock(&list->lock);                                                   \
}                                                                               \
                                                                                \
struct nType *peek_##nType##_list(nType##_list_t *list) {                       \
    return list->head;                                                          \
}                                                                               \
                                                                                \
struct nType *pop_##nType##_list(nType##_list_t *list) {                        \
    spin_lock(&list->lock);                                                     \
    struct nType *result = list->head;                                          \
    if (result == NULL) {                                                       \
        spin_unlock(&list->lock);                                               \
        return NULL;  /* Empty list */                                          \
    }                                                                           \
    list->head = list->head->next##nType;                                       \
    if (list->head != NULL) {                                                   \
        list->head->prev##nType = NULL;  /* Only if there's a new head */       \
    } else {                                                                    \
        list->tail = NULL;  /* List is now empty */                             \
    }                                                                           \
    list->size--;                                                               \
    result->container = NULL;                                                   \
    spin_unlock(&list->lock);                                                   \
    return result;                                                              \
}                                                                               \
                                                                                \
unsigned long size_##nType##_list(nType##_list_t *list) {                       \
    return list->size;                                                          \
}                                                                               \
                                                                                \
struct nType *next_##nType##_list(struct nType *node) {                         \
    return node->next##nType;                                                   \
}                                                                               \
                                                                                \
void remove_##nType(nType##_list_t *list, struct nType *node) {                 \
    spin_lock(&list->lock);                                                     \
    if (node->container == list) {                                              \
        if (node->prev##nType == NULL) {                                        \
            list->head = node->next##nType;                                     \
        } else {                                                                \
            node->prev##nType = node->next##nType;                              \
        }                                                                       \
        if (node->next##nType == NULL) {                                        \
            list->tail = node->prev##nType;                                     \
        } else {                                                                \
            node->next##nType = node->prev##nType;                              \
        }                                                                       \
    }                                                                           \
    node->container = NULL;                                                     \
    spin_unlock(&list->lock);                                                   \
}

#endif