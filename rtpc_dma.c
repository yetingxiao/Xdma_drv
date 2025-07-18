/*
 *    This file is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * */


#include <generated/autoconf.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>       /* printk() */
#include <linux/errno.h>        /* error codes */
#include <linux/ioport.h>
#include <linux/mm.h>
#include <asm/page.h>
#include <asm/io.h>
#include <linux/dma-mapping.h>


MODULE_LICENSE("GPL");

static const char * device_name = "rtpc_dma";


static long device_ioctl(struct file *file, unsigned int number, unsigned long param);
static int device_open(struct inode *inode, struct file *file);
static int device_release(struct inode *inode, struct file *file);
static ssize_t device_write(struct file *file, const char *p, size_t size, loff_t *off);
static ssize_t device_read(struct file *file, char *p, size_t size, loff_t *off);
static int device_mmap(struct file *file, struct vm_area_struct *vma);

static struct file_operations fops = {
    .unlocked_ioctl = device_ioctl,
    .open  = device_open,
    .release = device_release,
    .write = device_write,
    .read = device_read,
    .mmap = device_mmap,
};


#define MAX_MINOR 64
struct pci_dma_data_t {
    void *dma_address;
    dma_addr_t dma_bus_addr;
    char state;
};

static struct pci_dma_data_t pci_dma_data[MAX_MINOR];

static int major=0;


static long device_ioctl(struct file *file, unsigned int number, unsigned long param)
{
    struct inode *inode = file->f_path.dentry->d_inode;
    unsigned int minor = MINOR(inode->i_rdev);
    if (minor >= MAX_MINOR)
        return -ENODEV;

    switch (number)
    {
        case 3:
            __put_user(pci_dma_data[minor].dma_bus_addr, (dma_addr_t*)param);

            break;
        default:
            return -EINVAL;
    }
    return 0;
}

static int device_open(struct inode *inode, struct file *file)
{
    dma_addr_t dma_bus_addr;
    void * dma_address;
    unsigned int minor = MINOR(inode->i_rdev);
    if (minor >= MAX_MINOR)
        return -ENODEV;

    if (pci_dma_data[minor].dma_address)
        return -EBUSY;

    dma_address = dma_alloc_coherent(NULL, 4096, &dma_bus_addr, GFP_KERNEL);


    if (!dma_address || !dma_bus_addr)
    {
        return -ENODEV;
    }

    pci_dma_data[minor].dma_address = dma_address;
    pci_dma_data[minor].dma_bus_addr = dma_bus_addr;
    pci_dma_data[minor].state = 0;
    file->private_data = &pci_dma_data[minor];

    printk("rtpc_dma open %i: dma_address %p %llx\n", minor, dma_address, dma_bus_addr);
    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    unsigned int minor = MINOR(inode->i_rdev);
    if (minor >= MAX_MINOR)
        return -ENODEV;

    dma_free_coherent(NULL, 4096, 
                        pci_dma_data[minor].dma_address, pci_dma_data[minor].dma_bus_addr);


    pci_dma_data[minor].dma_address = NULL;
    pci_dma_data[minor].dma_bus_addr = 0;
    file->private_data = NULL;

    printk("rtpc_dma close %i\n", minor);
    return 0;
}

static ssize_t device_write(struct file *file, const char *p, size_t size, loff_t *off)
{
    int rc;
    struct pci_dma_data_t *pd = file->private_data;

    // printk("%s - ", __func__);
    // printk("size %i\n", size);
    if (size > 4096)
        size = 4096;

    rc = copy_from_user(pd->dma_address, p, size);
    // printk("(%i) %s\n", rc, dummy);


    return size;
}

static ssize_t device_read(struct file *file, char *p, size_t size, loff_t *off)
{
    int rc;
    ssize_t len;
    struct pci_dma_data_t *pd = file->private_data;

    pd->state = !pd->state;
    if (!pd->state)
        return 0;

    len = 64;
    rc = copy_to_user(p, pd->dma_address, len);
    return len;

}

static int device_mmap(struct file *file, struct vm_area_struct *vma)
{
    struct pci_dma_data_t *pd = file->private_data;

    if (remap_pfn_range(vma, vma->vm_start, pd->dma_bus_addr >> PAGE_SHIFT, 4096, vma->vm_page_prot))
    {
        return -EAGAIN;
    }
    return 0;

}


int init_module(void)
{
    int rc;
    rc = register_chrdev(major, device_name, &fops);
    if (rc > 0)
        major = rc;


    printk("%s registered as device %i\n", device_name, major);


    return 0;

}


void cleanup_module(void)
{
    if (major > 0)
        unregister_chrdev(major, device_name);
}



