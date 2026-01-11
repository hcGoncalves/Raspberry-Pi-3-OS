.section ".text"
.global try_lock

try_lock:
    mov     w1, #1              // Value to write (locked)
1:  
    ldaxr   w2, [x0]            // Exclusive load
    cbnz    w2, fail            // Already locked?
    stlxr   w3, w1, [x0]        // Try exclusive store
    cbnz    w3, 1b              // Store failed? Retry
    mov     w0, #1              // Success
    ret

fail:
    clrex                       // Clear exclusive monitor
    mov     w0, #0              // Failed
    ret
