/**
 * @file startup_cortexm4.c
 * @brief Minimal bare-metal Cortex-M4 vector table and buffered semihosting startup for QEMU
 * mps2-an385.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern uint32_t _estack;
extern uint32_t _etext;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

extern int main(void);

void Reset_Handler(void);
void Default_Handler(void);

/* Vector Table for ARM Cortex-M4 */
__attribute__((section(".isr_vector"))) void (*const g_pfnVectors[])(void) = {
    (void (*)(void))((uint32_t)&_estack),
    Reset_Handler,
    Default_Handler, /* NMI */
    Default_Handler, /* HardFault */
    Default_Handler, /* MemManage */
    Default_Handler, /* BusFault */
    Default_Handler, /* UsageFault */
    0,
    0,
    0,
    0,               /* Reserved */
    Default_Handler, /* SVCall */
    Default_Handler, /* Debug Monitor */
    0,               /* Reserved */
    Default_Handler, /* PendSV */
    Default_Handler, /* SysTick */
};

void semihosting_exit(int code)
{
    uint32_t arg[2] = {0x20026, (uint32_t)code};
    __asm__ volatile("mov r0, #0x20\n"
                     "mov r1, %0\n"
                     "bkpt 0xab\n"
                     :
                     : "r"(arg)
                     : "r0", "r1", "memory");
}

void semihosting_write0(const char *str)
{
    if (str == NULL)
        return;
    for (const char *p = str; *p; p++) {
        *(volatile uint32_t *)0x40004000 = (uint32_t)*p;
    }
}

#define BUF_SIZE 512
static char g_io_buf[BUF_SIZE];
static uint16_t g_io_idx = 0;

void qemu_flush_buf(void)
{
    if (g_io_idx > 0) {
        g_io_buf[g_io_idx] = '\0';
        semihosting_write0(g_io_buf);
        g_io_idx = 0;
    }
}

extern uint32_t end;

struct _reent;
void *_sbrk_r(struct _reent *r, ptrdiff_t incr)
{
    (void)r;
    static uint8_t *heap_ptr = NULL;
    if (heap_ptr == NULL) {
        heap_ptr = (uint8_t *)&end;
    }
    size_t aligned_incr = ((size_t)incr + 7U) & ~(size_t)7U;
    uintptr_t stack_limit = (uintptr_t)&_estack - (64 * 1024);
    if ((uintptr_t)heap_ptr + aligned_incr >= stack_limit) {
        return (void *)-1;
    }
    void *prev = heap_ptr;
    heap_ptr += aligned_incr;
    return prev;
}

void *_sbrk(ptrdiff_t incr)
{
    return _sbrk_r(NULL, incr);
}

int _write(int fd, char *ptr, int len)
{
    (void)fd;
    for (int i = 0; i < len; i++) {
        *(volatile uint32_t *)0x40004000 = (uint32_t)ptr[i];
    }
    return len;
}

void __attribute__((noreturn)) __assert_func(const char *file, int line, const char *func,
                                             const char *expr)
{
    char buf[256];
    snprintf(buf, sizeof(buf), "\n=== NEWLIB ASSERT FAILED: %s (%s:%d in %s) ===\n",
             expr ? expr : "?", file ? file : "?", line, func ? func : "?");
    semihosting_write0(buf);
    semihosting_exit(1);
    while (1)
        ;
}

void qemu_flush_dummy(void)
{
}

void semihosting_putc(char c)
{
    *(volatile uint32_t *)0x40004000 = (uint32_t)c;
}

static void hex_to_str(uint32_t val, char *str)
{
    static const char hex[] = "0123456789ABCDEF";
    str[0] = '0';
    str[1] = 'x';
    for (int i = 7; i >= 0; i--) {
        str[2 + i] = hex[val & 0xF];
        val >>= 4;
    }
    str[10] = '\n';
    str[11] = '\0';
}

void HardFault_Handler_C(uint32_t *sp)
{
    qemu_flush_buf();
    uint32_t lr = sp[5];
    uint32_t pc = sp[6];
    char hexbuf[16];
    semihosting_write0("\n=== HARDFAULT AT PC: ");
    hex_to_str(pc, hexbuf);
    semihosting_write0(hexbuf);
    semihosting_write0("=== CALLER LR: ");
    hex_to_str(lr, hexbuf);
    semihosting_write0(hexbuf);
    semihosting_exit(1);
    while (1)
        ;
}

void Default_Handler(void)
{
    __asm__ volatile("tst lr, #4\n"
                     "ite eq\n"
                     "mrseq r0, msp\n"
                     "mrsne r0, psp\n"
                     "b HardFault_Handler_C\n");
}

extern struct _reent _impure_data;
extern struct _reent *_impure_ptr;

struct _reent *__getreent(void)
{
    return &_impure_data;
}

__attribute__((noreturn)) static void run_app(void)
{
    semihosting_write0("Executing main()...\n");
    int ret = main();
    char hexbuf[16];
    semihosting_write0("main() completed with return code: ");
    hex_to_str((uint32_t)ret, hexbuf);
    semihosting_write0(hexbuf);
    qemu_flush_buf();
    int rc = (ret == 0) ? 0 : 1;
    semihosting_exit(rc);
    while (1)
        ;
}

void Reset_Handler(void)
{
    __asm__ volatile("mov r0, sp\n"
                     "bic r0, r0, #7\n"
                     "mov sp, r0\n" ::
                         : "r0");

    extern uint32_t _sidata;
    /* Copy initialized data from FLASH to RAM */
    uint8_t *pSrc = (uint8_t *)&_sidata;
    uint8_t *pDst = (uint8_t *)&_sdata;
    while (pDst < (uint8_t *)&_edata) {
        *pDst++ = *pSrc++;
    }

    /* Zero fill the bss section */
    uint8_t *pBss = (uint8_t *)&_sbss;
    while (pBss < (uint8_t *)&_ebss) {
        *pBss++ = 0;
    }

    /* Configure Cortex-M4 SCB->CCR: Enable 8-byte stack alignment (bit 9), disable unaligned trap
     * (bit 3) */
    *(volatile uint32_t *)0xE000ED14 =
        (*(volatile uint32_t *)0xE000ED14 | (1UL << 9)) & ~(1UL << 3);

    /* Initialize Newlib reentrancy structures & malloc sbrk base */
    extern struct _reent *_impure_ptr;
    extern char *__malloc_sbrk_base;
    _impure_ptr = &_impure_data;
    __malloc_sbrk_base = (char *)&end;

    /* Enable CMSDK APB UART0 TX (0x40004000: BAUDDIV=16 at +0x10, CTRL.TX_EN=1 at +0x08) */
    *(volatile uint32_t *)0x40004010 = 16UL;
    *(volatile uint32_t *)0x40004008 = 1UL;

    /* Enable FPU coprocessor CP10 and CP11 full access */
    *(volatile uint32_t *)0xE000ED88 |= (0xFUL << 20);
    run_app();
}
