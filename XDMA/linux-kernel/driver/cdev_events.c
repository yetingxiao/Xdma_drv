/*
 * This file is part of the Xilinx DMA IP Core driver for Linux
 *
 * Copyright (c) 2016-present,  Xilinx, Inc.
 * All rights reserved.
 *
 * This source code is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 */

#define pr_fmt(fmt)     KBUILD_MODNAME ":%s: " fmt, __func__

#include "xdma_cdev.h"

/*
 * character device file operations for events
 */
static ssize_t char_events_read(struct file *file, char __user *buf,
		size_t count, loff_t *pos)
{
	int rv;
	struct xdma_user_irq *user_irq;
	struct xdma_cdev *xcdev = (struct xdma_cdev *)file->private_data;
	u32 events_user;
	unsigned long flags;

	rv = xcdev_check(__func__, xcdev, 0);
	if (rv < 0)
		return rv;
	user_irq = xcdev->user_irq;
	if (!user_irq) {
		pr_info("xcdev 0x%p, user_irq NULL.\n", xcdev);
		return -EINVAL;
	}

	if (count != 4)
		return -EPROTO;

	if (*pos & 3)
		return -EPROTO;

	/*
	 * sleep until any interrupt events have occurred,
	 * or a signal arrived
	 * 
	 * 	wait_event_interruptible(wait_queue_head_t wq, condition)
		wq: 一个等待队列头（wait_queue_head_t），用于管理所有等待特定事件发生的进程。
		condition: 一个布尔表达式。如果表达式为假，进程会继续休眠；如果为真，进程将返回。这个表达式会在进程被唤醒时进行重新评估。

		宏的作用是使当前进程在等待队列上休眠，从而释放 CPU 资源。它会持续休眠，直到：
		1、条件满足：第二个参数中的表达式评估结果为真（非零）。2、信号到达：进程收到一个中断信号（如 SIGINT 或 SIGTERM）。
		如果进程被信号唤醒，wait_event_interruptible 会返回 -ERESTARTSYS。如果条件满足而被唤醒，则返回 0。
	 */
	rv = wait_event_interruptible(user_irq->events_wq,
			user_irq->events_irq != 0);
	if (rv)
		dbg_sg("wait_event_interruptible=%d\n", rv);

	/* wait_event_interruptible() was interrupted by a signal */
	if (rv == -ERESTARTSYS)
		return -ERESTARTSYS;

	/* atomically decide which events are passed to the user */
	spin_lock_irqsave(&user_irq->events_lock, flags);
	events_user = user_irq->events_irq;
	user_irq->events_irq = 0;
	spin_unlock_irqrestore(&user_irq->events_lock, flags);

	rv = copy_to_user(buf, &events_user, 4);
	if (rv)
		dbg_sg("Copy to user failed but continuing\n");

	return 4;
}

static unsigned int char_events_poll(struct file *file, poll_table *wait)
{
	struct xdma_user_irq *user_irq;
	struct xdma_cdev *xcdev = (struct xdma_cdev *)file->private_data;
	unsigned long flags;
	unsigned int mask = 0;
	int rv;

	rv = xcdev_check(__func__, xcdev, 0);
	if (rv < 0)
		return rv;
	user_irq = xcdev->user_irq;
	if (!user_irq) {
		pr_info("xcdev 0x%p, user_irq NULL.\n", xcdev);
		return -EINVAL;
	}
/*
poll_wait(file, &user_irq->events_wq, wait),用于将当前进程加入到一个等待队列。

file: 用户空间文件句柄，通常是设备文件的文件结构体。

&user_irq->events_wq: 这是一个 等待队列头 (wait_queue_head_t)。当驱动程序检测到事件发生时，会调用 wake_up_interruptible() 或类似函数来唤醒所有在这个队列上等待的进程。

wait: poll_table 结构体，poll_wait 会将当前进程的等待信息添加到这个表中。

作用: 这行代码的目的是让用户进程进入睡眠状态，直到驱动程序在中断处理程序中通过 wake_up_interruptible 唤醒它。这是一种高效的等待方式，避免了 CPU 占用
*/
	poll_wait(file, &user_irq->events_wq,  wait);

	spin_lock_irqsave(&user_irq->events_lock, flags);
	if (user_irq->events_irq)
		mask = POLLIN | POLLRDNORM;	/* readable */

	spin_unlock_irqrestore(&user_irq->events_lock, flags);

	return mask;
}

/*
 * character device file operations for the irq events
 */
static const struct file_operations events_fops = {
	.owner = THIS_MODULE,
	.open = char_open,
	.release = char_close,
	.read = char_events_read,
	.poll = char_events_poll,
};

void cdev_event_init(struct xdma_cdev *xcdev)
{
	xcdev->user_irq = &(xcdev->xdev->user_irq[xcdev->bar]);
	cdev_init(&xcdev->cdev, &events_fops);
}
