#include <spinlock.h>
#include <terminal.h>

void spin_init(spin_lock_t *lock) {
    lock->locked = 0;
    lock->saved_daif = 0x000003C0;
}

void spin_lock(spin_lock_t *lock) {
    unsigned long daif;
    __asm__ __volatile__("mrs %0, daif" : "=r"(daif));
    __asm__ __volatile__("msr daifset, #2" ::: "memory");  // Disable IRQ
    
    lock->saved_daif = daif;
    lock->locked = 1;
}

void spin_unlock(spin_lock_t *lock) {
    lock->locked = 0;
    __asm__ __volatile__("msr daif, %0" :: "r"(lock->saved_daif) : "memory");
}