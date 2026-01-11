#include <stdint.h>
#include <list.h>
#ifndef MEM_H
#define MEM_H

#define PAGE_SIZE 4096
#define KERNEL_HEAP_SIZE (1024*1024)
#define KERNEL_STACK_SIZE PAGE_SIZE

typedef struct {
    uint8_t allocated: 1;
    uint8_t kernel_page: 1;
    uint8_t kernel_heap_page: 1;
    uint32_t reserved: 29;
} page_flags_t;

DEFINE_LIST(page);

typedef struct page {
    uint32_t vaddr_mapped;
    page_flags_t flags;
    DEFINE_LINK(page);
} page_t;

int mem_init(void * dtb);

void * alloc_page();
void free_page(void * ptr);

void * kmalloc(uint32_t bytes);
void kfree(void * ptr);

void verify_mem();

#endif