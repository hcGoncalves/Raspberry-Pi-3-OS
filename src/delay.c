#include <process.h>
#include <interrupts.h>
#include <delay.h>
#include <terminal.h>

#define SYSTEM_TIMER_LOW (volatile unsigned int*)(SYSTEM_TIMER_BASE+0x4)
#define SYSTEM_TIMER_HIGH (volatile unsigned int*)(SYSTEM_TIMER_BASE+0x8)

#define TIMER_CS_M0   (1 << 0)
#define TIMER_CS_M1   (1 << 1)
#define TIMER_CS_M2   (1 << 2)
#define TIMER_CS_M3   (1 << 3)

static timer_registers_t * timer_regs;

static void timer_irq_handler() {
    schedule();
}

static void timer_irq_clearer() {
    timer_regs->control = TIMER_CS_M1;  // Write 1 to clear timer 1 match bit
}

void timer_init() {
    timer_regs = (timer_registers_t *) SYSTEM_TIMER_BASE;
    timer_regs->timer_compare1 = 1;

    register_irq_handler(SYSTEM_TIMER_1, timer_irq_handler, timer_irq_clearer);
}

void timer_set(uint32_t usecs) {
    timer_regs->timer_compare1 = timer_regs->counter_low + usecs;
}

void delay_cycles(unsigned int n) {
    if (n) while (n--) {asm volatile("nop");}
}

void delay_u_secs(unsigned int n) {
    register unsigned long f,t,r;

    asm volatile ("mrs %0, cntfrq_el0" : "=r"(f));
    asm volatile ("mrs %0, cntpct_el0" : "=r"(t));

    unsigned long i=((f/1000)*n)/1000;

    do {asm volatile("mrs %0, cntpct_el0" : "=r"(r));} while (r-t<i);
}

unsigned long get_system_timer() {
    unsigned int h=-1, l;

    h=*SYSTEM_TIMER_HIGH;
    l=*SYSTEM_TIMER_LOW;

    if (h!=*SYSTEM_TIMER_HIGH) {
        h=*SYSTEM_TIMER_HIGH;
        l=*SYSTEM_TIMER_LOW;
    }

    return ((unsigned long) h << 32) | l;
}

void debug_regs() {
    pushTerminal("Timer control: 0x");
    pushTerminalHex(timer_regs->control);
    pushTerminal("\nTimer compare: ");
    pushTerminalInt(timer_regs->timer_compare1);
    pushTerminal("\nTimer counter: ");
    pushTerminalInt(timer_regs->counter_low);
    pushTerminal("\n");
}