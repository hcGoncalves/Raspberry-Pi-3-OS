.section ".text"

.global switch_to_thread

switch_to_thread:
    sub     sp, sp, #272        // Allocate space (34 registers × 8 bytes + 16 for alignment)
    
    stp     x0, x1, [sp, #0]    // Save x0-x1
    stp     x2, x3, [sp, #16]   // Save x2-x3
    stp     x4, x5, [sp, #32]   // Save x4-x5
    stp     x6, x7, [sp, #48]   // Save x6-x7
    stp     x8, x9, [sp, #64]   // Save x8-x9
    stp     x10, x11, [sp, #80] // Save x10-x11
    stp     x12, x13, [sp, #96] // Save x12-x13
    stp     x14, x15, [sp, #112]// Save x14-x15
    stp     x16, x17, [sp, #128]// Save x16-x17
    stp     x18, x19, [sp, #144]// Save x18-x19
    stp     x20, x21, [sp, #160]// Save x20-x21
    stp     x22, x23, [sp, #176]// Save x22-x23
    stp     x24, x25, [sp, #192]// Save x24-x25
    stp     x26, x27, [sp, #208]// Save x26-x27
    stp     x28, x29, [sp, #224]// Save x28-x29 (x29 is FP)
    
    mov     x9, sp              // Get current SP
    str     x30, [sp, #240]     // Save LR (x30)
    
    mrs     x10, nzcv           // Save condition flags (NZCV)
    mrs     x11, daif           // Save interrupt mask
    stp     x10, x11, [sp, #248]// Save NZCV and DAIF
    
    str     x9, [x0]            // Store current SP to old process
    mov     x19, x1             // Save new process pointer in callee-saved register
    mov     x0, #10000
    bl      timer_set            // Store current SP to current_sp_ptr
    
    // Load new context
    ldr     x9, [x19]
    mov     sp, x9            // Load new process's SP
    
    ldp     x10, x11, [sp, #248]// Restore NZCV and DAIF
    msr     nzcv, x10           // Restore condition flags
    msr     daif, x11           // Restore interrupt mask
    
    ldr     x30, [sp, #240]     // Restore LR
    
    ldp     x28, x29, [sp, #224]// Restore x28-x29
    ldp     x26, x27, [sp, #208]// Restore x26-x27
    ldp     x24, x25, [sp, #192]// Restore x24-x25
    ldp     x22, x23, [sp, #176]// Restore x22-x23
    ldp     x20, x21, [sp, #160]// Restore x20-x21
    ldp     x18, x19, [sp, #144]// Restore x18-x19
    ldp     x16, x17, [sp, #128]// Restore x16-x17
    ldp     x14, x15, [sp, #112]// Restore x14-x15
    ldp     x12, x13, [sp, #96] // Restore x12-x13
    ldp     x10, x11, [sp, #80] // Restore x10-x11
    ldp     x8, x9, [sp, #64]   // Restore x8-x9
    ldp     x6, x7, [sp, #48]   // Restore x6-x7
    ldp     x4, x5, [sp, #32]   // Restore x4-x5
    ldp     x2, x3, [sp, #16]   // Restore x2-x3
    ldp     x0, x1, [sp, #0]    // Restore x0-x1
    
    add     sp, sp, #272        // Restore stack pointer
    
    ret
