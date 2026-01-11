.section ".text.boot"

.global _start

_start:
    // Check processor ID is zero (executing on main core), else hang
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f
    // Not main core, hang
1:  wfe
    b       1b
2:  // Main Core

    //mov     x0, #(1 << 4)       // IMO bit
    //msr     hcr_el2, x0
    
    // Also explicitly enable IRQs
    //msr     daifclr, #2

    // Set stack to start below code
    ldr     x1, =_stack_top
    mov     sp, x1

    // Clean the BSS section
    ldr     x1, =__bss_start     // Start address
    ldr     w2, =__bss_size      // Size of the section
3:  cbz     w2, 4f               // Quit loop if zero
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b               // Loop if non zero

    // Jump to main() routine in C
    mov     x2, #0x100
4:  bl      main
    // Halt main core if it returns
    b       1b
