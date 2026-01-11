/* STM32F103C8T6 Linker Script */

ENTRY(Reset_Handler)

/* Memory layout */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

/* Stack pointer symbol */
_estack = ORIGIN(RAM) + LENGTH(RAM); /* 栈顶 */

/* Sections */
SECTIONS
{
    /* Vector table + code */
    .text :
    {
        KEEP(*(.isr_vector))      /* 中断向量表 */
        *(.text*)
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)
    } > FLASH

    /* Initialized data */
    .data : AT(ADDR(.text) + SIZEOF(.text))
    {
        _sdata = .;
        *(.data*)
        _edata = .;
    } > RAM

    /* Uninitialized data */
    .bss :
    {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > RAM
}

/* Provide symbols for startup code */
PROVIDE(_sdata   = _sdata);
PROVIDE(_edata   = _edata);
PROVIDE(_sbss    = _sbss);
PROVIDE(_ebss    = _ebss);
PROVIDE(_estack  = _estack);


