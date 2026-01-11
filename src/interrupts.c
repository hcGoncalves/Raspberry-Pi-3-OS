#include <interrupts.h>
#include <string.h>
#include <terminal.h>
#include <process.h>

static interrupt_registers_t * interrupt_regs;

static interrupt_handler_f handlers[NUM_IRQS];
static interrupt_clearer_f clearers[NUM_IRQS];

extern void move_exception_vector();
extern uint32_t exception_vector;

volatile uint32_t irq_entry_count = 0;

void interrupts_init() {
    interrupt_regs = (interrupt_registers_t *)INTERRUPTS_PENDING;
    bzero(handlers, sizeof(interrupt_handler_f) * NUM_IRQS);
    bzero(clearers, sizeof(interrupt_clearer_f) * NUM_IRQS);
    interrupt_regs->irq_basic_disable = 0xFFFFFFFF; //disable interrupts
    interrupt_regs->irq_gpu_disable1 = 0xFFFFFFFF;
    interrupt_regs->irq_gpu_disable2 = 0xFFFFFFFF;
    move_exception_vector();

    ENABLE_INTERRUPTS();

}

void irq_handler() {
    int j;

    for (j=0; j<NUM_IRQS; j++) {
        if (IRQ_IS_PENDING(interrupt_regs, j) && (handlers[j] != 0)) {
            clearers[j]();
            ENABLE_INTERRUPTS();
            handlers[j]();
            DISABLE_INTERRUPTS();
            
            return;
        }
    }
}

void register_irq_handler(irq_number_t irq_num, interrupt_handler_f handler, interrupt_clearer_f clearer) {
    uint32_t irq_pos;

    if (IRQ_IS_BASIC(irq_num)) {
        irq_pos = irq_num - 64;
        handlers[irq_num] = handler;
		clearers[irq_num] = clearer;
        interrupt_regs->irq_basic_enable |= (1 << irq_pos);
    } else if (IRQ_IS_GPU2(irq_num)) {
        irq_pos = irq_num - 32;
        handlers[irq_num] = handler;
		clearers[irq_num] = clearer;
        interrupt_regs->irq_gpu_enable2 |= (1 << irq_pos);
    } else if (IRQ_IS_GPU1(irq_num)) {
        irq_pos = irq_num;
        handlers[irq_num] = handler;
		clearers[irq_num] = clearer;
        interrupt_regs->irq_gpu_enable1 |= (1 << irq_pos);
    } else {
        pushTerminal("ERROR: Cannot Register IRQ Handler: Invalid IRQ Number: ");
        pushTerminalInt(irq_num);
        pushTerminalChar('\n');
    }
}

void unregister_irq_handler(irq_number_t irq_num) {
    uint32_t irq_pos;
    if (IRQ_IS_BASIC(irq_num)) {
        irq_pos = irq_num - 64;
        handlers[irq_num] = 0;
        clearers[irq_num] = 0;
        interrupt_regs->irq_basic_disable |= (1 << irq_pos);
    } else if (IRQ_IS_GPU2(irq_num)) {
        irq_pos = irq_num - 32;
        handlers[irq_num] = 0;
        clearers[irq_num] = 0;
        interrupt_regs->irq_gpu_disable2 |= (1 << irq_pos);
    } else if (IRQ_IS_GPU1(irq_num)) {
        irq_pos = irq_num;
        handlers[irq_num] = 0;
        clearers[irq_num] = 0;
        interrupt_regs->irq_gpu_disable1 |= (1 << irq_pos);
    } else {
        pushTerminal("ERROR: Cannot unregister IRQ handler: Invalid IRQ number: ");
        pushTerminalInt(irq_num);
        pushTerminalChar('\n');
    }
}

void debug_ints() {
    pushTerminal("IRQ enable1: 0x");
    pushTerminalHex(interrupt_regs->irq_gpu_enable1);
    pushTerminal("\nIRQ pending1: 0x");
    pushTerminalHex(interrupt_regs->irq_gpu_pending1);
    pushTerminal("\n");
}