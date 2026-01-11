#include <peripheral.h>
#include <stdint.h>

#ifndef TIMER_H
#define TIMER_H

#define SYSTEM_TIMER_BASE (PERIPHERAL_BASE + 0x3000)

void timer_init();
void timer_set(uint32_t usecs);

void delay_cycles(unsigned int n);
void delay_u_secs(unsigned int us);
unsigned long get_system_timer();

void debug_regs();

typedef struct {
    uint8_t timer0_matched: 1;
    uint8_t timer1_matched: 1;
    uint8_t timer2_matched: 1;
    uint8_t timer3_matched: 1;
    uint32_t reserved: 28;
} timer_control_reg_t;

typedef struct {
    uint32_t control;        // CS register - all 32 bits as one
    uint32_t counter_low;    // CLO
    uint32_t counter_high;   // CHI
    uint32_t timer_compare0; // C0
    uint32_t timer_compare1; // C1
    uint32_t timer_compare2; // C2
    uint32_t timer_compare3; // C3
} timer_registers_t;

#endif