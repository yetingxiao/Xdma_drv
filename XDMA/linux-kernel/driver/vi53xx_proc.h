
#ifndef VI53XX_PROC_H
#define VI53XX_PROC_H

#include <linux/hrtimer.h>
#include <linux/irqflags.h>
#include <linux/kernel.h>
#include <linux/kallsyms.h>
#include <linux/module.h>
#include <linux/percpu.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sizes.h>
#include <linux/stacktrace.h>
#include <linux/timer.h>
#include <linux/uaccess.h>
#include <linux/kprobes.h>
#include <linux/version.h>

int create_boards_info_proc(void);
int create_proc_device(void *device, const char *name);
void remove_board_proc(void);

#endif

