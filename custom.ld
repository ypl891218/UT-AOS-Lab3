/* Define memory regions */
MEMORY {
    TEXT_REGION (rx) : ORIGIN = 0x70000, LENGTH = 0x200000   /* Code region */
    DATA_REGION (rw) : ORIGIN = 0x280000, LENGTH = 0x10000   /* Initialized data */
    BSS_REGION  (rw) : ORIGIN = 0x300000, LENGTH = 0x10000   /* Uninitialized data */
}

/* Define how sections are placed in memory */
SECTIONS {
    .text : {
        *(.text)           /* All `.text` sections from input files */
        *(.text.*)         /* All `.text.*` sections */
    } > TEXT_REGION       /* Place into TEXT_REGION */

    .preinit_array :
    {
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array*))
        PROVIDE_HIDDEN (__preinit_array_end = .);
    } > TEXT_REGION
    .init_array :
    {
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array*))
        PROVIDE_HIDDEN (__init_array_end = .);
    } > TEXT_REGION
    .fini_array :
    {
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT(.fini_array.*)))
        KEEP (*(.fini_array*))
        PROVIDE_HIDDEN (__fini_array_end = .);
    } > TEXT_REGION

    .ehdr_start :
    {
        PROVIDE_HIDDEN (__ehdr_start = .);
        KEEP (*(SORT(.ehdr_start.*)))
        KEEP (*(.ehdr_start))
        PROVIDE_HIDDEN (__ehdr_end = .);
    } > TEXT_REGION

    .data : {
        *(.data)           /* All `.data` sections */
        *(.data.*)
    } > DATA_REGION       /* Place into DATA_REGION */

    .bss : {
        *(.bss)            /* All `.bss` sections */
        *(.bss.*)
        *(COMMON)          /* Common symbols */
    } > BSS_REGION        /* Place into BSS_REGION */

    _end = .;
}
