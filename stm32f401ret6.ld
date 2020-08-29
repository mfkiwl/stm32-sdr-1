ENTRY(Reset_Handler)

MEMORY {
    FLASH(rx) : ORIGIN = 0x08000000, LENGTH = 512K
    RAM(rwx) : ORIGIN = 0x20000000, LENGTH = 96K
}

/* End of RAM */
_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS {

    .isr_vector : {
        . = ALIGN(4);
        KEEP(*(.isr_vector));
    } > FLASH

    .text : {
        *(.text*);

        KEEP(*(.init));
        KEEP(*(.fini));

        /* ctors */
        *crtbegin.o(.ctors)
        *crtbegin.o?(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors);
        *(SORT(.ctors.*));
        *(.ctors);

        /* dtors */
        *crtbegin.o(.dtors)
        *crtbegin.o?(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors);
        *(SORT(.dtors.*));
        *(.dtors);

        KEEP(*(.eh_frame*));
        . = ALIGN(4);
        PROVIDE(_etext = .);
    } > FLASH

    .rodata  : {
        *(.rodata*);
    } > FLASH

    /* used for C++ exception handling and unwinding */
    .ARM.extab : {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    .ARM.exidx : {
        *(.ARM.exidx* .gnu.linkonce.armextab.*)
    } > FLASH

    .preinit_array : {
        PROVIDE_HIDDEN(__preinit_array_start = .);
        KEEP(*(.preinit_array*));
        PROVIDE_HIDDEN(__preinit_array_end = .);
    } > FLASH

    .init_array : {
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(SORT(.init_array*)));
        KEEP(*(.init_array));
        PROVIDE_HIDDEN(__init_array_end = .);
    } > FLASH

    .fini_array : {
        PROVIDE_HIDDEN(__fini_array_start = .);
        KEEP(*(SORT(.fini_array*)));
        KEEP(*(.fini_array));
        PROVIDE_HIDDEN(__fini_array_end = .);
    } > FLASH

    /* _sidata contains the LMA of the initialization data in FLASH;
        it is used by the startup code to initialize the data in RAM*/
    _sidata = LOADADDR(.data);

    .data : {
        . = ALIGN(4);
        /* _sdata is the VMA of the initialized data section in RAM */
        _sdata = .;
        *(.data*);
        _edata = .;
    } > RAM AT> FLASH

    /* uninitialized data section is zeroed out by startup */
    .bss : {
        . = ALIGN(4);
        _sbss = .;
        *(.bss*);
        _ebss = .;
    } > RAM
}
