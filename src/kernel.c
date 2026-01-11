#include <fb.h>
#include <mb.h>
#include <delay.h>
#include <sd.h>
#include <fat.h>
#include <mem.h>

#include <terminal.h>

#include <utils.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include <process.h>
#include <mutex.h>
#include <interrupts.h>

#include <plot.h>
#include <storage.h>

//extern unsigned char _end;

mutex_t mutex;

void kernel_loop1() {
    int i = 0;
    while (1) {
        //char uartChar = uartUpdateReturn();

        //writeIn(uartChar);

        char timer[64];
        intToString(timer, get_system_timer());
        drawString(BUFFER_WIDTH/2, 0, timer, 0xFFFFFF);

        char numProcesses[32];
        intToString(numProcesses, process_list_size());
        drawString(BUFFER_WIDTH/2, 32, numProcesses, 0xFFFFFF);

        drawString(BUFFER_WIDTH/2, 50, "Thread 1", 0xFF0000);

        if (i++>=500) {
            i=0;
            schedule();
        }
    };
    

}

void kernel_loop2() {
    int i = 0;
    while (1) {
        //char uartChar = uartUpdateReturn();

        //writeIn(uartChar);

        char timer[64];
        intToString(timer, get_system_timer());
        drawString(BUFFER_WIDTH/2, 0, timer, 0xFFFFFF);

        char numProcesses[32];
        intToString(numProcesses, process_list_size());
        drawString(BUFFER_WIDTH/2, 32, numProcesses, 0xFFFFFF);

        drawString(BUFFER_WIDTH/2, 50, "Thread 2", 0x00FF00);

        if (i++>=500) {
            i=0;
            schedule();
        }
    };
}

void kernel_loop3() {
    int i = 0;
    int times = 5;
    while (1) {
        //char uartChar = uartUpdateReturn();

        //writeIn(uartChar);

        char timer[64];
        intToString(timer, get_system_timer());
        drawString(BUFFER_WIDTH/2, 0, timer, 0xFFFFFF);

        char numProcesses[32];
        intToString(numProcesses, process_list_size());
        drawString(BUFFER_WIDTH/2, 32, numProcesses, 0xFFFFFF);

        char numTimes[32];
        intToString(numTimes, times);
        drawString(BUFFER_WIDTH/2+16, 32, numTimes, 0xFFFFFF);

        drawString(BUFFER_WIDTH/2, 50, "Thread 3", 0x0000FF);

        if (i++>=500) {
            i=0;
            times--;

            if (times <= 0) {
                return;
            } else {
                schedule();
            }
        }
    };
    

}

