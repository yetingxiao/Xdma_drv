#include <stddef.h>
#include <linux/printk.h>
volatile void __iomem * 		UART_KERNEL_REGS=NULL;
char IS_KERNEL_MAPPED=1;

void __iomem *	GetUartKernelBase(void) {
	if(IS_KERNEL_MAPPED)
	{
		//IS_KERNEL_MAPPED=0;
		return UART_KERNEL_REGS;
	}else
	{
		pr_err("UART registers unmapped\n");
	}
} 