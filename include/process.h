#include <list.h>
#include <stdint.h>

#ifndef PROCESS_H
#define PROCESS_H

typedef void (*kthread_function_f)(void);

enum process_state {
    ready,
    running,
    waiting,
    new,
    exit
};

typedef struct {
    uint64_t x0, x1, x2, x3, x4, x5, x6, x7;
    uint64_t x8, x9, x10, x11, x12, x13, x14, x15;
    uint64_t x16, x17, x18, x19, x20, x21, x22, x23;
    uint64_t x24, x25, x26, x27, x28, x29;  // x29 = FP
    uint64_t x30;      // LR
    uint64_t sp;       // Stack pointer
    uint64_t nzcv;     // Condition flags
    uint64_t daif;     // Interrupt mask
} process_saved_state_t;

DEFINE_LIST(pcb);

typedef struct pcb {
    uint64_t saved_sp;      // Changed: just store the SP value
    void *stack_page;
    int identifier;
    int priority;
    DEFINE_LINK(pcb);
    enum process_state state;
    char process_name[16];
} process_control_block_t;

void process_init();
void create_kernel_thread(kthread_function_f thread_func, char *name, int name_length);
void schedule();

int process_list_size();

#endif