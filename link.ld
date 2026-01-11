SECTIONS
{
    . = 0x80000;     /* Kernel load address for AArch64 */
    _start = .;

    _text_start = .;
    .text : { KEEP(*(.text.boot)) *(.text .text.* .gnu.linkonce.t*) }
    . = ALIGN(4096); /* align to page size */
    _text_end = .;

    _rodata_start = .;
    .rodata : { *(.rodata .rodata.* .gnu.linkonce.r*) }
    . = ALIGN(4096); /* align to page size */
    _rodata_end = .;

    _data_start = .;
    .data : { *(.data .data.* .gnu.linkonce.d*) }
    . = ALIGN(4096); /* align to page size */
    _data_end = .;

    __bss_start = .;
    .bss :
    {
        bss = .;
        *(.bss)
    }
    . = ALIGN(4096); /* align to page size */
    __bss_end = .;
    _end = .;

    /* Reserve space for stack */
    . = . + 0x800000;           /* 8MB stack */
    . = ALIGN(4096);
    _stack_top = .;

   /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}
__bss_size = (__bss_end - __bss_start)>>3;