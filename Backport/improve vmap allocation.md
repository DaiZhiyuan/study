
<!-- TOC -->

- [Performance: improve vmalloc allocation allocation](#performance-improve-vmalloc-allocation-allocation)
    - [[RFC PATCH 0/2] improve vmalloc allocation](#rfc-patch-02-improve-vmalloc-allocation)
    - [[RFC PATCH 1/2] mm/vmalloc: keep track of free blocks for allocation](#rfc-patch-12-mmvmalloc-keep-track-of-free-blocks-for-allocation)
    - [[RFC PATCH 2/2] mm: add priority threshold to __purge_vmap_area_lazy()](#rfc-patch-22-mm-add-priority-threshold-to-__purge_vmap_area_lazy)

<!-- /TOC -->

# Performance: improve vmalloc allocation allocation

## [RFC PATCH 0/2] improve vmalloc allocation

```patch
Objective
---------
Initiative of improving vmalloc allocator comes from getting many issues
related to allocation time, i.e. sometimes it is terribly slow. As a result
many workloads which are sensitive for long (more than 1 millisecond) preemption
off scenario are affected by that slowness(test cases like UI or audio, etc.).

The problem is that, currently an allocation of the new VA area is done over
busy list iteration until a suitable hole is found between two busy areas.
Therefore each new allocation causes the list being grown. Due to long list
and different permissive parameters an allocation can take a long time on
embedded devices(milliseconds).

Description
-----------
This approach keeps track of free blocks and allocation is done over free list
iteration instead. During initialization phase the vmalloc memory layout is
organized into one free area(can be more) building free double linked list
within VMALLOC_START-VMALLOC_END range.

Proposed algorithm uses red-black tree that keeps blocks sorted by their offsets
in pair with linked list keeping the free space in order of increasing addresses.

Allocation. It uses a first-fit policy. To allocate a new block a search is done
over free list areas until a first suitable block is large enough to encompass
the requested size. If the block is bigger than requested size - it is split.

A free block can be split by three different ways. Their names are FL_FIT_TYPE,
LE_FIT_TYPE/RE_FIT_TYPE and NE_FIT_TYPE, i.e. they correspond to how requested
size and alignment fit to a free block.

FL_FIT_TYPE - in this case a free block is just removed from the free list/tree
because it fully fits. Comparing with current design there is an extra work with
rb-tree updating.

LE_FIT_TYPE/RE_FIT_TYPE - left/right edges fit. In this case what we do is
just cutting a free block. It is as fast as a current design. Most of the vmalloc
allocations just end up with this case, because the edge is always aligned to 1.

NE_FIT_TYPE - Is much less common case. Basically it happens when requested size
and alignment does not fit left nor right edges, i.e. it is between them. In this
case during splitting we have to build a remaining left free area and place it
back to the free list/tree.

Comparing with current design there are two extra steps. First one is we have to
allocate a new vmap_area structure. Second one we have to insert that remaining 
free block to the address sorted list/tree.

In order to optimize a first case there is a cache with free_vmap objects. Instead
of allocating from slab we just take an object from the cache and reuse it.

Second one is pretty optimized. Since we know a start point in the tree we do not
do a search from the top. Instead a traversal begins from a rb-tree node we split.

De-allocation. Red-black tree allows efficiently find a spot in the tree whereas
a linked list allows fast access to neighbors, thus a fast merge of de-allocated
memory chunks with existing free blocks creating large coalesced areas. It means
comparing with current design there is an extra step we have to done when a block
is freed.

In order to optimize a merge logic a free vmap area is not inserted straight
away into free structures. Instead we find a place in the rbtree where a free
block potentially can be inserted. Its parent node and left or right direction.
Knowing that, we can identify future next/prev list nodes, thus at this point
it becomes possible to check if a block can be merged. If not, just link it.

Test environment
----------------
I have used two systems to test. One is i5-3320M CPU @ 2.60GHz and another
is HiKey960(arm64) board. i5-3320M runs on 4.18 kernel, whereas Hikey960
uses 4.15 linaro kernel.

i5-3320M:
set performance governor
echo 0 > /proc/sys/kernel/nmi_watchdog
echo -1 > /proc/sys/kernel/perf_event_paranoid
echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo

Stability and stress tests
--------------------------
As for stress testing of the new approach, i wrote a small patch with
different test cases and allocations methods to make a pressure on
vmalloc subsystem. In short, it runs different kind of allocation
tests simultaneously on each online CPU with different run-time(few days).

A test-suite patch you can find here, it is based on 4.18 kernel.
ftp://vps418301.ovh.net/incoming/0001-mm-vmalloc-stress-test-suite-v4.18.patch

Below are some issues i run into during stability check phase(default kernel).

1) This Kernel BUG can be triggered by align_shift_alloc_test() stress test.
See it in test-suite patch:

<snip>
[66970.279289] kernel BUG at mm/vmalloc.c:512!
[66970.279363] invalid opcode: 0000 [#1] PREEMPT SMP PTI
[66970.279411] CPU: 1 PID: 652 Comm: insmod Tainted: G           O      4.18.0+ #741
[66970.279463] Hardware name: QEMU Standard PC (i440FX + PIIX, 1996), BIOS 1.10.2-1 04/01/2014
[66970.279525] RIP: 0010:alloc_vmap_area+0x358/0x370
<snip>

Patched version does not suffer from that BUG().

2) This one has been introduced by commit 763b218ddfaf. Which introduced some
preemption points into __purge_vmap_area_lazy(). Under heavy simultaneous
allocations/releases number of lazy pages can easily go beyond millions
hitting out_of_memory -> panic or making operations like: allocation, lookup,
unmap, remove, etc. much slower.

It is fixed by second commit in this series. Please see more description in
the commit message of the patch.

3) This one is related to PCPU allocator(see pcpu_alloc_test()). In that
stress test case i see that SUnreclaim(/proc/meminfo) parameter gets increased,
i.e. there is a memory leek somewhere in percpu allocator. It sounds like
a memory that is allocated by pcpu_get_vm_areas() sometimes is not freed.
Resulting in memory leaking or "Kernel panic":

---[ end Kernel panic - not syncing: Out of memory and no killable processes...

There is no fix for that.

Performance test results
------------------------
I run 5 different tests to compare the performance between the new approach
and current one. Since there are three different type of allocations i wanted
to compare them with default version, apart of those there are two extra.
One allocates in long busy list condition. Another one does random number
of pages allocation(will not post here to keep it short).

- reboot the system;
- do three iteration of full run. One run is 5 tests;
- calculate average of three run(microseconds).

i5-3320M:
le_fit_alloc_test                b_fit_alloc_test
1218459 vs 1146597 diff 5%       972322 vs 1008655 diff -3.74%
1219721 vs 1145212 diff 6%      1013817 vs  994195 diff  1.94%
1226255 vs 1142134 diff 6%      1002057 vs  993364 diff  0.87%
1239828 vs 1144809 diff 7%       985092 vs  977549 diff  0.77%
1232131 vs 1144775 diff 7%      1031320 vs  999952 diff  3.04%

ne_fit_alloc_test                long_busy_list_alloc_test
2056336 vs 2043121 diff 0.64%    55866315 vs 15037680 diff 73%
2043136 vs 2041888 diff 0.06%    57601435 vs 14809454 diff 74%
2042384 vs 2040181 diff 0.11%    52612371 vs 14550292 diff 72%
2041287 vs 2038905 diff 0.12%    48894648 vs 14769538 diff 69%
2039014 vs 2038632 diff 0.02%    55718063 vs 14727350 diff 73%

Hikey960:
le_fit_alloc_test                b_fit_alloc_test
2382168 vs 2115617 diff 11.19%   2864080 vs 2081309 diff 27.33%
2772909 vs 2114988 diff 23.73%   2968612 vs 2062716 diff 30.52%
2772579 vs 2113069 diff 23.79%   2748495 vs 2106658 diff 23.35%
2770596 vs 2111823 diff 23.78%   2966023 vs 2071139 diff 30.17%
2759768 vs 2111868 diff 23.48%   2765568 vs 2125216 diff 23.15%

ne_fit_alloc_test                long_busy_list_alloc_test
4353846 vs 4241838 diff  2.57    239789754 vs 33364305 diff 86%
4133506 vs 4241615 diff -2.62    778283461 vs 34551548 diff 95%
4134769 vs 4240714 diff -2.56    210244212 vs 33467529 diff 84%
4132224 vs 4242281 diff -2.66    429232377 vs 33307173 diff 92%
4410969 vs 4240864 diff  3.86    527560967 vs 33661115 diff 93%

Almost all results are better. Full data and the test module you can find here:

ftp://vps418301.ovh.net/incoming/vmalloc_test_module.tar.bz2
ftp://vps418301.ovh.net/incoming/HiKey960_test_result.txt
ftp://vps418301.ovh.net/incoming/i5-3320M_test_result.txt

Conclusion
----------
According to provided results and my subjective opinion, it is worth to organize
and maintain a free list and do an allocation based on it.

Appreciate for any valuable comments and sorry for the long description :)

Best Regards,
Uladzislau Rezki

Uladzislau Rezki (Sony) (2):
  mm/vmalloc: keep track of free blocks for allocation
  mm: add priority threshold to __purge_vmap_area_lazy()

 include/linux/vmalloc.h |   2 +-
 mm/vmalloc.c            | 850 ++++++++++++++++++++++++++++++++++++++----------
 2 files changed, 676 insertions(+), 176 deletions(-)

-- 
2.11.0
```

## [RFC PATCH 1/2] mm/vmalloc: keep track of free blocks for allocation

```patch
Currently an allocation of the new VA area is done over
busy list iteration until a suitable hole is found between
two busy areas. Therefore each new allocation causes the
list being grown. Due to long list and different permissive
parameters an allocation can take a long time on embedded
devices(milliseconds).

This patch organizes the vmalloc memory layout into free
areas of the VMALLOC_START-VMALLOC_END range. It uses a
red-black tree that keeps blocks sorted by their offsets
in pair with linked list keeping the free space in order
of increasing addresses.

Allocation: to allocate a new block a search is done over
free list areas until a suitable block is large enough to
encompass the requested size. If the block is bigger than
requested size - it is split.

De-allocation: red-black tree allows efficiently find a
spot in the tree whereas a linked list allows fast merge
of de-allocated memory chunks with existing free blocks
creating large coalesced areas.

model name: Intel(R) Core(TM) i5-3320M CPU @ 2.60GHz

test_1:
<measure this loop time>
for (n = 0; n < 1000000; n++) {
    void *ptr_1 = vmalloc(3 * PAGE_SIZE);
    *((__u8 *)ptr_1) = 0; /* Pretend we used the mem */
    vfree(ptr_1);
}
<measure this loop time>

1218459(us) vs 1146597(us) 5%
1219721(us) vs 1145212(us) 6%
1226255(us) vs 1142134(us) 6%
1239828(us) vs 1144809(us) 7%
1232131(us) vs 1144775(us) 7%

test_2:
for (n = 0; n < 15000; n++)
    ptr[n] = vmalloc(1 * PAGE_SIZE);

<measure this loop time>
for (n = 0; n < 1000000; n++) {
    void *ptr_1 = vmalloc(100 * PAGE_SIZE);
    void *ptr_2 = vmalloc(1 * PAGE_SIZE);
    *((__u8 *)ptr_1) = 0; /* Pretend we used the mem */
    *((__u8 *)ptr_2) = 1; /* Pretend we used the mem */

    vfree(ptr_1);
    vfree(ptr_2);
}
<measure this loop time>

55866315(us) vs 15037680(us) 73%
57601435(us) vs 14809454(us) 74%
52612371(us) vs 14550292(us) 72%
48894648(us) vs 14769538(us) 69%
55718063(us) vs 14727350(us) 73%

Signed-off-by: Uladzislau Rezki (Sony) <urezki@gmail.com>
---
 include/linux/vmalloc.h |   2 +-
 mm/vmalloc.c            | 836 ++++++++++++++++++++++++++++++++++++++----------
 2 files changed, 668 insertions(+), 170 deletions(-)

diff --git a/include/linux/vmalloc.h b/include/linux/vmalloc.h
index 398e9c95cd61..01a73d4795f4 100644
--- a/include/linux/vmalloc.h
+++ b/include/linux/vmalloc.h
@@ -46,11 +46,11 @@ struct vmap_area {
 	unsigned long va_start;
 	unsigned long va_end;
 	unsigned long flags;
+	unsigned long align;
 	struct rb_node rb_node;         /* address sorted rbtree */
 	struct list_head list;          /* address sorted list */
 	struct llist_node purge_list;    /* "lazy purge" list */
 	struct vm_struct *vm;
-	struct rcu_head rcu_head;
 };
 
 /*
diff --git a/mm/vmalloc.c b/mm/vmalloc.c
index cfea25be7754..a7f257540a05 100644
--- a/mm/vmalloc.c
+++ b/mm/vmalloc.c
@@ -326,12 +326,57 @@ EXPORT_SYMBOL(vmalloc_to_pfn);
 #define VM_LAZY_FREE	0x02
 #define VM_VM_AREA	0x04
 
+/*
+ * Define default padding for alignment purposes.
+ */
+#define VMALLOC_MAX_PADDING THREAD_ALIGN
+
 static DEFINE_SPINLOCK(vmap_area_lock);
 /* Export for kexec only */
 LIST_HEAD(vmap_area_list);
 static LLIST_HEAD(vmap_purge_list);
 static struct rb_root vmap_area_root = RB_ROOT;
 
+/*
+ * This linked list is used in pair with free_vmap_area_root.
+ * It makes it possible of fast accessing to next/prev nodes
+ * to perform coalescing.
+ */
+static LIST_HEAD(free_vmap_area_list);
+
+/*
+ * This red-black tree is used for storing address-sorted
+ * vmap areas during free operation. Sorting is done using
+ * va_start address. We make use of it to merge a VA with
+ * its prev/next neighbors.
+ */
+static struct rb_root free_vmap_area_root = RB_ROOT;
+
+/*
+ * This cache list is used for keeping free vmap_area objects.
+ * Basically, when free VA area is split, a remaining space has
+ * to be placed back to free list/tree structures. Instead of
+ * allocating from slab we reuse vmap_area objects from this
+ * cache.
+ */
+static LIST_HEAD(free_va_cache);
+
+/*
+ * This is a cache size counter. A maximum cache size depends on
+ * lazy_max_pages() and is not higher than lazy_max_pages() / 2.
+ * A "purge layer" drains free areas feeding the cache back when
+ * the threshold is crossed.
+ */
+static unsigned long free_va_cache_size;
+
+/*
+ * For vmalloc specific area allocation.
+ */
+static struct vmap_area *last_free_area;
+static unsigned long last_alloc_vstart;
+static unsigned long last_alloc_align;
+static unsigned long free_va_max_size;
+
 /* The vmap cache globals are protected by vmap_area_lock */
 static struct rb_node *free_vmap_cache;
 static unsigned long cached_hole_size;
@@ -340,6 +385,10 @@ static unsigned long cached_align;
 
 static unsigned long vmap_area_pcpu_hole;
 
+static void purge_vmap_area_lazy(void);
+static BLOCKING_NOTIFIER_HEAD(vmap_notify_list);
+static unsigned long lazy_max_pages(void);
+
 static struct vmap_area *__find_vmap_area(unsigned long addr)
 {
 	struct rb_node *n = vmap_area_root.rb_node;
@@ -359,41 +408,411 @@ static struct vmap_area *__find_vmap_area(unsigned long addr)
 	return NULL;
 }
 
-static void __insert_vmap_area(struct vmap_area *va)
+static inline bool
+__put_free_va_to_cache(struct vmap_area *va)
+{
+	if (free_va_cache_size < (lazy_max_pages() >> 1)) {
+		list_add(&va->list, &free_va_cache);
+		free_va_cache_size++;
+		return true;
+	}
+
+	return false;
+}
+
+static inline void *
+__get_free_va_from_cache(void)
 {
-	struct rb_node **p = &vmap_area_root.rb_node;
-	struct rb_node *parent = NULL;
-	struct rb_node *tmp;
+	struct vmap_area *va;
+
+	va = list_first_entry_or_null(&free_va_cache,
+			struct vmap_area, list);
+
+	if (va) {
+		list_del(&va->list);
+		free_va_cache_size--;
+	}
 
-	while (*p) {
-		struct vmap_area *tmp_va;
+	return va;
+}
+
+static inline void
+__find_va_slot(struct vmap_area *va,
+	struct rb_root *root, struct rb_node *from,
+	struct rb_node **parent, struct rb_node ***link)
+{
+	struct vmap_area *tmp_va;
+
+	if (root) {
+		*link = &root->rb_node;
+		if (unlikely(!**link)) {
+			*parent = NULL;
+			return;
+		}
+	} else {
+		*link = &from;
+	}
 
-		parent = *p;
-		tmp_va = rb_entry(parent, struct vmap_area, rb_node);
+	do {
+		tmp_va = rb_entry(**link, struct vmap_area, rb_node);
 		if (va->va_start < tmp_va->va_end)
-			p = &(*p)->rb_left;
+			*link = &(**link)->rb_left;
 		else if (va->va_end > tmp_va->va_start)
-			p = &(*p)->rb_right;
+			*link = &(**link)->rb_right;
 		else
 			BUG();
+	} while (**link);
+
+	/*
+	 * Return back addresses of parent node of VA and
+	 * parent's left/right link for further inserting.
+	 */
+	*parent = &tmp_va->rb_node;
+}
+
+static inline void
+__find_va_free_siblings(struct rb_node *parent, struct rb_node **link,
+	struct list_head **prev, struct list_head **next)
+{
+	struct list_head *list;
+
+	if (likely(parent)) {
+		list = &rb_entry(parent, struct vmap_area, rb_node)->list;
+		if (&parent->rb_right == link) {
+			*next = list->next;
+			*prev = list;
+		} else {
+			*prev = list->prev;
+			*next = list;
+		}
+	} else {
+		/*
+		 * The red-black tree where we try to find VA neighbors
+		 * before merging or inserting is empty, i.e. it means
+		 * there is no free vmalloc space. Normally it does not
+		 * happen but we handle this case anyway.
+		 */
+		*next = *prev = &free_vmap_area_list;
+	}
+}
+
+static inline void
+__link_va(struct vmap_area *va, struct rb_root *root,
+	struct rb_node *parent, struct rb_node **link, struct list_head *head)
+{
+	/*
+	 * VA is still not in the list, but we can
+	 * identify its future previous list_head node.
+	 */
+	if (likely(parent)) {
+		head = &rb_entry(parent, struct vmap_area, rb_node)->list;
+		if (&parent->rb_right != link)
+			head = head->prev;
 	}
 
-	rb_link_node(&va->rb_node, parent, p);
-	rb_insert_color(&va->rb_node, &vmap_area_root);
+	/* Insert to the rb-tree */
+	rb_link_node(&va->rb_node, parent, link);
+	rb_insert_color(&va->rb_node, root);
 
-	/* address-sort this list */
-	tmp = rb_prev(&va->rb_node);
-	if (tmp) {
-		struct vmap_area *prev;
-		prev = rb_entry(tmp, struct vmap_area, rb_node);
-		list_add_rcu(&va->list, &prev->list);
-	} else
-		list_add_rcu(&va->list, &vmap_area_list);
+	/* Address-sort this list */
+	list_add(&va->list, head);
 }
 
-static void purge_vmap_area_lazy(void);
+static inline void
+__unlink_va(struct vmap_area *va, struct rb_root *root)
+{
+	/*
+	 * During merging a VA node can be empty, therefore
+	 * not linked with the tree nor list. Just check it.
+	 */
+	if (!RB_EMPTY_NODE(&va->rb_node)) {
+		rb_erase(&va->rb_node, root);
+		list_del(&va->list);
+	}
+}
 
-static BLOCKING_NOTIFIER_HEAD(vmap_notify_list);
+static void
+__insert_vmap_area(struct vmap_area *va,
+	struct rb_root *root, struct list_head *head)
+{
+	struct rb_node **link;
+	struct rb_node *parent;
+
+	__find_va_slot(va, root, NULL, &parent, &link);
+	__link_va(va, root, parent, link, head);
+}
+
+static inline void
+__remove_vmap_area(struct vmap_area *va, struct rb_root *root)
+{
+	__unlink_va(va, root);
+
+	/*
+	 * Fill the cache. If it is full, we just free VA.
+	 */
+	if (!__put_free_va_to_cache(va))
+		kfree(va);
+}
+
+/*
+ * Merge de-allocated chunk of VA memory with previous
+ * and next free blocks. Either a pointer to the new
+ * merged area is returned if coalesce is done or VA
+ * area if inserting is done.
+ */
+static inline struct vmap_area *
+__merge_add_free_va_area(struct vmap_area *va,
+	struct rb_root *root, struct list_head *head)
+{
+	struct vmap_area *sibling;
+	struct list_head *next, *prev;
+	struct rb_node **link;
+	struct rb_node *parent;
+	bool merged = false;
+
+	/*
+	 * To perform merging we have to restore an area which belongs
+	 * to this VA if the allocation has been done with specific align
+	 * value. In case of PCPU allocations nothing is changed.
+	 */
+	if (va->align <= VMALLOC_MAX_PADDING)
+		va->va_end = ALIGN(va->va_end, va->align);
+
+	/*
+	 * Find a place in the tree where VA potentially will be
+	 * inserted, unless it is merged with its sibling/siblings.
+	 */
+	__find_va_slot(va, root, NULL, &parent, &link);
+
+	/*
+	 * Get next/prev nodes of VA to check if merging can be done.
+	 */
+	__find_va_free_siblings(parent, link, &prev, &next);
+
+	/*
+	 * start            end
+	 * |                |
+	 * |<------VA------>|<-----Next----->|
+	 *                  |                |
+	 *                  start            end
+	 */
+	if (next != head) {
+		sibling = list_entry(next, struct vmap_area, list);
+		if (sibling->va_start == va->va_end) {
+			sibling->va_start = va->va_start;
+			__remove_vmap_area(va, root);
+
+			/* Point to the new merged area. */
+			va = sibling;
+			merged = true;
+		}
+	}
+
+	/*
+	 * start            end
+	 * |                |
+	 * |<-----Prev----->|<------VA------>|
+	 *                  |                |
+	 *                  start            end
+	 */
+	if (prev != head) {
+		sibling = list_entry(prev, struct vmap_area, list);
+		if (sibling->va_end == va->va_start) {
+			sibling->va_end = va->va_end;
+			__remove_vmap_area(va, root);
+
+			/* Point to the new merged area. */
+			va = sibling;
+			merged = true;
+		}
+	}
+
+	if (!merged)
+		__link_va(va, root, parent, link, head);
+
+	return va;
+}
+
+enum alloc_fit_type {
+	NOTHING_FIT = 0,
+	FL_FIT_TYPE = 1,	/* full fit */
+	LE_FIT_TYPE = 2,	/* left edge fit */
+	RE_FIT_TYPE = 3,	/* right edge fit */
+	NE_FIT_TYPE = 4		/* no edge fit */
+};
+
+static inline unsigned long
+alloc_vmalloc_area(struct vmap_area **fl_fit_va, unsigned long size,
+	unsigned long align, unsigned long vstart, unsigned long vend)
+{
+	unsigned long nva_start_addr;
+	struct vmap_area *va, *lva;
+	struct rb_node *parent;
+	struct rb_node **link;
+	u8 fit_type;
+
+	va = last_free_area;
+	fit_type = NOTHING_FIT;
+	*fl_fit_va = NULL;
+
+	/*
+	 * Use aligned size if the align value is within
+	 * allowed padding range. This is done to reduce
+	 * external fragmentation.
+	 */
+	if (align <= VMALLOC_MAX_PADDING)
+		size = ALIGN(size, align);
+
+	if (!last_free_area || size < free_va_max_size ||
+			vstart < last_alloc_vstart ||
+			align < last_alloc_align) {
+		va = list_first_entry_or_null(&free_vmap_area_list,
+				struct vmap_area, list);
+
+		if (unlikely(!va))
+			return vend;
+
+		free_va_max_size = 0;
+		last_free_area = NULL;
+	}
+
+	nva_start_addr = ALIGN(vstart, align);
+	list_for_each_entry_from(va, &free_vmap_area_list, list) {
+		if (va->va_start > vstart)
+			nva_start_addr = ALIGN(va->va_start, align);
+
+		/*
+		 * Sanity test for following scenarios:
+		 * - overflow, due to big size;
+		 * - vend restriction check;
+		 * - vstart check, due to big align.
+		 */
+		if (nva_start_addr + size < nva_start_addr ||
+				nva_start_addr + size > vend ||
+					nva_start_addr < vstart)
+			break;
+
+		/*
+		 * VA does not fit to requested parameters. In this case we
+		 * calculate max available aligned size if nva_start_addr is
+		 * within this VA.
+		 */
+		if (nva_start_addr + size > va->va_end) {
+			if (nva_start_addr < va->va_end)
+				free_va_max_size = max(free_va_max_size,
+						va->va_end - nva_start_addr);
+			continue;
+		}
+
+		/* Classify what we have found. */
+		if (va->va_start == nva_start_addr) {
+			if (va->va_end == nva_start_addr + size)
+				fit_type = FL_FIT_TYPE;
+			else
+				fit_type = LE_FIT_TYPE;
+		} else if (va->va_end == nva_start_addr + size) {
+			fit_type = RE_FIT_TYPE;
+		} else {
+			fit_type = NE_FIT_TYPE;
+		}
+
+		last_free_area = va;
+		last_alloc_vstart = vstart;
+		last_alloc_align = align;
+		break;
+	}
+
+	if (fit_type == FL_FIT_TYPE) {
+		/*
+		 * No need to split VA, it fully fits.
+		 *
+		 * |               |
+		 * V      NVA      V
+		 * |---------------|
+		 */
+		if (va->list.prev != &free_vmap_area_list)
+			last_free_area = list_prev_entry(va, list);
+		else
+			last_free_area = NULL;
+
+		__unlink_va(va, &free_vmap_area_root);
+		*fl_fit_va = va;
+	} else if (fit_type == LE_FIT_TYPE) {
+		/*
+		 * Split left edge fit VA.
+		 *
+		 * |       |
+		 * V  NVA  V   R
+		 * |-------|-------|
+		 */
+		va->va_start += size;
+	} else if (fit_type == RE_FIT_TYPE) {
+		/*
+		 * Split right edge fit VA.
+		 *
+		 *         |       |
+		 *     L   V  NVA  V
+		 * |-------|-------|
+		 */
+		va->va_end = nva_start_addr;
+	} else if (fit_type == NE_FIT_TYPE) {
+		/*
+		 * Split no edge fit VA.
+		 *
+		 *     |       |
+		 *   L V  NVA  V R
+		 * |---|-------|---|
+		 */
+		lva = __get_free_va_from_cache();
+		if (!lva) {
+			lva = kmalloc(sizeof(*lva), GFP_NOWAIT);
+			if (unlikely(!lva))
+				return vend;
+		}
+
+		/*
+		 * Build the remainder.
+		 */
+		lva->va_start = va->va_start;
+		lva->va_end = nva_start_addr;
+
+		/*
+		 * Shrink this VA to remaining size.
+		 */
+		va->va_start = nva_start_addr + size;
+
+		/*
+		 * Add the remainder to the address sorted free list/tree.
+		 */
+		__find_va_slot(lva, NULL, &va->rb_node, &parent, &link);
+		__link_va(lva, &free_vmap_area_root,
+			parent, link, &free_vmap_area_list);
+	} else {
+		/* Not found. */
+		nva_start_addr = vend;
+	}
+
+	return nva_start_addr;
+}
+
+static inline struct vmap_area *
+kmalloc_va_node_leak_scan(gfp_t mask, int node)
+{
+	struct vmap_area *va;
+
+	mask &= GFP_RECLAIM_MASK;
+
+	va = kmalloc_node(sizeof(*va), mask, node);
+	if (unlikely(!va))
+		return NULL;
+
+	/*
+	 * Only scan the relevant parts containing pointers
+	 * to other objects to avoid false negatives.
+	 */
+	kmemleak_scan_area(&va->rb_node, SIZE_MAX, mask);
+	return va;
+}
 
 /*
  * Allocate a region of KVA of the specified size and alignment, within the
@@ -404,11 +823,12 @@ static struct vmap_area *alloc_vmap_area(unsigned long size,
 				unsigned long vstart, unsigned long vend,
 				int node, gfp_t gfp_mask)
 {
-	struct vmap_area *va;
+	struct vmap_area *va = NULL;
 	struct rb_node *n;
 	unsigned long addr;
 	int purged = 0;
 	struct vmap_area *first;
+	bool is_vmalloc_alloc;
 
 	BUG_ON(!size);
 	BUG_ON(offset_in_page(size));
@@ -416,19 +836,38 @@ static struct vmap_area *alloc_vmap_area(unsigned long size,
 
 	might_sleep();
 
-	va = kmalloc_node(sizeof(struct vmap_area),
-			gfp_mask & GFP_RECLAIM_MASK, node);
-	if (unlikely(!va))
-		return ERR_PTR(-ENOMEM);
-
-	/*
-	 * Only scan the relevant parts containing pointers to other objects
-	 * to avoid false negatives.
-	 */
-	kmemleak_scan_area(&va->rb_node, SIZE_MAX, gfp_mask & GFP_RECLAIM_MASK);
+	is_vmalloc_alloc = is_vmalloc_addr((void *) vstart);
+	if (!is_vmalloc_alloc) {
+		va = kmalloc_va_node_leak_scan(gfp_mask, node);
+		if (unlikely(!va))
+			return ERR_PTR(-ENOMEM);
+	}
 
 retry:
 	spin_lock(&vmap_area_lock);
+	if (is_vmalloc_alloc) {
+		addr = alloc_vmalloc_area(&va, size, align, vstart, vend);
+
+		/*
+		 * If an allocation fails, the "vend" address is
+		 * returned. Therefore trigger an overflow path.
+		 */
+		if (unlikely(addr == vend))
+			goto overflow;
+
+		if (!va) {
+			spin_unlock(&vmap_area_lock);
+
+			va = kmalloc_va_node_leak_scan(gfp_mask, node);
+			if (unlikely(!va))
+				return ERR_PTR(-ENOMEM);
+
+			spin_lock(&vmap_area_lock);
+		}
+
+		goto insert_vmap_area;
+	}
+
 	/*
 	 * Invalidate cache if we have more permissive parameters.
 	 * cached_hole_size notes the largest hole noticed _below_
@@ -501,11 +940,15 @@ static struct vmap_area *alloc_vmap_area(unsigned long size,
 	if (addr + size > vend)
 		goto overflow;
 
+	free_vmap_cache = &va->rb_node;
+
+insert_vmap_area:
 	va->va_start = addr;
 	va->va_end = addr + size;
+	va->align = align;
 	va->flags = 0;
-	__insert_vmap_area(va);
-	free_vmap_cache = &va->rb_node;
+	__insert_vmap_area(va, &vmap_area_root, &vmap_area_list);
+
 	spin_unlock(&vmap_area_lock);
 
 	BUG_ON(!IS_ALIGNED(va->va_start, align));
@@ -552,9 +995,13 @@ EXPORT_SYMBOL_GPL(unregister_vmap_purge_notifier);
 
 static void __free_vmap_area(struct vmap_area *va)
 {
+	unsigned long last_free_va_start;
+	bool is_vmalloc_area;
+
 	BUG_ON(RB_EMPTY_NODE(&va->rb_node));
+	is_vmalloc_area = is_vmalloc_addr((void *) va->va_start);
 
-	if (free_vmap_cache) {
+	if (!is_vmalloc_area && free_vmap_cache) {
 		if (va->va_end < cached_vstart) {
 			free_vmap_cache = NULL;
 		} else {
@@ -571,18 +1018,38 @@ static void __free_vmap_area(struct vmap_area *va)
 	}
 	rb_erase(&va->rb_node, &vmap_area_root);
 	RB_CLEAR_NODE(&va->rb_node);
-	list_del_rcu(&va->list);
+	list_del(&va->list);
 
-	/*
-	 * Track the highest possible candidate for pcpu area
-	 * allocation.  Areas outside of vmalloc area can be returned
-	 * here too, consider only end addresses which fall inside
-	 * vmalloc area proper.
-	 */
-	if (va->va_end > VMALLOC_START && va->va_end <= VMALLOC_END)
+	if (is_vmalloc_area) {
+		/*
+		 * Track the highest possible candidate for pcpu area
+		 * allocation.  Areas outside of vmalloc area can be returned
+		 * here too, consider only end addresses which fall inside
+		 * vmalloc area proper.
+		 */
 		vmap_area_pcpu_hole = max(vmap_area_pcpu_hole, va->va_end);
 
-	kfree_rcu(va, rcu_head);
+		if (last_free_area)
+			last_free_va_start = last_free_area->va_start;
+		else
+			last_free_va_start = 0;
+
+		/*
+		 * Merge VA with its neighbors, otherwise just add it.
+		 */
+		va = __merge_add_free_va_area(va,
+			&free_vmap_area_root, &free_vmap_area_list);
+
+		/*
+		 * Update a search criteria if merging/inserting is done
+		 * before the va_start address of last_free_area marker.
+		 */
+		if (last_free_area)
+			if (va->va_start < last_free_va_start)
+				last_free_area = va;
+	} else {
+		kfree(va);
+	}
 }
 
 /*
@@ -1238,6 +1705,51 @@ void __init vm_area_register_early(struct vm_struct *vm, size_t align)
 	vm_area_add_early(vm);
 }
 
+static void vmalloc_init_free_space(void)
+{
+	unsigned long free_hole_start = VMALLOC_START;
+	const unsigned long vmalloc_end = VMALLOC_END;
+	struct vmap_area *busy_area, *free_area;
+
+	/*
+	 *     B     F     B     B     B     F
+	 * -|-----|.....|-----|-----|-----|.....|-
+	 *  |           vmalloc space           |
+	 *  |<--------------------------------->|
+	 */
+	list_for_each_entry(busy_area, &vmap_area_list, list) {
+		if (!is_vmalloc_addr((void *) busy_area->va_start))
+			continue;
+
+		if (busy_area->va_start - free_hole_start > 0) {
+			free_area = kzalloc(sizeof(*free_area), GFP_NOWAIT);
+			free_area->va_start = free_hole_start;
+			free_area->va_end = busy_area->va_start;
+
+			__insert_vmap_area(free_area,
+				&free_vmap_area_root, &free_vmap_area_list);
+		}
+
+		free_hole_start = busy_area->va_end;
+	}
+
+	if (vmalloc_end - free_hole_start > 0) {
+		free_area = kzalloc(sizeof(*free_area), GFP_NOWAIT);
+		free_area->va_start = free_hole_start;
+		free_area->va_end = vmalloc_end;
+
+		__insert_vmap_area(free_area,
+			&free_vmap_area_root, &free_vmap_area_list);
+	}
+
+	/*
+	 * Assume if busy VA overlaps two areas it is wrong.
+	 * I.e. a start address is vmalloc address whereas an
+	 * end address is not. Warn if so.
+	 */
+	WARN_ON(free_hole_start > vmalloc_end);
+}
+
 void __init vmalloc_init(void)
 {
 	struct vmap_area *va;
@@ -1263,9 +1775,14 @@ void __init vmalloc_init(void)
 		va->va_start = (unsigned long)tmp->addr;
 		va->va_end = va->va_start + tmp->size;
 		va->vm = tmp;
-		__insert_vmap_area(va);
+		__insert_vmap_area(va, &vmap_area_root, &vmap_area_list);
 	}
 
+	/*
+	 * Now we can initialize a free vmalloc space.
+	 */
+	vmalloc_init_free_space();
+
 	vmap_area_pcpu_hole = VMALLOC_END;
 
 	vmap_initialized = true;
@@ -2365,82 +2882,23 @@ static struct vmap_area *node_to_va(struct rb_node *n)
 	return rb_entry_safe(n, struct vmap_area, rb_node);
 }
 
-/**
- * pvm_find_next_prev - find the next and prev vmap_area surrounding @end
- * @end: target address
- * @pnext: out arg for the next vmap_area
- * @pprev: out arg for the previous vmap_area
- *
- * Returns: %true if either or both of next and prev are found,
- *	    %false if no vmap_area exists
- *
- * Find vmap_areas end addresses of which enclose @end.  ie. if not
- * NULL, *pnext->va_end > @end and *pprev->va_end <= @end.
- */
-static bool pvm_find_next_prev(unsigned long end,
-			       struct vmap_area **pnext,
-			       struct vmap_area **pprev)
+static struct vmap_area *
+addr_to_free_va(unsigned long addr)
 {
-	struct rb_node *n = vmap_area_root.rb_node;
+	struct rb_node *n = free_vmap_area_root.rb_node;
 	struct vmap_area *va = NULL;
 
 	while (n) {
 		va = rb_entry(n, struct vmap_area, rb_node);
-		if (end < va->va_end)
+		if (addr < va->va_start)
 			n = n->rb_left;
-		else if (end > va->va_end)
+		else if (addr > va->va_end)
 			n = n->rb_right;
 		else
-			break;
-	}
-
-	if (!va)
-		return false;
-
-	if (va->va_end > end) {
-		*pnext = va;
-		*pprev = node_to_va(rb_prev(&(*pnext)->rb_node));
-	} else {
-		*pprev = va;
-		*pnext = node_to_va(rb_next(&(*pprev)->rb_node));
-	}
-	return true;
-}
-
-/**
- * pvm_determine_end - find the highest aligned address between two vmap_areas
- * @pnext: in/out arg for the next vmap_area
- * @pprev: in/out arg for the previous vmap_area
- * @align: alignment
- *
- * Returns: determined end address
- *
- * Find the highest aligned address between *@pnext and *@pprev below
- * VMALLOC_END.  *@pnext and *@pprev are adjusted so that the aligned
- * down address is between the end addresses of the two vmap_areas.
- *
- * Please note that the address returned by this function may fall
- * inside *@pnext vmap_area.  The caller is responsible for checking
- * that.
- */
-static unsigned long pvm_determine_end(struct vmap_area **pnext,
-				       struct vmap_area **pprev,
-				       unsigned long align)
-{
-	const unsigned long vmalloc_end = VMALLOC_END & ~(align - 1);
-	unsigned long addr;
-
-	if (*pnext)
-		addr = min((*pnext)->va_start & ~(align - 1), vmalloc_end);
-	else
-		addr = vmalloc_end;
-
-	while (*pprev && (*pprev)->va_end > addr) {
-		*pnext = *pprev;
-		*pprev = node_to_va(rb_prev(&(*pnext)->rb_node));
+			return va;
 	}
 
-	return addr;
+	return NULL;
 }
 
 /**
@@ -2473,11 +2931,14 @@ struct vm_struct **pcpu_get_vm_areas(const unsigned long *offsets,
 {
 	const unsigned long vmalloc_start = ALIGN(VMALLOC_START, align);
 	const unsigned long vmalloc_end = VMALLOC_END & ~(align - 1);
-	struct vmap_area **vas, *prev, *next;
+	struct vmap_area **vas, *va;
+	struct vmap_area **off = NULL;
 	struct vm_struct **vms;
-	int area, area2, last_area, term_area;
-	unsigned long base, start, end, last_end;
+	int area, area2, last_area;
+	unsigned long start, end, last_end;
+	unsigned long base;
 	bool purged = false;
+	u8 fit_type = NOTHING_FIT;
 
 	/* verify parameters and allocate data structures */
 	BUG_ON(offset_in_page(align) || !is_power_of_2(align));
@@ -2512,90 +2973,122 @@ struct vm_struct **pcpu_get_vm_areas(const unsigned long *offsets,
 	if (!vas || !vms)
 		goto err_free2;
 
+	if (nr_vms > 1) {
+		off = kcalloc(nr_vms, sizeof(off[0]), GFP_KERNEL);
+		if (!off)
+			goto err_free2;
+	}
+
 	for (area = 0; area < nr_vms; area++) {
 		vas[area] = kzalloc(sizeof(struct vmap_area), GFP_KERNEL);
 		vms[area] = kzalloc(sizeof(struct vm_struct), GFP_KERNEL);
 		if (!vas[area] || !vms[area])
 			goto err_free;
+
+		if (nr_vms > 1) {
+			off[area] = kzalloc(sizeof(off[0]), GFP_KERNEL);
+			if (!off[area])
+				goto err_free;
+		}
 	}
+
 retry:
 	spin_lock(&vmap_area_lock);
 
-	/* start scanning - we scan from the top, begin with the last area */
-	area = term_area = last_area;
-	start = offsets[area];
-	end = start + sizes[area];
+	/*
+	 * Initialize va here, since we can retry the search.
+	 */
+	va = NULL;
 
-	if (!pvm_find_next_prev(vmap_area_pcpu_hole, &next, &prev)) {
-		base = vmalloc_end - last_end;
-		goto found;
+	if (unlikely(list_empty(&free_vmap_area_list))) {
+		spin_unlock(&vmap_area_lock);
+		goto err_free;
 	}
-	base = pvm_determine_end(&next, &prev, align) - end;
 
-	while (true) {
-		BUG_ON(next && next->va_end <= base + end);
-		BUG_ON(prev && prev->va_end > base + end);
+	if (off)
+		va = addr_to_free_va(vmap_area_pcpu_hole);
+
+	if (!va)
+		va = list_last_entry(&free_vmap_area_list,
+				struct vmap_area, list);
+
+	list_for_each_entry_from_reverse(va, &free_vmap_area_list, list) {
+		base = (va->va_end & ~(align - 1)) - last_end;
 
 		/*
 		 * base might have underflowed, add last_end before
 		 * comparing.
 		 */
-		if (base + last_end < vmalloc_start + last_end) {
-			spin_unlock(&vmap_area_lock);
-			if (!purged) {
-				purge_vmap_area_lazy();
-				purged = true;
-				goto retry;
-			}
-			goto err_free;
-		}
+		if (base + last_end < vmalloc_start + last_end)
+			break;
 
-		/*
-		 * If next overlaps, move base downwards so that it's
-		 * right below next and then recheck.
-		 */
-		if (next && next->va_start < base + end) {
-			base = pvm_determine_end(&next, &prev, align) - end;
-			term_area = area;
+		if (base < va->va_start)
 			continue;
-		}
 
+		if (base > va->va_start)
+			fit_type = RE_FIT_TYPE;
+		else
+			/* base == va->va_start */
+			fit_type = FL_FIT_TYPE;
+
+		break;
+	}
+
+	if (fit_type == RE_FIT_TYPE) {
+		va->va_end = base;
+	} else if (fit_type == FL_FIT_TYPE) {
 		/*
-		 * If prev overlaps, shift down next and prev and move
-		 * base so that it's right below new next and then
-		 * recheck.
+		 * Check if there is an interaction with regular
+		 * vmalloc allocations when a free block fully fits.
+		 * If so just shift back last_free_area marker.
 		 */
-		if (prev && prev->va_end > base + start)  {
-			next = prev;
-			prev = node_to_va(rb_prev(&next->rb_node));
-			base = pvm_determine_end(&next, &prev, align) - end;
-			term_area = area;
-			continue;
+		if (last_free_area == va)
+			last_free_area = node_to_va(rb_prev(&va->rb_node));
+
+		__remove_vmap_area(va, &free_vmap_area_root);
+	} else {
+		spin_unlock(&vmap_area_lock);
+		if (!purged) {
+			purge_vmap_area_lazy();
+			purged = true;
+			goto retry;
 		}
 
-		/*
-		 * This area fits, move on to the previous one.  If
-		 * the previous one is the terminal one, we're done.
-		 */
-		area = (area + nr_vms - 1) % nr_vms;
-		if (area == term_area)
-			break;
-		start = offsets[area];
-		end = start + sizes[area];
-		pvm_find_next_prev(base + end, &next, &prev);
+		goto err_free;
 	}
-found:
-	/* we've found a fitting base, insert all va's */
-	for (area = 0; area < nr_vms; area++) {
-		struct vmap_area *va = vas[area];
 
+	/* we've found a fitting base, insert all va's */
+	for (area = 0, start = base; area < nr_vms; area++) {
+		va = vas[area];
 		va->va_start = base + offsets[area];
 		va->va_end = va->va_start + sizes[area];
-		__insert_vmap_area(va);
-	}
 
-	vmap_area_pcpu_hole = base + offsets[last_area];
+		/*
+		 * If there are several areas to allocate, we should insert
+		 * back a free space that is organized by area size and offset.
+		 */
+		if (off) {
+			off[area]->va_start = start;
+			off[area]->va_end = start + offsets[area];
 
+			/* Shift to next free start. */
+			start = va->va_end;
+
+			/*
+			 * Some initialization before adding/merging.
+			 */
+			RB_CLEAR_NODE(&off[area]->rb_node);
+			off[area]->align = 1;
+
+			(void) __merge_add_free_va_area(off[area],
+				&free_vmap_area_root, &free_vmap_area_list);
+		}
+
+		__insert_vmap_area(va, &vmap_area_root, &vmap_area_list);
+		va->align = 1;
+	}
+
+	vmap_area_pcpu_hole = base;
 	spin_unlock(&vmap_area_lock);
 
 	/* insert all vm's */
@@ -2604,16 +3097,21 @@ struct vm_struct **pcpu_get_vm_areas(const unsigned long *offsets,
 				 pcpu_get_vm_areas);
 
 	kfree(vas);
+	kfree(off);
 	return vms;
 
 err_free:
 	for (area = 0; area < nr_vms; area++) {
+		if (nr_vms > 1)
+			kfree(off[area]);
+
 		kfree(vas[area]);
 		kfree(vms[area]);
 	}
 err_free2:
 	kfree(vas);
 	kfree(vms);
+	kfree(off);
 	return NULL;
 }
 
-- 
2.11.0
```

## [RFC PATCH 2/2] mm: add priority threshold to __purge_vmap_area_lazy()

```patch
commit 763b218ddfaf ("mm: add preempt points into
__purge_vmap_area_lazy()")

introduced some preempt points, one of those is making
an allocation more prioritized.

Prioritizing an allocation over freeing does not work
well all the time, i.e. it should be rather a compromise.

1) Number of lazy pages directly influence on busy list
length thus on operations like: allocation, lookup, unmap,
remove, etc.

2) Under heavy simultaneous allocations/releases there may
be a situation when memory usage grows too fast hitting
out_of_memory -> panic.

Establish a threshold passing which the freeing path is
prioritized over allocation creating a balance between both.

Signed-off-by: Uladzislau Rezki (Sony) <urezki@gmail.com>
---
 mm/vmalloc.c | 14 ++++++++------
 1 file changed, 8 insertions(+), 6 deletions(-)

diff --git a/mm/vmalloc.c b/mm/vmalloc.c
index a7f257540a05..bbafcff6632b 100644
--- a/mm/vmalloc.c
+++ b/mm/vmalloc.c
@@ -1124,23 +1124,23 @@ static bool __purge_vmap_area_lazy(unsigned long start, unsigned long end)
 	struct llist_node *valist;
 	struct vmap_area *va;
 	struct vmap_area *n_va;
-	bool do_free = false;
+	int resched_threshold;
 
 	lockdep_assert_held(&vmap_purge_lock);
 
 	valist = llist_del_all(&vmap_purge_list);
+	if (unlikely(valist == NULL))
+		return false;
+
 	llist_for_each_entry(va, valist, purge_list) {
 		if (va->va_start < start)
 			start = va->va_start;
 		if (va->va_end > end)
 			end = va->va_end;
-		do_free = true;
 	}
 
-	if (!do_free)
-		return false;
-
 	flush_tlb_kernel_range(start, end);
+	resched_threshold = (int) lazy_max_pages() << 1;
 
 	spin_lock(&vmap_area_lock);
 	llist_for_each_entry_safe(va, n_va, valist, purge_list) {
@@ -1148,7 +1148,9 @@ static bool __purge_vmap_area_lazy(unsigned long start, unsigned long end)
 
 		__free_vmap_area(va);
 		atomic_sub(nr, &vmap_lazy_nr);
-		cond_resched_lock(&vmap_area_lock);
+
+		if (atomic_read(&vmap_lazy_nr) < resched_threshold)
+			cond_resched_lock(&vmap_area_lock);
 	}
 	spin_unlock(&vmap_area_lock);
 	return true;
-- 
2.11.0
```