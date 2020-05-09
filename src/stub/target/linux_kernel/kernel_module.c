#include <linux/init.h>
#include <asm/pgtable.h>
#include <linux/set_memory.h>
#include <asm/cacheflush.h>
#include <linux/uaccess.h>
#include <linux/moduleloader.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <stub.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("green");
MODULE_DESCRIPTION("mom");
MODULE_VERSION("13.37");


int serial_write(const void* buffer, int size) {
    return 0;
}
int serial_read(void* buffer, int size) {
    return 0;
}                                                                                                                                                                                     

void *big_alloc(size_t size) {
    void *p = kmalloc(size, GFP_KERNEL);
    set_memory_x(p, size / 1024);
    return (void *)((unsigned long)p | 0x20000000);
    /* return __vmalloc(size, GFP_KERNEL, PAGE_KERNEL_EXEC); */
    /* return module_alloc(size); */
    	/* return __vmalloc_node_range(size, 1, MODULE_START, MODULE_END, */
				/* GFP_KERNEL, PAGE_KERNEL, 0, NUMA_NO_NODE, */
				/* __builtin_return_address(0)); */
    /* return vmalloc_exec(size); */
}
void printf(char *s) {
    printk(s);
}

void test2(void) {
}

static int __init init(void) {
    unsigned long args[] = { (unsigned long)&serial_read, (unsigned long)&serial_write, (unsigned long)&printf, (unsigned long)&big_alloc, (unsigned long)&test2 };
    void *new_addr;
    mm_segment_t old_fs;

    printk(KERN_INFO "init kernel module\n");

    new_addr = big_alloc(stub_bin_len);
    memcpy(new_addr, stub_bin, stub_bin_len);

    old_fs = get_fs();
	set_fs(KERNEL_DS);
    flush_icache_range((unsigned long)new_addr, (unsigned long)new_addr + stub_bin_len);
    set_fs(old_fs);

    ((void (*)(void *))new_addr)(args);

    return 0;
}
static void __exit exit(void) {
    printk(KERN_INFO "");
}

module_init(init);
module_exit(exit);

