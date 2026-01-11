#include <process.h>
#include <delay.h>
#include <string.h>
#include <mem.h>
#include <list.h>
#include <mutex.h>
#include <interrupts.h>
#include <terminal.h>

static unsigned long next_process_num = 1;
#define NEW_PID next_process_num++;

extern unsigned char _end;
extern void switch_to_thread(uint64_t *old, uint64_t *new);

IMPLEMENT_LIST(pcb);

pcb_list_t run_queue;
pcb_list_t all_proc_list;

process_control_block_t * current_process;

void schedule() {
    DISABLE_INTERRUPTS();
    process_control_block_t * new_thread, * old_thread;

    if (size_pcb_list(&run_queue) == 0) {
        timer_set(10000);
        ENABLE_INTERRUPTS();
        return;
    }

    new_thread = pop_pcb_list(&run_queue);
    old_thread = current_process;
    current_process = new_thread;
    
    append_pcb_list(&run_queue, old_thread);

    switch_to_thread(&old_thread->saved_sp, &new_thread->saved_sp);  //pass addresses of SP fields

    ENABLE_INTERRUPTS();
}

void process_init() {
    process_control_block_t * main_pcb;
    INITIALISE_LIST(run_queue);
    INITIALISE_LIST(all_proc_list);

    main_pcb = kmalloc(sizeof(process_control_block_t));
    main_pcb->stack_page = (void *)&_end;
    main_pcb->identifier = NEW_PID;
    memcpy(main_pcb->process_name, "init", 4);

    append_pcb_list(&all_proc_list, main_pcb);

    current_process = main_pcb;

    timer_set(10000);
}

static void reap() {
    DISABLE_INTERRUPTS();
    process_control_block_t * new_thread, * old_thread;

    //nothing on run queue so loop
    while (size_pcb_list(&run_queue) == 0);

    new_thread = pop_pcb_list(&run_queue);
    old_thread = current_process;

    current_process = new_thread;

    //free resources from old thread
    free_page(old_thread->stack_page);
    kfree(old_thread);

    remove_pcb(&all_proc_list, old_thread);

    switch_to_thread(&old_thread->saved_sp, &new_thread->saved_sp);
}

void create_kernel_thread(kthread_function_f thread_func, char * name, int name_len) {
    process_control_block_t * pcb;
    uint64_t *stack_top;

    pcb = kmalloc(sizeof(process_control_block_t));
    pcb->stack_page = alloc_page();
    pcb->identifier = NEW_PID;

    memcpy(pcb->process_name, name, MIN(name_len, 15));
    pcb->process_name[MIN(name_len, 15)] = 0;

    //set up initial stack frame
    stack_top = (uint64_t *)((uint8_t *)pcb->stack_page + PAGE_SIZE);
    
    stack_top -= 2; //space for FP and LR
    stack_top[0] = 0;
    stack_top[1] = (uint64_t)reap;
    
    stack_top -= 34;  //272 bytes for saved context
    bzero(stack_top, 272);
    
    //set x29 and x30 in the saved context
    stack_top[28] = 0;
    stack_top[29] = 0;                   
    stack_top[30] = (uint64_t)thread_func;
    
    //DAIF - interrupts enabled
    stack_top[32] = 0x000003C0;
    
    //save the stack pointer
    pcb->saved_sp = (uint64_t)stack_top;

    append_pcb_list(&all_proc_list, pcb);
    append_pcb_list(&run_queue, pcb);
}

void mutex_init(mutex_t * lock) {
    lock->locked = 0;
    lock->locker = 0;
    INITIALISE_LIST(lock->wait_queue);
}

void mutex_lock(mutex_t * lock) {
    process_control_block_t * new_thread, * old_thread;
    
    //try to acquire the mutex
    DISABLE_INTERRUPTS();
    
    if (lock->locked == 0) {
        //mutex is free acquire it
        lock->locked = 1;
        lock->locker = current_process;
        ENABLE_INTERRUPTS();
        return;
    }
    
    //mutex is held by someone else block this process
    new_thread = pop_pcb_list(&run_queue);
    old_thread = current_process;
    current_process = new_thread;
    
    //add ourselves to the wait queue
    append_pcb_list(&lock->wait_queue, old_thread);
    
    //context switch (this will re-enable interrupts on return)
    switch_to_thread(&old_thread->saved_sp, &new_thread->saved_sp);
    
    //when wake up the mutex has been given to us
    ENABLE_INTERRUPTS();
}

void mutex_unlock(mutex_t * lock) {
    process_control_block_t * thread;
    
    DISABLE_INTERRUPTS();
    
    //check if we own this mutex
    if (lock->locker != current_process) {
        ENABLE_INTERRUPTS();
        return;  //error: trying to unlock a mutex we dont own
    }
    
    //check if there are waiting threads
    if (size_pcb_list(&lock->wait_queue) > 0) {
        //give mutex directly to waiting thread
        thread = pop_pcb_list(&lock->wait_queue);
        lock->locker = thread;
        //keep locked = 1 transfer ownership
        push_pcb_list(&run_queue, thread);
    } else {
        //no one waiting release the mutex
        lock->locked = 0;
        lock->locker = 0;
    }
    
    ENABLE_INTERRUPTS();
}

int process_list_size() {
    return size_pcb_list(&run_queue);
}