#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

__attribute__((section(".myflashend"))) void fun1(void)
{
        printk("Hello from fun 1\n");
}
