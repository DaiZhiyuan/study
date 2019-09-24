<!-- TOC -->

- [1. 页表相关的控制寄存器](#1-%e9%a1%b5%e8%a1%a8%e7%9b%b8%e5%85%b3%e7%9a%84%e6%8e%a7%e5%88%b6%e5%af%84%e5%ad%98%e5%99%a8)
  - [1.1. CR0](#11-cr0)
  - [1.2. CR4](#12-cr4)
  - [1.3. EFER(Extended Feature Enable Register)](#13-eferextended-feature-enable-register)
- [2. Example](#2-example)
  - [2.1. walk_page_table.c](#21-walkpagetablec)
  - [2.2. Makefile](#22-makefile)
  - [2.3. dmesg](#23-dmesg)
- [3. 总结](#3-%e6%80%bb%e7%bb%93)

<!-- /TOC -->

# 1. 页表相关的控制寄存器

## 1.1. CR0

- CR0.PE: 开启保护模式，这个标志不会直接开启分页，它仅仅开启分段的保护。
- CR0.PG: 开启分页机制，未开启分页机制时所有线性视作地址物理地址，开启分页必须同时设置PE与PG标志。

## 1.2. CR4

- CR4.PSE: IA32模式，开启4M页面。IA32-e模式需要置位。
- CR4.PAE: IA32模式，物理地址扩展。IA32-e模式需要置位。
- CR4.PGE: 开启全局页特性，它所有页目录和页表项的gloabl标志（bit8），这个global页在TLB中长期保留，使用`mov cr3,reg`指令更新CR3时，这个global页面不会被刷新。

## 1.3. EFER(Extended Feature Enable Register)

- EFER.SCE (System Call Enable): 可以使用syscall/sysret汇编指令快速切换到内核态。
- EFER.LME (Long Mode Enable): 启用长模式。
- EFER.NXE (No-Execute Enable): 可以使用Execution Disbale功能，给某些页定义为不可执行的页面。
- EFER.LMA (Long Mode Active): 当EFER.LME=1并且CR0.PG=1时，处理器置EFER.LMA为1，表示long mode处于激活状态。

# 2. Example 

## 2.1. walk_page_table.c 

```c
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/sched.h>
#include <linux/export.h>
#include <linux/delay.h>

static unsigned long cr0,cr3,cr4,efer;
static unsigned long vaddr;

static void get_pgtable_macro(void)
{
    cr0 = read_cr0();
    cr4 = __read_cr4();
    cr3 = read_cr3_pa();
    rdmsrl(MSR_EFER, efer);

    printk("CR0: %08lx CR3: %08lx CR4: %08lx\n", cr0, cr3, cr4);
    printk("EFER: %08lx\n", efer);
    printk("PAGETABLE_LEVELS = %d\n", PAGETABLE_LEVELS);
    printk("PGDIR_SHIFT = %d\n", PGDIR_SHIFT);
    printk("P4D_SHIFT = %d\n",P4D_SHIFT);
    printk("PUD_SHIFT = %d\n", PUD_SHIFT);
    printk("PMD_SHIFT = %d\n", PMD_SHIFT);
    printk("PAGE_SHIFT = %d\n", PAGE_SHIFT);
 
    printk("PTRS_PER_PGD = %d\n", PTRS_PER_PGD);
    printk("PTRS_PER_P4D = %d\n", PTRS_PER_P4D);
    printk("PTRS_PER_PUD = %d\n", PTRS_PER_PUD);
    printk("PTRS_PER_PMD = %d\n", PTRS_PER_PMD);
    printk("PTRS_PER_PTE = %d\n", PTRS_PER_PTE);
    printk("PAGE_MASK = 0x%lx\n", PAGE_MASK);
}
 
static unsigned long vaddr2paddr(unsigned long vaddr)
{
    pgd_t *pgd;
    p4d_t *p4d;
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    unsigned long paddr = 0;
    unsigned long page_addr = 0;
    unsigned long page_offset = 0;
    pgd = pgd_offset(current->mm,vaddr);
    printk("pgd_val = 0x%lx, pgd_index = %lu\n", pgd_val(*pgd),pgd_index(vaddr));
    if (pgd_none(*pgd)){
        printk("not mapped in pgd\n");
        return -1;
    }

    p4d = p4d_offset(pgd, vaddr);
    printk("p4d_val = 0x%lx, p4d_index = %lu\n", p4d_val(*p4d),p4d_index(vaddr));
    if(p4d_none(*p4d))
    { 
        printk("not mapped in p4d\n");
        return -1;
    }

    pud = pud_offset(p4d, vaddr);
    printk("pud_val = 0x%lx, pud_index = %lu\n", pud_val(*pud),pud_index(vaddr));
    if (pud_none(*pud)) {
        printk("not mapped in pud\n");
        return -1;
    }
 
    pmd = pmd_offset(pud, vaddr);
    printk("pmd_val = 0x%lx, pmd_index = %lu\n", pmd_val(*pmd),pmd_index(vaddr));
    if (pmd_none(*pmd)) {
        printk("not mapped in pmd\n");
        return -1;
    }
 
    pte = pte_offset_kernel(pmd, vaddr);
    printk("pte_val = 0x%lx, pte_index = %lu\n", pte_val(*pte),pte_index(vaddr));

    if (pte_none(*pte)) {
        printk("not mapped in pte\n");
        return -1;
    }
    page_addr = pte_val(*pte) & PAGE_MASK; // first 52bits
    page_offset = vaddr & ~PAGE_MASK;      // last 12bits
    paddr = page_addr | page_offset;       // join 64bits
    printk("page_addr = %lx, page_offset = %lx\n", page_addr, page_offset);
    printk("vaddr = %lx, paddr = %lx\n", vaddr, paddr);
    return paddr;
}

static int __init v2p_init(void)
{
    unsigned long vaddr = 0 ;
    printk("vaddr to paddr module is running..\n");
    get_pgtable_macro();
    printk("\n");
    vaddr = __get_free_page(GFP_KERNEL);
    if (vaddr == 0) {
        printk("__get_free_page failed..\n");
        return 0;
    }
    sprintf((char *)vaddr, "hello world from kernel");
    printk("get_page_vaddr=0x%lx\n", vaddr);
    vaddr2paddr(vaddr);
    ssleep(60);
    return 0;
}
static void __exit v2p_exit(void)
{
    printk("vaddr to paddr module is leaving..\n");
    free_page(vaddr);
}

module_init(v2p_init);
module_exit(v2p_exit);
MODULE_LICENSE("GPL"); 
```

## 2.2. Makefile

```Makefile
obj-m:= walk_page_table.o
PWD:= $(shell pwd)
KERNELDIR:= /lib/modules/$(shell uname -r)/build

all:
	make -C $(KERNELDIR)  M=$(PWD) modules
clean:
	@rm -rf *.o *.mod.c *.mod.o *.ko *.order *.symvers .*.cmd .tmp_versions
```

## 2.3. dmesg
```
[ 3198.189824] PGDIR_SHIFT = 39
[ 3198.189825] P4D_SHIFT = 39
[ 3198.189826] PUD_SHIFT = 30
[ 3198.189827] PMD_SHIFT = 21
[ 3198.189828] PAGE_SHIFT = 12
[ 3198.189829] PTRS_PER_PGD = 512
[ 3198.189830] PTRS_PER_P4D = 1
[ 3198.189831] PTRS_PER_PUD = 512
[ 3198.189832] PTRS_PER_PMD = 512
[ 3198.189833] PTRS_PER_PTE = 512
[ 3198.189835] PAGE_MASK = 0xfffffffffffff000

[ 3198.189838] get_page_vaddr=0xffff97213f1db000
[ 3198.189840] pgd_val = 0x133b2a067, pgd_index = 302   /* 0x067 = PRESENT+RW+DIRTY+ACCESSED */
[ 3198.189841] p4d_val = 0x133b2a067, p4d_index = 0
[ 3198.189843] pud_val = 0x14940e063, pud_index = 132   /* 0x063 = PRESENT+RW+USER+DIRTY+ACCESSED */
[ 3198.189844] pmd_val = 0x13f7dc063, pmd_index = 504   
[ 3198.189846] pte_val = 0x800000013f1db063, pte_index = 475    /* PAGE_KERNEL, No execute flags(bit63) + PRESENT+RW+USER+DIRTY+ACCESSED*/
[ 3198.189848] page_addr = 800000013f1db000, page_offset = 0
[ 3198.189849] vaddr = ffff97213f1db000, paddr = 800000013f1db000
```
# 3. 总结
Paging Structre | Entry Name | Linux Name| Physical Address of Structure | Bits Selecting Entry | Page Mapping
---|---|---|---|---|---
PMl4 table | PML4E | pgd | CR3 | 47:39 | PS must be 0
Page-directory-pointer table | PDPTE |pud | PML4E | 38:30 | 1G page if PS=1
Page directory | PDE |pmd| PDPTE | 29:21 | 2M page if PS=1
page table | PTE | pte | PDE | 20:12 | 4K page
