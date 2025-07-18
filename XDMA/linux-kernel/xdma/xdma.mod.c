#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x8a443098, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x2d3385d3, __VMLINUX_SYMBOL_STR(system_wq) },
	{ 0xd824bcb, __VMLINUX_SYMBOL_STR(kmem_cache_destroy) },
	{ 0x5b3e5384, __VMLINUX_SYMBOL_STR(cdev_del) },
	{ 0xc1541bfd, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x36f7d341, __VMLINUX_SYMBOL_STR(pci_bus_read_config_byte) },
	{ 0xd2b09ce5, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0x2f570610, __VMLINUX_SYMBOL_STR(cdev_init) },
	{ 0xd6ee688f, __VMLINUX_SYMBOL_STR(vmalloc) },
	{ 0x9fff342, __VMLINUX_SYMBOL_STR(__mutex_do_init) },
	{ 0xbd522d8a, __VMLINUX_SYMBOL_STR(pcie_set_readrq) },
	{ 0x83bfa8c9, __VMLINUX_SYMBOL_STR(boot_cpu_data) },
	{ 0xe082781, __VMLINUX_SYMBOL_STR(pci_disable_device) },
	{ 0x56aef86f, __VMLINUX_SYMBOL_STR(pci_disable_msix) },
	{ 0x3ff0c4b7, __VMLINUX_SYMBOL_STR(set_page_dirty_lock) },
	{ 0x12b1323d, __VMLINUX_SYMBOL_STR(__rt_mutex_init) },
	{ 0xc0a3d105, __VMLINUX_SYMBOL_STR(find_next_bit) },
	{ 0xdf566a59, __VMLINUX_SYMBOL_STR(__x86_indirect_thunk_r9) },
	{ 0x44b1d426, __VMLINUX_SYMBOL_STR(__dynamic_pr_debug) },
	{ 0x4c71c1fb, __VMLINUX_SYMBOL_STR(device_destroy) },
	{ 0x238c8b66, __VMLINUX_SYMBOL_STR(kobject_set_name) },
	{ 0x6729d3df, __VMLINUX_SYMBOL_STR(__get_user_4) },
	{ 0x28a2c6f9, __VMLINUX_SYMBOL_STR(_mutex_unlock) },
	{ 0x3fec048f, __VMLINUX_SYMBOL_STR(sg_next) },
	{ 0x1d8a72c3, __VMLINUX_SYMBOL_STR(pci_release_regions) },
	{ 0x1236ec2a, __VMLINUX_SYMBOL_STR(pcie_capability_clear_and_set_word) },
	{ 0x7485e15e, __VMLINUX_SYMBOL_STR(unregister_chrdev_region) },
	{ 0x999e8297, __VMLINUX_SYMBOL_STR(vfree) },
	{ 0xf4030a35, __VMLINUX_SYMBOL_STR(pci_bus_write_config_word) },
	{ 0x4629334c, __VMLINUX_SYMBOL_STR(__preempt_count) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0x73c5ba04, __VMLINUX_SYMBOL_STR(kthread_create_on_node) },
	{ 0x15ba50a6, __VMLINUX_SYMBOL_STR(jiffies) },
	{ 0xaac2af41, __VMLINUX_SYMBOL_STR(rt_spin_lock) },
	{ 0x2a44bac8, __VMLINUX_SYMBOL_STR(rt_spin_unlock) },
	{ 0x689da580, __VMLINUX_SYMBOL_STR(kthread_bind) },
	{ 0x1b9e565c, __VMLINUX_SYMBOL_STR(__init_waitqueue_head) },
	{ 0xc671e369, __VMLINUX_SYMBOL_STR(_copy_to_user) },
	{ 0xfe7c4287, __VMLINUX_SYMBOL_STR(nr_cpu_ids) },
	{ 0xe7c72217, __VMLINUX_SYMBOL_STR(pci_set_master) },
	{ 0x955a832f, __VMLINUX_SYMBOL_STR(___preempt_schedule) },
	{ 0xfb578fc5, __VMLINUX_SYMBOL_STR(memset) },
	{ 0x706c5a65, __VMLINUX_SYMBOL_STR(preempt_count_sub) },
	{ 0xc5e501eb, __VMLINUX_SYMBOL_STR(pci_enable_msix) },
	{ 0xa43b2e62, __VMLINUX_SYMBOL_STR(pci_restore_state) },
	{ 0xf1eb0f8d, __VMLINUX_SYMBOL_STR(pci_iounmap) },
	{ 0x42b31d93, __VMLINUX_SYMBOL_STR(current_task) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x920f3009, __VMLINUX_SYMBOL_STR(kthread_stop) },
	{ 0xd4ab6664, __VMLINUX_SYMBOL_STR(__cpu_online_mask) },
	{ 0xc917e655, __VMLINUX_SYMBOL_STR(debug_smp_processor_id) },
	{ 0x3eb4b112, __VMLINUX_SYMBOL_STR(kmem_cache_free) },
	{ 0x16305289, __VMLINUX_SYMBOL_STR(warn_slowpath_null) },
	{ 0x59246e04, __VMLINUX_SYMBOL_STR(finish_swait) },
	{ 0xd27eb051, __VMLINUX_SYMBOL_STR(device_create) },
	{ 0xd6b8e852, __VMLINUX_SYMBOL_STR(request_threaded_irq) },
	{ 0x6091797f, __VMLINUX_SYMBOL_STR(synchronize_rcu) },
	{ 0x622598b1, __VMLINUX_SYMBOL_STR(init_wait_entry) },
	{ 0x5d3a1a41, __VMLINUX_SYMBOL_STR(__rt_spin_lock_init) },
	{ 0x23b4b0c9, __VMLINUX_SYMBOL_STR(pci_find_capability) },
	{ 0x536655e9, __VMLINUX_SYMBOL_STR(cdev_add) },
	{ 0x98834566, __VMLINUX_SYMBOL_STR(arch_dma_alloc_attrs) },
	{ 0x9c491f60, __VMLINUX_SYMBOL_STR(sg_alloc_table) },
	{ 0x6568f552, __VMLINUX_SYMBOL_STR(kmem_cache_alloc) },
	{ 0xb601be4c, __VMLINUX_SYMBOL_STR(__x86_indirect_thunk_rdx) },
	{ 0xb2fd5ceb, __VMLINUX_SYMBOL_STR(__put_user_4) },
	{ 0xd0b38b11, __VMLINUX_SYMBOL_STR(pci_bus_read_config_word) },
	{ 0x5944d015, __VMLINUX_SYMBOL_STR(__cachemode2pte_tbl) },
	{ 0x15dfdbae, __VMLINUX_SYMBOL_STR(pci_cleanup_aer_uncorrect_error_status) },
	{ 0x37e5fb45, __VMLINUX_SYMBOL_STR(rt_spin_trylock) },
	{ 0x1000e51, __VMLINUX_SYMBOL_STR(schedule) },
	{ 0xd62c833f, __VMLINUX_SYMBOL_STR(schedule_timeout) },
	{ 0x89a1a77e, __VMLINUX_SYMBOL_STR(___ratelimit) },
	{ 0xa202a8e5, __VMLINUX_SYMBOL_STR(kmalloc_order_trace) },
	{ 0x7412e9f0, __VMLINUX_SYMBOL_STR(prepare_to_swait_event) },
	{ 0x2ea2c95c, __VMLINUX_SYMBOL_STR(__x86_indirect_thunk_rax) },
	{ 0x4743ff55, __VMLINUX_SYMBOL_STR(wake_up_process) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
	{ 0xf9ca7bb5, __VMLINUX_SYMBOL_STR(pci_enable_msi_range) },
	{ 0x9e4e568d, __VMLINUX_SYMBOL_STR(pci_unregister_driver) },
	{ 0x897026c6, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x91995028, __VMLINUX_SYMBOL_STR(kmem_cache_create) },
	{ 0x60ca6340, __VMLINUX_SYMBOL_STR(__wake_up) },
	{ 0xb3f7646e, __VMLINUX_SYMBOL_STR(kthread_should_stop) },
	{ 0x3b6a8937, __VMLINUX_SYMBOL_STR(prepare_to_wait_event) },
	{ 0x55bf9330, __VMLINUX_SYMBOL_STR(swake_up) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x22eee1aa, __VMLINUX_SYMBOL_STR(remap_pfn_range) },
	{ 0x44f2b260, __VMLINUX_SYMBOL_STR(pci_request_regions) },
	{ 0x6df1aaf1, __VMLINUX_SYMBOL_STR(kernel_sigaction) },
	{ 0x93f2f5d8, __VMLINUX_SYMBOL_STR(pci_disable_msi) },
	{ 0x57b9c29d, __VMLINUX_SYMBOL_STR(dma_supported) },
	{ 0x7d6beda1, __VMLINUX_SYMBOL_STR(__pci_register_driver) },
	{ 0xf4374310, __VMLINUX_SYMBOL_STR(class_destroy) },
	{ 0xb38fb172, __VMLINUX_SYMBOL_STR(finish_wait) },
	{ 0x398d3a8b, __VMLINUX_SYMBOL_STR(__init_swait_queue_head) },
	{ 0xb1645a2e, __VMLINUX_SYMBOL_STR(sg_free_table) },
	{ 0x2e0d2f7f, __VMLINUX_SYMBOL_STR(queue_work_on) },
	{ 0x28318305, __VMLINUX_SYMBOL_STR(snprintf) },
	{ 0x405246a9, __VMLINUX_SYMBOL_STR(pci_iomap) },
	{ 0xe65325fc, __VMLINUX_SYMBOL_STR(_mutex_lock) },
	{ 0xdd291461, __VMLINUX_SYMBOL_STR(pci_enable_device_mem) },
	{ 0x7f02188f, __VMLINUX_SYMBOL_STR(__msecs_to_jiffies) },
	{ 0x436c2179, __VMLINUX_SYMBOL_STR(iowrite32) },
	{ 0xf229424a, __VMLINUX_SYMBOL_STR(preempt_count_add) },
	{ 0xb2aadf0b, __VMLINUX_SYMBOL_STR(pci_enable_device) },
	{ 0xb5419b40, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0x8c0c7a67, __VMLINUX_SYMBOL_STR(param_ops_uint) },
	{ 0xe50ad56a, __VMLINUX_SYMBOL_STR(__class_create) },
	{ 0xd94824f5, __VMLINUX_SYMBOL_STR(dma_ops) },
	{ 0x29537c9e, __VMLINUX_SYMBOL_STR(alloc_chrdev_region) },
	{ 0xb13bc5c7, __VMLINUX_SYMBOL_STR(__put_page) },
	{ 0xe484e35f, __VMLINUX_SYMBOL_STR(ioread32) },
	{ 0xbe8a5541, __VMLINUX_SYMBOL_STR(get_user_pages_fast) },
	{ 0xf20dabd8, __VMLINUX_SYMBOL_STR(free_irq) },
	{ 0xfa560bfc, __VMLINUX_SYMBOL_STR(pci_save_state) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v000010EEd00009048sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009044sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009042sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009041sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd0000903Fsv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009038sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009028sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009018sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009034sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009024sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009014sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009032sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009022sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009012sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009031sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009021sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00009011sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008011sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008012sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008014sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008018sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008021sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008022sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008024sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008028sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008031sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008032sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008034sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00008038sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007011sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007012sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007014sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007018sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007021sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007022sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007024sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007028sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007031sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007032sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007034sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00007038sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00006828sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00006830sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00006928sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00006930sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00006A28sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00006A30sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00006D30sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00004808sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00004828sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00004908sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00004A28sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00004B28sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd00002808sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000010EEd000016F2sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00001D0Fd0000F000sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v00001D0Fd0000F001sv*sd*bc*sc*i*");

MODULE_INFO(srcversion, "1154660A288125C049FDFC7");
