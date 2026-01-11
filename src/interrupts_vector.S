.section ".text"
.global move_exception_vector
.global exception_vector_table

// AArch64 Exception Vector Table
// Each entry is 128 bytes (0x80), must be aligned to 2KB (0x800)
.balign 0x800
exception_vector_table:
    // Current EL with SP0
    .balign 0x80
    b       sync_exception_handler      // Synchronous
    .balign 0x80
    b       irq_handler_entry            // IRQ
    .balign 0x80
    b       fiq_handler_entry            // FIQ
    .balign 0x80
    b       serror_handler              // SError

    // Current EL with SPx
    .balign 0x80
    b       sync_exception_handler
    .balign 0x80
    b       irq_handler_entry
    .balign 0x80
    b       fiq_handler_entry
    .balign 0x80
    b       serror_handler

    // Lower EL using AArch64
    .balign 0x80
    b       sync_exception_handler
    .balign 0x80
    b       irq_handler_entry
    .balign 0x80
    b       fiq_handler_entry
    .balign 0x80
    b       serror_handler

    // Lower EL using AArch32
    .balign 0x80
    b       sync_exception_handler
    .balign 0x80
    b       irq_handler_entry
    .balign 0x80
    b       fiq_handler_entry
    .balign 0x80
    b       serror_handler

// Install exception vector table
move_exception_vector:
    stp     x29, x30, [sp, #-16]!   // Save FP and LR
    
    adr     x0, exception_vector_table
    msr     vbar_el2, x0            // Set Vector Base Address Register
    
    ldp     x29, x30, [sp], #16     // Restore FP and LR
    ret

// IRQ Handler Entry Point
irq_handler_entry:

    // Increment counter FIRST before any other operations
    stp     x0, x1, [sp, #-16]!     // Save x0, x1 temporarily
    
    ldr     x0, =irq_entry_count
    ldr     w1, [x0]
    add     w1, w1, #1
    str     w1, [x0]
    
    ldp     x0, x1, [sp], #16       // Restore x0, x1

    // Save all registers
    stp     x0, x1, [sp, #-16]!
    stp     x2, x3, [sp, #-16]!
    stp     x4, x5, [sp, #-16]!
    stp     x6, x7, [sp, #-16]!
    stp     x8, x9, [sp, #-16]!
    stp     x10, x11, [sp, #-16]!
    stp     x12, x13, [sp, #-16]!
    stp     x14, x15, [sp, #-16]!
    stp     x16, x17, [sp, #-16]!
    stp     x18, x19, [sp, #-16]!
    stp     x20, x21, [sp, #-16]!
    stp     x22, x23, [sp, #-16]!
    stp     x24, x25, [sp, #-16]!
    stp     x26, x27, [sp, #-16]!
    stp     x28, x29, [sp, #-16]!
    
    mrs     x0, elr_el2             // Save exception return address
    mrs     x1, spsr_el2            // Save processor state
    stp     x0, x1, [sp, #-16]!
    str     x30, [sp, #-16]!        // Save LR
    
    // Ensure 16-byte stack alignment
    mov     x0, sp
    and     x0, x0, #15
    sub     sp, sp, x0
    str     x0, [sp, #-16]!         // Save alignment adjustment
    
    // Call C handler
    bl      irq_handler
    
    // Restore alignment
    ldr     x0, [sp], #16
    add     sp, sp, x0
    
    // Restore registers
    ldr     x30, [sp], #16          // Restore LR
    ldp     x0, x1, [sp], #16       // Restore ELR and SPSR
    msr     elr_el2, x0
    msr     spsr_el2, x1
    
    ldp     x28, x29, [sp], #16
    ldp     x26, x27, [sp], #16
    ldp     x24, x25, [sp], #16
    ldp     x22, x23, [sp], #16
    ldp     x20, x21, [sp], #16
    ldp     x18, x19, [sp], #16
    ldp     x16, x17, [sp], #16
    ldp     x14, x15, [sp], #16
    ldp     x12, x13, [sp], #16
    ldp     x10, x11, [sp], #16
    ldp     x8, x9, [sp], #16
    ldp     x6, x7, [sp], #16
    ldp     x4, x5, [sp], #16
    ldp     x2, x3, [sp], #16
    ldp     x0, x1, [sp], #16
    
    eret                            // Exception return

// Placeholder handlers
sync_exception_handler:
    b       sync_exception_handler  // Hang on synchronous exception

fiq_handler_entry:
    b       fiq_handler_entry       // Hang on FIQ

serror_handler:
    b       serror_handler          // Hang on SError
