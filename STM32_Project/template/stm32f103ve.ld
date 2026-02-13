/* STM32F103VET6 Linker Script
 * Flash: 512K (0x08000000 - 0x0807FFFF)
 * RAM  : 64K  (0x20000000 - 0x2000FFFF)
 */

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}

/* 栈顶 = RAM 末尾 */
_estack = ORIGIN(RAM) + LENGTH(RAM);

ENTRY(Reset_Handler)

SECTIONS
{
    /* ========== 中断向量表 ========== */
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    /* ========== 程序代码 ========== */
    .text :
    {
        . = ALIGN(4);
        *(.text*)
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)
        KEEP (*(.init))
        KEEP (*(.fini))
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    /* ========== 已初始化数据 ========== */
    _sidata = LOADADDR(.data);

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT > FLASH

    /* ========== 未初始化数据 ========== */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    /* ========== 堆和栈空间 ========== */
    ._user_heap_stack :
    {
        . = ALIGN(8);
        PROVIDE ( end = . );
        PROVIDE ( _end = . );
        . = . + 0x1000; /* 4KB heap */
        . = . + 0x1000; /* 4KB stack */
        . = ALIGN(8);
    } > RAM
}

