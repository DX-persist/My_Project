ENTRY(Reset_Handler)

/* STM32F103C8T6 内存布局 */
MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 64K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 20K
}

/* 栈顶 */
_estack = ORIGIN(RAM) + LENGTH(RAM);

/* 段布局 */
SECTIONS
{
    /* ========== 向量表 + 代码段 ========== */
    .text :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))     /* 向量表必须最前 */
        . = ALIGN(4);
        *(.text*)
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame)
        . = ALIGN(4);
    } > FLASH

    /* ========== 已初始化数据段 ========== */
    _sidata = LOADADDR(.data);   /* Flash 中 .data 的起始地址 */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;             /* RAM 中 .data 起始 */
        *(.data*)
        . = ALIGN(4);
        _edata = .;             /* RAM 中 .data 结束 */
    } > RAM AT > FLASH

    /* ========== 未初始化数据段 ========== */
    .bss :
    {
        . = ALIGN(4);
        _sbss = .;              /* .bss 起始 */
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;              /* .bss 结束 */
    } > RAM

    /* ========== 堆的起始地址 (用于 malloc) ========== */
    . = ALIGN(8);
    _end = .;                   /* 堆起始地址，syscalls.c 的 _sbrk 需要 */
    PROVIDE(end = .);

    /* ARM 属性段 */
    .ARM.attributes 0 : { *(.ARM.attributes) }
}

/* 启动文件用到的符号 */
PROVIDE(_sdata  = _sdata);
PROVIDE(_edata  = _edata);
PROVIDE(_sidata = _sidata);
PROVIDE(_sbss   = _sbss);
PROVIDE(_ebss   = _ebss);
PROVIDE(_estack = _estack);
