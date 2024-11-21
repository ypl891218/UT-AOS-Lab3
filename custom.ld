/* Define memory regions */
MEMORY {
    TEXT_REGION (rx) : ORIGIN = 0x70000, LENGTH = 0x200000   /* Code region */
    DATA_REGION (rw) : ORIGIN = 0x280000, LENGTH = 0x10000   /* Initialized data */
    BSS_REGION  (rw) : ORIGIN = 0x290000, LENGTH = 0x10000   /* Uninitialized data */
}

/* Define how sections are placed in memory */
SECTIONS {
    .text : {
        *(.text)           /* All `.text` sections from input files */
        *(.text.*)         /* All `.text.*` sections */
    } > TEXT_REGION       /* Place into TEXT_REGION */

    .data : {
        *(.data)           /* All `.data` sections */
        *(.data.*)
    } > DATA_REGION       /* Place into DATA_REGION */

    .bss : {
        *(.bss)            /* All `.bss` sections */
        *(.bss.*)
        *(COMMON)          /* Common symbols */
    } > BSS_REGION        /* Place into BSS_REGION */
}
