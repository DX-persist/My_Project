/**
 ******************************************************************************
 * @file    syscalls.c
 * @brief   系统调用桩函数实现 (用于支持 newlib-nano)
 * @note    为 C 标准库提供必要的底层系统调用接口
 ******************************************************************************
 */

#include <sys/stat.h>
#include <errno.h>

/* 外部符号声明 */
extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));

/**
 * @brief  关闭文件 (桩函数)
 * @param  file: 文件描述符
 * @retval -1 表示不支持
 */
int _close(int file)
{
    (void)file;
    return -1;
}

/**
 * @brief  获取文件状态 (桩函数)
 * @param  file: 文件描述符
 * @param  st: 状态结构体指针
 * @retval 0 表示成功
 */
int _fstat(int file, struct stat *st)
{
    (void)file;
    st->st_mode = S_IFCHR;  /* 字符设备 */
    return 0;
}

/**
 * @brief  判断是否为终端设备 (桩函数)
 * @param  file: 文件描述符
 * @retval 1 表示是终端
 */
int _isatty(int file)
{
    (void)file;
    return 1;
}

/**
 * @brief  文件定位 (桩函数)
 * @param  file: 文件描述符
 * @param  ptr: 偏移量
 * @param  dir: 方向
 * @retval 0
 */
int _lseek(int file, int ptr, int dir)
{
    (void)file;
    (void)ptr;
    (void)dir;
    return 0;
}

/**
 * @brief  读取数据 (桩函数)
 * @param  file: 文件描述符
 * @param  ptr: 缓冲区指针
 * @param  len: 读取长度
 * @retval 实际读取的字节数
 * @note   可以在这里实现 USART 接收功能
 */
int _read(int file, char *ptr, int len)
{
    (void)file;
    int count = 0;
    
    for (int i = 0; i < len; i++)
    {
        if (__io_getchar)
        {
            *ptr++ = __io_getchar();
            count++;
        }
        else
        {
            break;
        }
    }
    
    return count;
}

/**
 * @brief  写入数据 (桩函数)
 * @param  file: 文件描述符
 * @param  ptr: 数据指针
 * @param  len: 写入长度
 * @retval 实际写入的字节数
 * @note   可以在这里实现 USART 发送功能，支持 printf
 */
#if 0
int _write(int file, char *ptr, int len)
{
    (void)file;
    int count = 0;
    
    for (int i = 0; i < len; i++)
    {
        if (__io_putchar)
        {
            __io_putchar(*ptr++);
            count++;
        }
        else
        {
            count++;
            ptr++;
        }
    }
    
    return count;
}
#endif
/**
 * @brief  增加堆空间 (用于 malloc)
 * @param  incr: 增加的字节数
 * @retval 新分配内存的起始地址
 * @note   需要在链接脚本中定义 _end 符号
 */
void *_sbrk(int incr)
{
    extern char _end;              /* 由链接器定义，表示 .bss 段结束位置 */
    static char *heap_end = 0;     /* 堆的当前结束位置 */
    char *prev_heap_end;
    
    /* 初始化堆指针 */
    if (heap_end == 0)
    {
        heap_end = &_end;
    }
    
    prev_heap_end = heap_end;
    
    /* 检查堆是否溢出 (可选的安全检查) */
    /* 你需要在链接脚本中定义 _estack 或 __stack */
    // extern char _estack;
    // if (heap_end + incr > &_estack)
    // {
    //     errno = ENOMEM;
    //     return (void *)-1;
    // }
    
    heap_end += incr;
    
    return (void *)prev_heap_end;
}

/**
 * @brief  获取进程 ID (桩函数)
 * @retval 1
 */
int _getpid(void)
{
    return 1;
}

/**
 * @brief  发送信号 (桩函数)
 * @param  pid: 进程 ID
 * @param  sig: 信号
 * @retval -1
 */
int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    errno = EINVAL;
    return -1;
}

/**
 * @brief  退出程序 (桩函数)
 * @param  status: 退出状态
 */
void _exit(int status)
{
    (void)status;
    
    /* 进入死循环 */
    while (1)
    {
        __asm("NOP");
    }
}