void main(uint32_t x0, uint32_t x1, uint32_t x2) {
    //(void) x0; //device tree
    (void) x1;
    (void) x2;

    delay_u_secs(100);

    framebufferInit();
    
    initTerminal();

    /*pushTerminal("Stack top: 0x");
    extern char _stack_top;
    pushTerminalHex((uintptr_t)&_stack_top);
    pushTerminal("\nCurrent SP: 0x");
    register uint64_t sp_val;
    __asm__ __volatile__("mov %0, sp" : "=r"(sp_val));
    pushTerminalHex(sp_val);
    pushTerminalChar('\n');*/

    drawCircle(100,100,50,0xFFFFFF,0);
    
    pushTerminal("About to draw terminal\n");

    drawCircle(100,100,25,0xFFFFFF,0);
    drawTerminal(1);

    logTerminal("Initialising UART:");
    uartInit();
    uartWriteText("UART Initialised\n");
    logTerminal("DONE");

    logTerminal("Initialising Memory:");
    if (mem_init((void *)x0)) {
        logTerminal("FAILED");
    } else {
        logTerminal("DONE");
    }

    /*
    register uint64_t sp_after_mem;
    __asm__ __volatile__("mov %0, sp" : "=r"(sp_after_mem));
    pushTerminal("SP after mem init: used ");
    pushTerminalInt(sp_start - sp_after_mem);
    pushTerminal(" bytes\n");*/

    logTerminal("Initialising Interrupts:");
    interrupts_init();
    logTerminal("DONE");

    logTerminal("Initialising Timer:");
    timer_init();
    logTerminal("DONE");

    /*
    unsigned long vbar, daif, current_el;

    __asm__ __volatile__("msr daifclr, #0xf" ::: "memory");  // Clear D, A, I, F

    // Check it worked
    __asm__ __volatile__("mrs %0, daif" : "=r"(daif));
    pushTerminal("After daifclr, DAIF: 0x");
    pushTerminalHex(daif);
    pushTerminalChar('\n');

    timer_set(10000);

    // Now wait and check
    delay_u_secs(20000);
    pushTerminal("IRQ entry count: ");
    pushTerminalInt(irq_entry_count);
    pushTerminalChar('\n');



__asm__ __volatile__("mrs %0, vbar_el2" : "=r"(vbar));
__asm__ __volatile__("mrs %0, daif" : "=r"(daif));
__asm__ __volatile__("mrs %0, CurrentEL" : "=r"(current_el));

pushTerminal("VBAR_EL2: 0x");
pushTerminalHex(vbar);
pushTerminal("\nDAIF: 0x");
pushTerminalHex(daif);
pushTerminal(" (I bit = ");
pushTerminalInt((daif >> 7) & 1);
pushTerminal(")\n");
pushTerminal("\nCurrent EL: ");
pushTerminalInt((current_el >> 2) & 3);
pushTerminal("\n");


debug_regs();
debug_ints();


// Wait for timer to fire
timer_set(20000);

debug_regs();
debug_ints();
pushTerminal("\nIRQ entry count: ");
pushTerminalInt(irq_entry_count);
pushTerminal("\n");



// In main loop after waiting for IRQ:
pushTerminal("Vector marker: 0x");
pushTerminalHex(vector_marker);
pushTerminalChar('\n');*/

    logTerminal("Initialising Processes:");
    process_init();
    logTerminal("DONE");

    //uint32_t size = getMemSize((void *)x0);
    //pushTerminal("Memory Size: ");
    //pushTerminalInt((int)size);
    //pushTerminalChar('\n');
    

    //char mem[64];
    //intToString(mem, mailboxCall('0x00010005'));
    
    //logTerminal(mem);

    init_storage_window(1050, 550, 500, 450);


    page_t * sd_page_buffer = alloc_page();
    
    if(sdInit()==SD_OK) {
        if (fatGetPartition((uint8_t*)sd_page_buffer->vaddr_mapped)) {
            //list root directory entries
            
            fatListDirectory((uint8_t*)sd_page_buffer->vaddr_mapped);
        } else {
            logTerminal("ERROR: FAT partition not found");
        }

        /*if(sdReadblock(0,&_end,1)) {
           dumpToTerminal(&_end);
        }*/
    }

    free_page(sd_page_buffer);
    
    clearIn();

    //drawString(100,100,"Hello World",0xFFFFFF);

    //int offset = 0;

    logTerminal("Initialising Mutex:");
    mutex_init(&mutex);
    logTerminal("DONE");

    create_kernel_thread(kernel_loop1, "kernel_loop1", 12);
    create_kernel_thread(kernel_loop2, "kernel_loop2", 12);
    create_kernel_thread(kernel_loop3, "kernel_loop3", 12);

    logTerminal("kernel_loop threads created");

    /*pushTerminal("Interrupts enabled: ");
    pushTerminalInt(INTERRUPTS_ENABLED());
    pushTerminalChar('\n');

    pushTerminal("DAIF register: 0x");
    unsigned long daif;
    __asm__ __volatile__("mrs %0, daif" : "=r"(daif));
    pushTerminalHex(daif);
    pushTerminalChar('\n');*/

    init_plot(1050, 10, 500, 500);
    plot(-20, 20, 0.05, "x^2", 0xFF0000);
    plot(-10,10, 0.05, "1/x", 0x00FF00);
    plot(-20,20, 0.05, "5*x+3", 0x0000FF);

    while(1) {
        schedule();
    }
    

    logTerminal("Kernel Main Returned");

    while (1) {
        //char uartChar = uartUpdateReturn();

        //writeIn(uartChar);

        char timer[64];
        intToString(timer, get_system_timer());
        drawString(BUFFER_WIDTH/2, 0, timer, 0xFFFFFF);

        char numProcesses[32];
        intToString(numProcesses, process_list_size());
        drawString(BUFFER_WIDTH/2, 32, numProcesses, 0xFFFFFF);

        drawString(BUFFER_WIDTH/2, 50, "Return Thread", 0xFFFFFF);
    };
    
    
    /* {
        for (int x=0;x<BUFFER_WIDTH;x++) {
            drawLine(x, BUFFER_HEIGHT-500-sin((x+(offset-1))/100, 4)*25, x,BUFFER_HEIGHT, 0x000000);

            drawLine(x, BUFFER_HEIGHT-500-sin((x+offset)/100, 4)*25, x,BUFFER_HEIGHT, 0x5a94f2);
            drawPixel(x, BUFFER_HEIGHT-500-sin((x+offset)/100, 4)*25, 0x1166f0);

            offset++;
        }
    }*/
}