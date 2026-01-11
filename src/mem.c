#include <libfdt.h>
#include <mem.h>
#include <string.h>
#include <terminal.h>

static void heap_init(uintptr_t heap_start);

typedef struct heap_segment {
    struct heap_segment * next;
    struct heap_segment * prev;
    uint32_t is_allocated;
    uint32_t segment_size;
} heap_segment_t;

static heap_segment_t * heap_segment_list_head;

extern uint8_t _end;
static uint32_t num_pages;

IMPLEMENT_LIST(page);

static page_t * all_pages_arr;
page_list_t free_pages;

uint64_t getMemSize(void * dtb)
{
    if (fdt_check_header(dtb) != 0) {
        return 0;
    }

    int mem = fdt_path_offset(dtb, "/memory");
    int len;

    const uint32_t *reg = fdt_getprop(dtb, mem, "reg", &len);

    uint64_t base = fdt32_to_cpu(reg[0]);
    uint64_t size = fdt32_to_cpu(reg[1]);

    return size;
}

int mem_init(void * dtb) {
    uint32_t mem_size, page_arr_len, kernel_pages, i;
    uintptr_t page_arr_end;

    mem_size = getMemSize(dtb);
    if (mem_size == 0) {
        return 1;
    }
    num_pages = mem_size / PAGE_SIZE;

    pushTerminal("MEM: Pages: ");
    pushTerminalInt(num_pages);
    pushTerminalChar('\n');

    page_arr_len = sizeof(page_t) * num_pages;
    all_pages_arr = (page_t *)((uintptr_t)&_end + KERNEL_STACK_SIZE);

    bzero(all_pages_arr, page_arr_len);

    INITIALISE_LIST(free_pages);

    logTerminal("MEM: free_pages initialised");

    page_arr_end = (uintptr_t)all_pages_arr + page_arr_len;
    page_arr_end += page_arr_end % PAGE_SIZE ? PAGE_SIZE - (page_arr_end % PAGE_SIZE) : 0;


    kernel_pages = (page_arr_end) / PAGE_SIZE;

    //kernel_pages = (page_arr_end - 0x80000) / PAGE_SIZE + (0x80000 / PAGE_SIZE);
    pushTerminal("MEM: kernel_pages: ");
    pushTerminalInt(kernel_pages);
    pushTerminalChar('\n');

    for (i = 0; i < kernel_pages; i++) {
        all_pages_arr[i].vaddr_mapped = i * PAGE_SIZE;
        all_pages_arr[i].flags.allocated = 1;
        all_pages_arr[i].flags.kernel_page = 1;
    }

    for (; i < kernel_pages + (KERNEL_HEAP_SIZE / PAGE_SIZE); i++) {
        all_pages_arr[i].vaddr_mapped = i * PAGE_SIZE;
        all_pages_arr[i].flags.allocated = 1;
        all_pages_arr[i].flags.kernel_heap_page = 1;
    }

    for(; i < num_pages; i++){
        all_pages_arr[i].flags.allocated = 0;
        append_page_list(&free_pages, &all_pages_arr[i]);
    }

    logTerminal("MEM: Pages Allocated");

    heap_init(page_arr_end);

    pushTerminal("Heap start: 0x");
    pushTerminalHex(page_arr_end);
    pushTerminal("\nStack top: 0x");
    extern char _stack_top;
    pushTerminalHex((uintptr_t)&_stack_top);
    pushTerminalChar('\n');

    /*
    pushTerminal("END OF MEM_INIT verification:\n");
    pushTerminal("  free_pages.head = 0x");
    pushTerminalHex((uintptr_t)free_pages.head);
    pushTerminal("\n  free_pages.head->nextpage = 0x");
    pushTerminalHex((uintptr_t)free_pages.head->nextpage);
    pushTerminalChar('\n');
    */

    logTerminal("MEM: Heap Initialised");
   
    return 0;
}

void * alloc_page() {
    page_t * page;
    void * page_mem;

    if (size_page_list(&free_pages) == 0) return 0;

    //get page
    page = pop_page_list(&free_pages);
    page->flags.kernel_page = 1;
    page->flags.allocated = 1;

    page_mem = (void *)((page - all_pages_arr) * PAGE_SIZE);

    //zero out page
    bzero(page_mem, PAGE_SIZE);

    return page_mem;
}

void free_page(void * ptr) {
    page_t * page;

    //physical meta data
    page = all_pages_arr + ((uintptr_t)ptr / PAGE_SIZE);

    page->flags.allocated = 0;
    append_page_list(&free_pages, page);
}


static void heap_init(uintptr_t heap_start) {
    heap_segment_list_head = (heap_segment_t *) heap_start;
    bzero(heap_segment_list_head, sizeof(heap_segment_t));
    heap_segment_list_head->segment_size = KERNEL_HEAP_SIZE;
}

void * kmalloc(uint32_t bytes) {
    heap_segment_t * current, *best = NULL;
    int difference, best_difference = INT_MAX;

    bytes += sizeof(heap_segment_t);
    //bytes = (bytes + 15) & ~15; //new align
    bytes += bytes % 16 ? 16 - (bytes % 16) : 0;

    //find allocation that is closest to request
    for (current = heap_segment_list_head; current != NULL; current = current->next) {
        difference = current->segment_size - bytes;
        if (!current->is_allocated && difference < best_difference && difference >= 0) {
            best = current;
            best_difference = difference;
        }
    }

    //no free memory
    if (best == NULL) return NULL;

    //split up segment if the best difference is too large
    if (best_difference > (int)(2 * sizeof(heap_segment_t))) {
        bzero(((void *)(best)) + bytes, sizeof(heap_segment_t));
        current = best->next;
        best->next = ((void *)(best)) + bytes;
        best->next->next = current;
        best->next->prev = best;
        best->next->segment_size = best->segment_size - bytes;
        best->segment_size = bytes;
    }

    best->is_allocated = 1;

    return best + 1;
}

void kfree(void * ptr) {
    heap_segment_t * seg;

    if (!ptr) return;

    seg = ptr - sizeof(heap_segment_t);
    //seg = (heap_segment_t *)ptr - 1;
    seg->is_allocated = 0;

    //coalesce segments to the left
    while (seg->prev != NULL && !seg->prev->is_allocated) {
        seg->prev->segment_size += seg->segment_size;
        seg->prev->next = seg->next;
        seg->next->prev = seg->prev;
        seg = seg->prev;
    }

    //coalesce segments to the right
    while(seg->next != NULL && !seg->next->is_allocated) {
        seg->segment_size += seg->next->segment_size;
        if(seg->next->next!=NULL) seg->next->next->prev = seg;
        seg->next = seg->next->next;
    }
}

void verify_mem() {
    pushTerminal("MEM verification:\n");
    pushTerminal("  free_pages.head = 0x");
    pushTerminalHex((uintptr_t)free_pages.head);
    pushTerminal("\n  free_pages.head->nextpage = 0x");
    pushTerminalHex((uintptr_t)free_pages.head->nextpage);
    pushTerminal("\n  free_pages.tail = 0x");
    pushTerminalHex((uintptr_t)free_pages.tail);
    pushTerminalChar('\n');
}