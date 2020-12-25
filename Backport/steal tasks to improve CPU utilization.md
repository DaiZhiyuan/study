<!-- TOC -->

- [Performance: steal task to improve CPU utilization](#performance-steal-task-to-improve-cpu-utilization)
    - [[PATCH v4 00/10] steal tasks to improve CPU utilization:](#patch-v4-0010-steal-tasks-to-improve-cpu-utilization)
    - [[PATCH v4 01/10] sched: Provide sparsemask, a reduced contention bitmap](#patch-v4-0110-sched-provide-sparsemask-a-reduced-contention-bitmap)
    - [[PATCH v4 02/10] sched/topology: Provide hooks to allocate data shared per LLC](#patch-v4-0210-schedtopology-provide-hooks-to-allocate-data-shared-per-llc)
    - [[PATCH v4 03/10] sched/topology: Provide cfs_overload_cpus bitmap](#patch-v4-0310-schedtopology-provide-cfs_overload_cpus-bitmap)
    - [[PATCH v4 04/10] sched/fair: Dynamically update cfs_overload_cpus](#patch-v4-0410-schedfair-dynamically-update-cfs_overload_cpus)
    - [[PATCH v4 05/10] sched/fair: Hoist idle_stamp up from idle_balance](#patch-v4-0510-schedfair-hoist-idle_stamp-up-from-idle_balance)
    - [[PATCH v4 06/10] sched/fair: Generalize the detach_task interface](#patch-v4-0610-schedfair-generalize-the-detach_task-interface)
    - [[PATCH v4 07/10] sched/fair: Provide can_migrate_task_llc](#patch-v4-0710-schedfair-provide-can_migrate_task_llc)
    - [[PATCH v4 08/10] sched/fair: Steal work from an overloaded CPU when CPU goes idle](#patch-v4-0810-schedfair-steal-work-from-an-overloaded-cpu-when-cpu-goes-idle)
    - [[PATCH v4 09/10] sched/fair: disable stealing if too many NUMA nodes](#patch-v4-0910-schedfair-disable-stealing-if-too-many-numa-nodes)
    - [[PATCH v4 10/10] sched/fair: Provide idle search schedstats](#patch-v4-1010-schedfair-provide-idle-search-schedstats)

<!-- /TOC -->

# Performance: steal task to improve CPU utilization

## [PATCH v4 00/10] steal tasks to improve CPU utilization:

```patch
When a CPU has no more CFS tasks to run, and idle_balance() fails to
find a task, then attempt to steal a task from an overloaded CPU in the
same LLC. Maintain and use a bitmap of overloaded CPUs to efficiently
identify candidates.  To minimize search time, steal the first migratable
task that is found when the bitmap is traversed.  For fairness, search
for migratable tasks on an overloaded CPU in order of next to run.

This simple stealing yields a higher CPU utilization than idle_balance()
alone, because the search is cheap, so it may be called every time the CPU
is about to go idle.  idle_balance() does more work because it searches
widely for the busiest queue, so to limit its CPU consumption, it declines
to search if the system is too busy.  Simple stealing does not offload the
globally busiest queue, but it is much better than running nothing at all.

The bitmap of overloaded CPUs is a new type of sparse bitmap, designed to
reduce cache contention vs the usual bitmap when many threads concurrently
set, clear, and visit elements.

Patch 1 defines the sparsemask type and its operations.

Patches 2, 3, and 4 implement the bitmap of overloaded CPUs.

Patches 5 and 6 refactor existing code for a cleaner merge of later
  patches.

Patches 7 and 8 implement task stealing using the overloaded CPUs bitmap.

Patch 9 disables stealing on systems with more than 2 NUMA nodes for the
time being because of performance regressions that are not due to stealing
per-se.  See the patch description for details.

Patch 10 adds schedstats for comparing the new behavior to the old, and
  provided as a convenience for developers only, not for integration.

The patch series is based on kernel 4.20.0-rc1.  It compiles, boots, and
runs with/without each of CONFIG_SCHED_SMT, CONFIG_SMP, CONFIG_SCHED_DEBUG,
and CONFIG_PREEMPT.  It runs without error with CONFIG_DEBUG_PREEMPT +
CONFIG_SLUB_DEBUG + CONFIG_DEBUG_PAGEALLOC + CONFIG_DEBUG_MUTEXES +
CONFIG_DEBUG_SPINLOCK + CONFIG_DEBUG_ATOMIC_SLEEP.  CPU hot plug and CPU
bandwidth control were tested.

Stealing improves utilization with only a modest CPU overhead in scheduler
code.  In the following experiment, hackbench is run with varying numbers
of groups (40 tasks per group), and the delta in /proc/schedstat is shown
for each run, averaged per CPU, augmented with these non-standard stats:

  %find - percent of time spent in old and new functions that search for
    idle CPUs and tasks to steal and set the overloaded CPUs bitmap.

  steal - number of times a task is stolen from another CPU.

X6-2: 1 socket * 10 cores * 2 hyperthreads = 20 CPUs
Intel(R) Xeon(R) CPU E5-2630 v4 @ 2.20GHz
hackbench <grps> process 100000
sched_wakeup_granularity_ns=15000000

  baseline
  grps  time  %busy  slice   sched   idle     wake %find  steal
  1    8.084  75.02   0.10  105476  46291    59183  0.31      0
  2   13.892  85.33   0.10  190225  70958   119264  0.45      0
  3   19.668  89.04   0.10  263896  87047   176850  0.49      0
  4   25.279  91.28   0.10  322171  94691   227474  0.51      0
  8   47.832  94.86   0.09  630636 144141   486322  0.56      0

  new
  grps  time  %busy  slice   sched   idle     wake %find  steal  %speedup
  1    5.938  96.80   0.24   31255   7190    24061  0.63   7433  36.1
  2   11.491  99.23   0.16   74097   4578    69512  0.84  19463  20.9
  3   16.987  99.66   0.15  115824   1985   113826  0.77  24707  15.8
  4   22.504  99.80   0.14  167188   2385   164786  0.75  29353  12.3
  8   44.441  99.86   0.11  389153   1616   387401  0.67  38190   7.6

Elapsed time improves by 8 to 36%, and CPU busy utilization is up
by 5 to 22% hitting 99% for 2 or more groups (80 or more tasks).
The cost is at most 0.4% more find time.

Additional performance results follow.  A negative "speedup" is a
regression.  Note: for all hackbench runs, sched_wakeup_granularity_ns
is set to 15 msec.  Otherwise, preemptions increase at higher loads and
distort the comparison between baseline and new.

------------------ 1 Socket Results ------------------

X6-2: 1 socket * 10 cores * 2 hyperthreads = 20 CPUs
Intel(R) Xeon(R) CPU E5-2630 v4 @ 2.20GHz
Average of 10 runs of: hackbench <groups> process 100000

            --- base --    --- new ---
  groups    time %stdev    time %stdev  %speedup
       1   8.008    0.1   5.905    0.2      35.6
       2  13.814    0.2  11.438    0.1      20.7
       3  19.488    0.2  16.919    0.1      15.1
       4  25.059    0.1  22.409    0.1      11.8
       8  47.478    0.1  44.221    0.1       7.3

X6-2: 1 socket * 22 cores * 2 hyperthreads = 44 CPUs
Intel(R) Xeon(R) CPU E5-2699 v4 @ 2.20GHz
Average of 10 runs of: hackbench <groups> process 100000

            --- base --    --- new ---
  groups    time %stdev    time %stdev  %speedup
       1   4.586    0.8   4.596    0.6      -0.3
       2   7.693    0.2   5.775    1.3      33.2
       3  10.442    0.3   8.288    0.3      25.9
       4  13.087    0.2  11.057    0.1      18.3
       8  24.145    0.2  22.076    0.3       9.3
      16  43.779    0.1  41.741    0.2       4.8

KVM 4-cpu
Intel(R) Xeon(R) CPU E5-2699 v3 @ 2.30GHz
tbench, average of 11 runs.

  clients    %speedup
        1        16.2
        2        11.7
        4         9.9
        8        12.8
       16        13.7

KVM 2-cpu
Intel(R) Xeon(R) CPU E5-2699 v3 @ 2.30GHz

  Benchmark                     %speedup
  specjbb2015_critical_jops          5.7
  mysql_sysb1.0.14_mutex_2          40.6
  mysql_sysb1.0.14_oltp_2            3.9

------------------ 2 Socket Results ------------------

X6-2: 2 sockets * 10 cores * 2 hyperthreads = 40 CPUs
Intel(R) Xeon(R) CPU E5-2630 v4 @ 2.20GHz
Average of 10 runs of: hackbench <groups> process 100000

            --- base --    --- new ---
  groups    time %stdev    time %stdev  %speedup
       1   7.945    0.2   7.219    8.7      10.0
       2   8.444    0.4   6.689    1.5      26.2
       3  12.100    1.1   9.962    2.0      21.4
       4  15.001    0.4  13.109    1.1      14.4
       8  27.960    0.2  26.127    0.3       7.0

X6-2: 2 sockets * 22 cores * 2 hyperthreads = 88 CPUs
Intel(R) Xeon(R) CPU E5-2699 v4 @ 2.20GHz
Average of 10 runs of: hackbench <groups> process 100000

            --- base --    --- new ---
  groups    time %stdev    time %stdev  %speedup
       1   5.826    5.4   5.840    5.0      -0.3
       2   5.041    5.3   6.171   23.4     -18.4
       3   6.839    2.1   6.324    3.8       8.1
       4   8.177    0.6   7.318    3.6      11.7
       8  14.429    0.7  13.966    1.3       3.3
      16  26.401    0.3  25.149    1.5       4.9


X6-2: 2 sockets * 22 cores * 2 hyperthreads = 88 CPUs
Intel(R) Xeon(R) CPU E5-2699 v4 @ 2.20GHz
Oracle database OLTP, logging disabled, NVRAM storage

  Customers   Users   %speedup
    1200000      40       -1.2
    2400000      80        2.7
    3600000     120        8.9
    4800000     160        4.4
    6000000     200        3.0

X6-2: 2 sockets * 14 cores * 2 hyperthreads = 56 CPUs
Intel(R) Xeon(R) CPU E5-2690 v4 @ 2.60GHz
Results from the Oracle "Performance PIT".

  Benchmark                                           %speedup

  mysql_sysb1.0.14_fileio_56_rndrd                        19.6
  mysql_sysb1.0.14_fileio_56_seqrd                        12.1
  mysql_sysb1.0.14_fileio_56_rndwr                         0.4
  mysql_sysb1.0.14_fileio_56_seqrewr                      -0.3

  pgsql_sysb1.0.14_fileio_56_rndrd                        19.5
  pgsql_sysb1.0.14_fileio_56_seqrd                         8.6
  pgsql_sysb1.0.14_fileio_56_rndwr                         1.0
  pgsql_sysb1.0.14_fileio_56_seqrewr                       0.5

  opatch_time_ASM_12.2.0.1.0_HP2M                          7.5
  select-1_users-warm_asmm_ASM_12.2.0.1.0_HP2M             5.1
  select-1_users_asmm_ASM_12.2.0.1.0_HP2M                  4.4
  swingbenchv3_asmm_soebench_ASM_12.2.0.1.0_HP2M           5.8

  lm3_memlat_L2                                            4.8
  lm3_memlat_L1                                            0.0

  ub_gcc_56CPUs-56copies_Pipe-based_Context_Switching     60.1
  ub_gcc_56CPUs-56copies_Shell_Scripts_1_concurrent        5.2
  ub_gcc_56CPUs-56copies_Shell_Scripts_8_concurrent       -3.0
  ub_gcc_56CPUs-56copies_File_Copy_1024_bufsize_2000_maxblocks 2.4

X5-2: 2 sockets * 18 cores * 2 hyperthreads = 72 CPUs
Intel(R) Xeon(R) CPU E5-2699 v3 @ 2.30GHz

  NAS_OMP
  bench class   ncpu    %improved(Mops)
  dc    B       72      1.3
  is    C       72      0.9
  is    D       72      0.7

  sysbench mysql, average of 24 runs
          --- base ---     --- new ---
  nthr   events  %stdev   events  %stdev %speedup
     1    331.0    0.25    331.0    0.24     -0.1
     2    661.3    0.22    661.8    0.22      0.0
     4   1297.0    0.88   1300.5    0.82      0.2
     8   2420.8    0.04   2420.5    0.04     -0.1
    16   4826.3    0.07   4825.4    0.05     -0.1
    32   8815.3    0.27   8830.2    0.18      0.1
    64  12823.0    0.24  12823.6    0.26      0.0

--------------------------------------------------------------

Changes from v1 to v2:
  - Remove stray find_time hunk from patch 5
  - Fix "warning: label out defined but not used" for !CONFIG_SCHED_SMT
  - Set SCHED_STEAL_NODE_LIMIT_DEFAULT to 2
  - Steal iff avg_idle exceeds the cost of stealing

Changes from v2 to v3:
  - Update series for kernel 4.20.  Context changes only.

Changes from v3 to v4:
  - Avoid 64-bit division on 32-bit processors in compute_skid()
  - Replace IF_SMP with inline functions to set idle_stamp
  - Push ZALLOC_MASK body into calling function
  - Set rq->cfs_overload_cpus in update_top_cache_domain instead of
    cpu_attach_domain
  - Rewrite sparsemask iterator for complete inlining
  - Cull and clean up sparsemask functions and moved all into
    sched/sparsemask.h

Steve Sistare (10):
  sched: Provide sparsemask, a reduced contention bitmap
  sched/topology: Provide hooks to allocate data shared per LLC
  sched/topology: Provide cfs_overload_cpus bitmap
  sched/fair: Dynamically update cfs_overload_cpus
  sched/fair: Hoist idle_stamp up from idle_balance
  sched/fair: Generalize the detach_task interface
  sched/fair: Provide can_migrate_task_llc
  sched/fair: Steal work from an overloaded CPU when CPU goes idle
  sched/fair: disable stealing if too many NUMA nodes
  sched/fair: Provide idle search schedstats

 include/linux/sched/topology.h |   1 +
 kernel/sched/core.c            |  31 +++-
 kernel/sched/fair.c            | 354 +++++++++++++++++++++++++++++++++++++----
 kernel/sched/features.h        |   6 +
 kernel/sched/sched.h           |  13 +-
 kernel/sched/sparsemask.h      | 210 ++++++++++++++++++++++++
 kernel/sched/stats.c           |  11 +-
 kernel/sched/stats.h           |  13 ++
 kernel/sched/topology.c        | 121 +++++++++++++-
 9 files changed, 726 insertions(+), 34 deletions(-)
 create mode 100644 kernel/sched/sparsemask.h

-- 
1.8.3.1
```

## [PATCH v4 01/10] sched: Provide sparsemask, a reduced contention bitmap
```patch
Provide struct sparsemask and functions to manipulate it.  A sparsemask is
a sparse bitmap.  It reduces cache contention vs the usual bitmap when many
threads concurrently set, clear, and visit elements, by reducing the number
of significant bits per cacheline.  For each cacheline chunk of the mask,
only the first K bits of the first word are used, and the remaining bits
are ignored, where K is a creation time parameter.  Thus a sparsemask that
can represent a set of N elements is approximately (N/K * CACHELINE) bytes
in size.

This type is simpler and more efficient than the struct sbitmap used by
block drivers.

Signed-off-by: Steve Sistare <steven.sistare@oracle.com>
---
 kernel/sched/sparsemask.h | 210 ++++++++++++++++++++++++++++++++++++++++++++++
 1 file changed, 210 insertions(+)
 create mode 100644 kernel/sched/sparsemask.h

diff --git a/kernel/sched/sparsemask.h b/kernel/sched/sparsemask.h
new file mode 100644
index 0000000..1194862
--- /dev/null
+++ b/kernel/sched/sparsemask.h
@@ -0,0 +1,210 @@
+/* SPDX-License-Identifier: GPL-2.0 */
+/*
+ * sparsemask.h - sparse bitmap operations
+ *
+ * Copyright (c) 2018 Oracle Corporation
+ *
+ * This program is free software; you can redistribute it and/or modify
+ * it under the terms of the GNU General Public License as published by
+ * the Free Software Foundation; either version 2 of the License, or
+ * (at your option) any later version.
+ *
+ * This program is distributed in the hope that it will be useful,
+ * but WITHOUT ANY WARRANTY; without even the implied warranty of
+ * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
+ * GNU General Public License for more details.
+ */
+
+#ifndef __LINUX_SPARSEMASK_H
+#define __LINUX_SPARSEMASK_H
+
+#include <linux/kernel.h>
+#include <linux/bitmap.h>
+#include <linux/bug.h>
+
+/*
+ * A sparsemask is a sparse bitmap.  It reduces cache contention vs the usual
+ * bitmap when many threads concurrently set, clear, and visit elements.  For
+ * each cacheline chunk of the mask, only the first K bits of the first word are
+ * used, and the remaining bits are ignored, where K is a creation time
+ * parameter.  Thus a sparsemask that can represent a set of N elements is
+ * approximately (N/K * CACHELINE) bytes in size.
+ *
+ * Clients pass and receive element numbers in the public API, and the
+ * implementation translates them to bit numbers to perform the bitmap
+ * operations.
+ */
+
+struct sparsemask_chunk {
+	unsigned long word;	/* the significant bits */
+} ____cacheline_aligned_in_smp;
+
+struct sparsemask {
+	short nelems;		/* current number of elements */
+	short density;		/* store 2^density elements per chunk */
+	struct sparsemask_chunk chunks[0];  /* embedded array of chunks */
+};
+
+#define _SMASK_INDEX(density, elem)	((elem) >> (density))
+#define _SMASK_BIT(density, elem)	((elem) & ((1U << (density)) - 1U))
+#define SMASK_INDEX(mask, elem)		_SMASK_INDEX((mask)->density, elem)
+#define SMASK_BIT(mask, elem)		_SMASK_BIT((mask)->density, elem)
+#define SMASK_WORD(mask, elem)		\
+	(&(mask)->chunks[SMASK_INDEX((mask), (elem))].word)
+
+/*
+ * sparsemask_next() - Return the next one bit in a bitmap, starting at a
+ * specified position and wrapping from the last bit to the first, up to but
+ * not including a specified origin.  This is a helper, so do not call it
+ * directly.
+ *
+ * @mask: Bitmap to search.
+ * @origin: Origin.
+ * @prev: Previous bit. Start search after this bit number.
+ *	  If -1, start search at @origin.
+ *
+ * Return: the bit number, else mask->nelems if no bits are set in the range.
+ */
+static inline int
+sparsemask_next(const struct sparsemask *mask, int origin, int prev)
+{
+	int density = mask->density;
+	int bits_per_word = 1U << density;
+	const struct sparsemask_chunk *chunk;
+	int nelems = mask->nelems;
+	int next, bit, nbits;
+	unsigned long word;
+
+	/* Calculate number of bits to be searched. */
+	if (prev == -1) {
+		nbits = nelems;
+		next = origin;
+	} else if (prev < origin) {
+		nbits = origin - prev;
+		next = prev + 1;
+	} else {
+		nbits = nelems - prev + origin - 1;
+		next = prev + 1;
+	}
+
+	if (unlikely(next >= nelems))
+		return nelems;
+
+	/*
+	 * Fetch and adjust first word.  Clear word bits below @next, and round
+	 * @next down to @bits_per_word boundary because later ffs will add
+	 * those bits back.
+	 */
+	chunk = &mask->chunks[_SMASK_INDEX(density, next)];
+	bit = _SMASK_BIT(density, next);
+	word = chunk->word & (~0UL << bit);
+	next -= bit;
+	nbits += bit;
+
+	while (!word) {
+		next += bits_per_word;
+		nbits -= bits_per_word;
+		if (nbits <= 0)
+			return nelems;
+
+		if (next >= nelems) {
+			chunk = mask->chunks;
+			nbits -= (next - nelems);
+			next = 0;
+		} else {
+			chunk++;
+		}
+		word = chunk->word;
+	}
+
+	next += __ffs(word);
+	if (next >= origin && prev != -1)
+		return nelems;
+	return next;
+}
+
+/****************** The public API ********************/
+
+/*
+ * Max value for the density parameter, limited by 64 bits in the chunk word.
+ */
+#define SMASK_DENSITY_MAX		6
+
+/*
+ * Return bytes to allocate for a sparsemask, for custom allocators.
+ */
+static inline size_t sparsemask_size(int nelems, int density)
+{
+	int index = _SMASK_INDEX(density, nelems) + 1;
+
+	return offsetof(struct sparsemask, chunks[index]);
+}
+
+/*
+ * Initialize an allocated sparsemask, for custom allocators.
+ */
+static inline void
+sparsemask_init(struct sparsemask *mask, int nelems, int density)
+{
+	WARN_ON(density < 0 || density > SMASK_DENSITY_MAX || nelems < 0);
+	mask->nelems = nelems;
+	mask->density = density;
+}
+
+/*
+ * sparsemask_alloc_node() - Allocate, initialize, and return a sparsemask.
+ *
+ * @nelems - maximum number of elements.
+ * @density - store 2^density elements per cacheline chunk.
+ *	      values from 0 to SMASK_DENSITY_MAX inclusive.
+ * @flags - kmalloc allocation flags
+ * @node - numa node
+ */
+static inline struct sparsemask *
+sparsemask_alloc_node(int nelems, int density, gfp_t flags, int node)
+{
+	int nbytes = sparsemask_size(nelems, density);
+	struct sparsemask *mask = kmalloc_node(nbytes, flags, node);
+
+	if (mask)
+		sparsemask_init(mask, nelems, density);
+	return mask;
+}
+
+static inline void sparsemask_free(struct sparsemask *mask)
+{
+	kfree(mask);
+}
+
+static inline void sparsemask_set_elem(struct sparsemask *dst, int elem)
+{
+	set_bit(SMASK_BIT(dst, elem), SMASK_WORD(dst, elem));
+}
+
+static inline void sparsemask_clear_elem(struct sparsemask *dst, int elem)
+{
+	clear_bit(SMASK_BIT(dst, elem), SMASK_WORD(dst, elem));
+}
+
+static inline int sparsemask_test_elem(const struct sparsemask *mask, int elem)
+{
+	return test_bit(SMASK_BIT(mask, elem), SMASK_WORD(mask, elem));
+}
+
+/*
+ * sparsemask_for_each() - iterate over each set bit in a bitmap, starting at a
+ *   specified position, and wrapping from the last bit to the first.
+ *
+ * @mask: Bitmap to iterate over.
+ * @origin: Bit number at which to start searching.
+ * @elem: Iterator.  Can be signed or unsigned integer.
+ *
+ * The implementation does not assume any bit in @mask is set, including
+ * @origin.  After the loop, @elem = @mask->nelems.
+ */
+#define sparsemask_for_each(mask, origin, elem)				\
+	for ((elem) = -1;						\
+	     (elem) = sparsemask_next((mask), (origin), (elem)),	\
+		(elem) < (mask)->nelems;)
+
+#endif /* __LINUX_SPARSEMASK_H */
-- 
1.8.3.1
```

## [PATCH v4 02/10] sched/topology: Provide hooks to allocate data shared per LLC

```patch
Add functions sd_llc_alloc_all() and sd_llc_free_all() to allocate and
free data pointed to by struct sched_domain_shared at the last-level-cache
domain.  sd_llc_alloc_all() is called after the SD hierarchy is known, to
eliminate the unnecessary allocations that would occur if we instead
allocated in __sdt_alloc() and then figured out which shared nodes are
redundant.

Signed-off-by: Steve Sistare <steven.sistare@oracle.com>
---
 kernel/sched/topology.c | 75 ++++++++++++++++++++++++++++++++++++++++++++++++-
 1 file changed, 74 insertions(+), 1 deletion(-)

diff --git a/kernel/sched/topology.c b/kernel/sched/topology.c
index 8d7f15b..3e72ce0 100644
--- a/kernel/sched/topology.c
+++ b/kernel/sched/topology.c
@@ -10,6 +10,12 @@
 static cpumask_var_t sched_domains_tmpmask;
 static cpumask_var_t sched_domains_tmpmask2;
 
+struct s_data;
+static int sd_llc_alloc(struct sched_domain *sd);
+static void sd_llc_free(struct sched_domain *sd);
+static int sd_llc_alloc_all(const struct cpumask *cpu_map, struct s_data *d);
+static void sd_llc_free_all(const struct cpumask *cpu_map);
+
 #ifdef CONFIG_SCHED_DEBUG
 
 static int __init sched_debug_setup(char *str)
@@ -361,8 +367,10 @@ static void destroy_sched_domain(struct sched_domain *sd)
 	 */
 	free_sched_groups(sd->groups, 1);
 
-	if (sd->shared && atomic_dec_and_test(&sd->shared->ref))
+	if (sd->shared && atomic_dec_and_test(&sd->shared->ref)) {
+		sd_llc_free(sd);
 		kfree(sd->shared);
+	}
 	kfree(sd);
 }
 
@@ -996,6 +1004,7 @@ static void __free_domain_allocs(struct s_data *d, enum s_alloc what,
 		free_percpu(d->sd);
 		/* Fall through */
 	case sa_sd_storage:
+		sd_llc_free_all(cpu_map);
 		__sdt_free(cpu_map);
 		/* Fall through */
 	case sa_none:
@@ -1610,6 +1619,62 @@ static void __sdt_free(const struct cpumask *cpu_map)
 	}
 }
 
+static int sd_llc_alloc(struct sched_domain *sd)
+{
+	/* Allocate sd->shared data here. Empty for now. */
+
+	return 0;
+}
+
+static void sd_llc_free(struct sched_domain *sd)
+{
+	struct sched_domain_shared *sds = sd->shared;
+
+	if (!sds)
+		return;
+
+	/* Free data here. Empty for now. */
+}
+
+static int sd_llc_alloc_all(const struct cpumask *cpu_map, struct s_data *d)
+{
+	struct sched_domain *sd, *hsd;
+	int i;
+
+	for_each_cpu(i, cpu_map) {
+		/* Find highest domain that shares resources */
+		hsd = NULL;
+		for (sd = *per_cpu_ptr(d->sd, i); sd; sd = sd->parent) {
+			if (!(sd->flags & SD_SHARE_PKG_RESOURCES))
+				break;
+			hsd = sd;
+		}
+		if (hsd && sd_llc_alloc(hsd))
+			return 1;
+	}
+
+	return 0;
+}
+
+static void sd_llc_free_all(const struct cpumask *cpu_map)
+{
+	struct sched_domain_topology_level *tl;
+	struct sched_domain *sd;
+	struct sd_data *sdd;
+	int j;
+
+	for_each_sd_topology(tl) {
+		sdd = &tl->data;
+		if (!sdd)
+			continue;
+		for_each_cpu(j, cpu_map) {
+			sd = *per_cpu_ptr(sdd->sd, j);
+			if (sd)
+				sd_llc_free(sd);
+		}
+	}
+}
+
 static struct sched_domain *build_sched_domain(struct sched_domain_topology_level *tl,
 		const struct cpumask *cpu_map, struct sched_domain_attr *attr,
 		struct sched_domain *child, int dflags, int cpu)
@@ -1769,6 +1834,14 @@ static struct sched_domain *build_sched_domain(struct sched_domain_topology_leve
 		}
 	}
 
+	/*
+	 * Allocate shared sd data at last level cache.  Must be done after
+	 * domains are built above, but before the data is used in
+	 * cpu_attach_domain and descendants below.
+	 */
+	if (sd_llc_alloc_all(cpu_map, &d))
+		goto error;
+
 	/* Attach the domains */
 	rcu_read_lock();
 	for_each_cpu(i, cpu_map) {
-- 
1.8.3.1
```
## [PATCH v4 03/10] sched/topology: Provide cfs_overload_cpus bitmap

```patch
From: Steve Sistare <steve.sistare@oracle.com>

Define and initialize a sparse bitmap of overloaded CPUs, per
last-level-cache scheduling domain, for use by the CFS scheduling class.
Save a pointer to cfs_overload_cpus in the rq for efficient access.

Signed-off-by: Steve Sistare <steven.sistare@oracle.com>
---
 include/linux/sched/topology.h |  1 +
 kernel/sched/sched.h           |  2 ++
 kernel/sched/topology.c        | 25 +++++++++++++++++++++++--
 3 files changed, 26 insertions(+), 2 deletions(-)

diff --git a/include/linux/sched/topology.h b/include/linux/sched/topology.h
index 6b99761..b173a77 100644
--- a/include/linux/sched/topology.h
+++ b/include/linux/sched/topology.h
@@ -72,6 +72,7 @@ struct sched_domain_shared {
 	atomic_t	ref;
 	atomic_t	nr_busy_cpus;
 	int		has_idle_cores;
+	struct sparsemask *cfs_overload_cpus;
 };
 
 struct sched_domain {
diff --git a/kernel/sched/sched.h b/kernel/sched/sched.h
index 618577f..eacf5db 100644
--- a/kernel/sched/sched.h
+++ b/kernel/sched/sched.h
@@ -81,6 +81,7 @@
 
 struct rq;
 struct cpuidle_state;
+struct sparsemask;
 
 /* task_struct::on_rq states: */
 #define TASK_ON_RQ_QUEUED	1
@@ -812,6 +813,7 @@ struct rq {
 	struct cfs_rq		cfs;
 	struct rt_rq		rt;
 	struct dl_rq		dl;
+	struct sparsemask	*cfs_overload_cpus;
 
 #ifdef CONFIG_FAIR_GROUP_SCHED
 	/* list of leaf cfs_rq on this CPU: */
diff --git a/kernel/sched/topology.c b/kernel/sched/topology.c
index 3e72ce0..89a78ce 100644
--- a/kernel/sched/topology.c
+++ b/kernel/sched/topology.c
@@ -3,6 +3,7 @@
  * Scheduler topology setup/handling methods
  */
 #include "sched.h"
+#include "sparsemask.h"
 
 DEFINE_MUTEX(sched_domains_mutex);
 
@@ -410,7 +411,9 @@ static void destroy_sched_domains(struct sched_domain *sd)
 
 static void update_top_cache_domain(int cpu)
 {
+	struct sparsemask *cfs_overload_cpus = NULL;
 	struct sched_domain_shared *sds = NULL;
+	struct rq *rq = cpu_rq(cpu);
 	struct sched_domain *sd;
 	int id = cpu;
 	int size = 1;
@@ -420,8 +423,10 @@ static void update_top_cache_domain(int cpu)
 		id = cpumask_first(sched_domain_span(sd));
 		size = cpumask_weight(sched_domain_span(sd));
 		sds = sd->shared;
+		cfs_overload_cpus = sds->cfs_overload_cpus;
 	}
 
+	rcu_assign_pointer(rq->cfs_overload_cpus, cfs_overload_cpus);
 	rcu_assign_pointer(per_cpu(sd_llc, cpu), sd);
 	per_cpu(sd_llc_size, cpu) = size;
 	per_cpu(sd_llc_id, cpu) = id;
@@ -1621,7 +1626,22 @@ static void __sdt_free(const struct cpumask *cpu_map)
 
 static int sd_llc_alloc(struct sched_domain *sd)
 {
-	/* Allocate sd->shared data here. Empty for now. */
+	struct sched_domain_shared *sds = sd->shared;
+	struct cpumask *span = sched_domain_span(sd);
+	int nid = cpu_to_node(cpumask_first(span));
+	int flags = __GFP_ZERO | GFP_KERNEL;
+	struct sparsemask *mask;
+
+	/*
+	 * Allocate the bitmap if not already allocated.  This is called for
+	 * every CPU in the LLC but only allocates once per sd_llc_shared.
+	 */
+	if (!sds->cfs_overload_cpus) {
+		mask = sparsemask_alloc_node(nr_cpu_ids, 3, flags, nid);
+		if (!mask)
+			return 1;
+		sds->cfs_overload_cpus = mask;
+	}
 
 	return 0;
 }
@@ -1633,7 +1653,8 @@ static void sd_llc_free(struct sched_domain *sd)
 	if (!sds)
 		return;
 
-	/* Free data here. Empty for now. */
+	sparsemask_free(sds->cfs_overload_cpus);
+	sds->cfs_overload_cpus = NULL;
 }
 
 static int sd_llc_alloc_all(const struct cpumask *cpu_map, struct s_data *d)
-- 
1.8.3.1
```
## [PATCH v4 04/10] sched/fair: Dynamically update cfs_overload_cpus

```patch
An overloaded CPU has more than 1 runnable task.  When a CFS task wakes
on a CPU, if h_nr_running transitions from 1 to more, then set the CPU in
the cfs_overload_cpus bitmap.  When a CFS task sleeps, if h_nr_running
transitions from 2 to less, then clear the CPU in cfs_overload_cpus.

Signed-off-by: Steve Sistare <steven.sistare@oracle.com>
---
 kernel/sched/fair.c | 52 ++++++++++++++++++++++++++++++++++++++++++++++++----
 1 file changed, 48 insertions(+), 4 deletions(-)

diff --git a/kernel/sched/fair.c b/kernel/sched/fair.c
index ee271bb..4e105db 100644
--- a/kernel/sched/fair.c
+++ b/kernel/sched/fair.c
@@ -21,6 +21,7 @@
  *  Copyright (C) 2007 Red Hat, Inc., Peter Zijlstra
  */
 #include "sched.h"
+#include "sparsemask.h"
 
 #include <trace/events/sched.h>
 
@@ -3724,6 +3725,28 @@ static inline void update_misfit_status(struct task_struct *p, struct rq *rq)
 	rq->misfit_task_load = task_h_load(p);
 }
 
+static void overload_clear(struct rq *rq)
+{
+	struct sparsemask *overload_cpus;
+
+	rcu_read_lock();
+	overload_cpus = rcu_dereference(rq->cfs_overload_cpus);
+	if (overload_cpus)
+		sparsemask_clear_elem(overload_cpus, rq->cpu);
+	rcu_read_unlock();
+}
+
+static void overload_set(struct rq *rq)
+{
+	struct sparsemask *overload_cpus;
+
+	rcu_read_lock();
+	overload_cpus = rcu_dereference(rq->cfs_overload_cpus);
+	if (overload_cpus)
+		sparsemask_set_elem(overload_cpus, rq->cpu);
+	rcu_read_unlock();
+}
+
 #else /* CONFIG_SMP */
 
 #define UPDATE_TG	0x0
@@ -3747,6 +3770,9 @@ static inline int idle_balance(struct rq *rq, struct rq_flags *rf)
 	return 0;
 }
 
+static inline void overload_clear(struct rq *rq) {}
+static inline void overload_set(struct rq *rq) {}
+
 static inline void
 util_est_enqueue(struct cfs_rq *cfs_rq, struct task_struct *p) {}
 
@@ -4441,6 +4467,7 @@ static int tg_throttle_down(struct task_group *tg, void *data)
 static void throttle_cfs_rq(struct cfs_rq *cfs_rq)
 {
 	struct rq *rq = rq_of(cfs_rq);
+	unsigned int prev_nr = rq->cfs.h_nr_running;
 	struct cfs_bandwidth *cfs_b = tg_cfs_bandwidth(cfs_rq->tg);
 	struct sched_entity *se;
 	long task_delta, dequeue = 1;
@@ -4468,8 +4495,12 @@ static void throttle_cfs_rq(struct cfs_rq *cfs_rq)
 			dequeue = 0;
 	}
 
-	if (!se)
+	if (!se) {
 		sub_nr_running(rq, task_delta);
+		if (prev_nr >= 2 && prev_nr - task_delta < 2)
+			overload_clear(rq);
+
+	}
 
 	cfs_rq->throttled = 1;
 	cfs_rq->throttled_clock = rq_clock(rq);
@@ -4499,6 +4530,7 @@ static void throttle_cfs_rq(struct cfs_rq *cfs_rq)
 void unthrottle_cfs_rq(struct cfs_rq *cfs_rq)
 {
 	struct rq *rq = rq_of(cfs_rq);
+	unsigned int prev_nr = rq->cfs.h_nr_running;
 	struct cfs_bandwidth *cfs_b = tg_cfs_bandwidth(cfs_rq->tg);
 	struct sched_entity *se;
 	int enqueue = 1;
@@ -4535,8 +4567,11 @@ void unthrottle_cfs_rq(struct cfs_rq *cfs_rq)
 			break;
 	}
 
-	if (!se)
+	if (!se) {
 		add_nr_running(rq, task_delta);
+		if (prev_nr < 2 && prev_nr + task_delta >= 2)
+			overload_set(rq);
+	}
 
 	/* Determine whether we need to wake up potentially idle CPU: */
 	if (rq->curr == rq->idle && rq->cfs.nr_running)
@@ -5082,6 +5117,7 @@ static inline void hrtick_update(struct rq *rq)
 {
 	struct cfs_rq *cfs_rq;
 	struct sched_entity *se = &p->se;
+	unsigned int prev_nr = rq->cfs.h_nr_running;
 
 	/*
 	 * The code below (indirectly) updates schedutil which looks at
@@ -5129,8 +5165,12 @@ static inline void hrtick_update(struct rq *rq)
 		update_cfs_group(se);
 	}
 
-	if (!se)
+	if (!se) {
 		add_nr_running(rq, 1);
+		if (prev_nr == 1)
+			overload_set(rq);
+
+	}
 
 	hrtick_update(rq);
 }
@@ -5147,6 +5187,7 @@ static void dequeue_task_fair(struct rq *rq, struct task_struct *p, int flags)
 	struct cfs_rq *cfs_rq;
 	struct sched_entity *se = &p->se;
 	int task_sleep = flags & DEQUEUE_SLEEP;
+	unsigned int prev_nr = rq->cfs.h_nr_running;
 
 	for_each_sched_entity(se) {
 		cfs_rq = cfs_rq_of(se);
@@ -5188,8 +5229,11 @@ static void dequeue_task_fair(struct rq *rq, struct task_struct *p, int flags)
 		update_cfs_group(se);
 	}
 
-	if (!se)
+	if (!se) {
 		sub_nr_running(rq, 1);
+		if (prev_nr == 2)
+			overload_clear(rq);
+	}
 
 	util_est_dequeue(&rq->cfs, p, task_sleep);
 	hrtick_update(rq);
-- 
1.8.3.1
```
## [PATCH v4 05/10] sched/fair: Hoist idle_stamp up from idle_balance

```patch
Move the update of idle_stamp from idle_balance to the call site in
pick_next_task_fair, to prepare for a future patch that adds work to
pick_next_task_fair which must be included in the idle_stamp interval.
No functional change.

Signed-off-by: Steve Sistare <steven.sistare@oracle.com>
---
 kernel/sched/fair.c | 31 ++++++++++++++++++++++---------
 1 file changed, 22 insertions(+), 9 deletions(-)

diff --git a/kernel/sched/fair.c b/kernel/sched/fair.c
index 4e105db..8a33ad9 100644
--- a/kernel/sched/fair.c
+++ b/kernel/sched/fair.c
@@ -3725,6 +3725,16 @@ static inline void update_misfit_status(struct task_struct *p, struct rq *rq)
 	rq->misfit_task_load = task_h_load(p);
 }
 
+static inline void rq_idle_stamp_update(struct rq *rq)
+{
+	rq->idle_stamp = rq_clock(rq);
+}
+
+static inline void rq_idle_stamp_clear(struct rq *rq)
+{
+	rq->idle_stamp = 0;
+}
+
 static void overload_clear(struct rq *rq)
 {
 	struct sparsemask *overload_cpus;
@@ -3770,6 +3780,8 @@ static inline int idle_balance(struct rq *rq, struct rq_flags *rf)
 	return 0;
 }
 
+static inline void rq_idle_stamp_update(struct rq *rq) {}
+static inline void rq_idle_stamp_clear(struct rq *rq) {}
 static inline void overload_clear(struct rq *rq) {}
 static inline void overload_set(struct rq *rq) {}
 
@@ -6764,8 +6776,18 @@ static void check_preempt_wakeup(struct rq *rq, struct task_struct *p, int wake_
 
 idle:
 	update_misfit_status(NULL, rq);
+
+	/*
+	 * We must set idle_stamp _before_ calling idle_balance(), such that we
+	 * measure the duration of idle_balance() as idle time.
+	 */
+	rq_idle_stamp_update(rq);
+
 	new_tasks = idle_balance(rq, rf);
 
+	if (new_tasks)
+		rq_idle_stamp_clear(rq);
+
 	/*
 	 * Because idle_balance() releases (and re-acquires) rq->lock, it is
 	 * possible for any higher priority task to appear. In that case we
@@ -9611,12 +9633,6 @@ static int idle_balance(struct rq *this_rq, struct rq_flags *rf)
 	u64 curr_cost = 0;
 
 	/*
-	 * We must set idle_stamp _before_ calling idle_balance(), such that we
-	 * measure the duration of idle_balance() as idle time.
-	 */
-	this_rq->idle_stamp = rq_clock(this_rq);
-
-	/*
 	 * Do not pull tasks towards !active CPUs...
 	 */
 	if (!cpu_active(this_cpu))
@@ -9707,9 +9723,6 @@ static int idle_balance(struct rq *this_rq, struct rq_flags *rf)
 	if (this_rq->nr_running != this_rq->cfs.h_nr_running)
 		pulled_task = -1;
 
-	if (pulled_task)
-		this_rq->idle_stamp = 0;
-
 	rq_repin_lock(this_rq, rf);
 
 	return pulled_task;
-- 
1.8.3.1
```
## [PATCH v4 06/10] sched/fair: Generalize the detach_task interface

```patch
The detach_task function takes a struct lb_env argument, but only needs a
few of its members.  Pass the rq and cpu arguments explicitly so the
function may be called from code that is not based on lb_env.  No
functional change.

Signed-off-by: Steve Sistare <steven.sistare@oracle.com>
---
 kernel/sched/fair.c | 14 +++++++-------
 1 file changed, 7 insertions(+), 7 deletions(-)

diff --git a/kernel/sched/fair.c b/kernel/sched/fair.c
index 8a33ad9..9b7c85b 100644
--- a/kernel/sched/fair.c
+++ b/kernel/sched/fair.c
@@ -7207,15 +7207,15 @@ int can_migrate_task(struct task_struct *p, struct lb_env *env)
 }
 
 /*
- * detach_task() -- detach the task for the migration specified in env
+ * detach_task() -- detach the task for the migration from @src_rq to @dst_cpu.
  */
-static void detach_task(struct task_struct *p, struct lb_env *env)
+static void detach_task(struct task_struct *p, struct rq *src_rq, int dst_cpu)
 {
-	lockdep_assert_held(&env->src_rq->lock);
+	lockdep_assert_held(&src_rq->lock);
 
 	p->on_rq = TASK_ON_RQ_MIGRATING;
-	deactivate_task(env->src_rq, p, DEQUEUE_NOCLOCK);
-	set_task_cpu(p, env->dst_cpu);
+	deactivate_task(src_rq, p, DEQUEUE_NOCLOCK);
+	set_task_cpu(p, dst_cpu);
 }
 
 /*
@@ -7235,7 +7235,7 @@ static struct task_struct *detach_one_task(struct lb_env *env)
 		if (!can_migrate_task(p, env))
 			continue;
 
-		detach_task(p, env);
+		detach_task(p, env->src_rq, env->dst_cpu);
 
 		/*
 		 * Right now, this is only the second place where
@@ -7302,7 +7302,7 @@ static int detach_tasks(struct lb_env *env)
 		if ((load / 2) > env->imbalance)
 			goto next;
 
-		detach_task(p, env);
+		detach_task(p, env->src_rq, env->dst_cpu);
 		list_add(&p->se.group_node, &env->tasks);
 
 		detached++;
-- 
1.8.3.1
```

## [PATCH v4 07/10] sched/fair: Provide can_migrate_task_llc

```patch
Define a simpler version of can_migrate_task called can_migrate_task_llc
which does not require a struct lb_env argument, and judges whether a
migration from one CPU to another within the same LLC should be allowed.

Signed-off-by: Steve Sistare <steven.sistare@oracle.com>
---
 kernel/sched/fair.c | 28 ++++++++++++++++++++++++++++
 1 file changed, 28 insertions(+)

diff --git a/kernel/sched/fair.c b/kernel/sched/fair.c
index 9b7c85b..3804156 100644
--- a/kernel/sched/fair.c
+++ b/kernel/sched/fair.c
@@ -7207,6 +7207,34 @@ int can_migrate_task(struct task_struct *p, struct lb_env *env)
 }
 
 /*
+ * Return true if task @p can migrate from @rq to @dst_rq in the same LLC.
+ * No need to test for co-locality, and no need to test task_hot(), as sharing
+ * LLC provides cache warmth at that level.
+ */
+static bool
+can_migrate_task_llc(struct task_struct *p, struct rq *rq, struct rq *dst_rq)
+{
+	int dst_cpu = dst_rq->cpu;
+
+	lockdep_assert_held(&rq->lock);
+
+	if (throttled_lb_pair(task_group(p), cpu_of(rq), dst_cpu))
+		return false;
+
+	if (!cpumask_test_cpu(dst_cpu, &p->cpus_allowed)) {
+		schedstat_inc(p->se.statistics.nr_failed_migrations_affine);
+		return false;
+	}
+
+	if (task_running(rq, p)) {
+		schedstat_inc(p->se.statistics.nr_failed_migrations_running);
+		return false;
+	}
+
+	return true;
+}
+
+/*
  * detach_task() -- detach the task for the migration from @src_rq to @dst_cpu.
  */
 static void detach_task(struct task_struct *p, struct rq *src_rq, int dst_cpu)
-- 
1.8.3.1
```

## [PATCH v4 08/10] sched/fair: Steal work from an overloaded CPU when CPU goes idle

```patch
When a CPU has no more CFS tasks to run, and idle_balance() fails to find a
task, then attempt to steal a task from an overloaded CPU in the same LLC,
using the cfs_overload_cpus bitmap to efficiently identify candidates.  To
minimize search time, steal the first migratable task that is found when
the bitmap is traversed.  For fairness, search for migratable tasks on an
overloaded CPU in order of next to run.

This simple stealing yields a higher CPU utilization than idle_balance()
alone, because the search is cheap, so it may be called every time the CPU
is about to go idle.  idle_balance() does more work because it searches
widely for the busiest queue, so to limit its CPU consumption, it declines
to search if the system is too busy.  Simple stealing does not offload the
globally busiest queue, but it is much better than running nothing at all.

Stealing is controlled by the sched feature SCHED_STEAL, which is enabled
by default.

Stealing imprroves utilization with only a modest CPU overhead in scheduler
code.  In the following experiment, hackbench is run with varying numbers
of groups (40 tasks per group), and the delta in /proc/schedstat is shown
for each run, averaged per CPU, augmented with these non-standard stats:

  %find - percent of time spent in old and new functions that search for
    idle CPUs and tasks to steal and set the overloaded CPUs bitmap.

  steal - number of times a task is stolen from another CPU.

X6-2: 1 socket * 10 cores * 2 hyperthreads = 20 CPUs
Intel(R) Xeon(R) CPU E5-2630 v4 @ 2.20GHz
hackbench <grps> process 100000
sched_wakeup_granularity_ns=15000000

  baseline
  grps  time  %busy  slice   sched   idle     wake %find  steal
  1    8.084  75.02   0.10  105476  46291    59183  0.31      0
  2   13.892  85.33   0.10  190225  70958   119264  0.45      0
  3   19.668  89.04   0.10  263896  87047   176850  0.49      0
  4   25.279  91.28   0.10  322171  94691   227474  0.51      0
  8   47.832  94.86   0.09  630636 144141   486322  0.56      0

  new
  grps  time  %busy  slice   sched   idle     wake %find  steal  %speedup
  1    5.938  96.80   0.24   31255   7190    24061  0.63   7433  36.1
  2   11.491  99.23   0.16   74097   4578    69512  0.84  19463  20.9
  3   16.987  99.66   0.15  115824   1985   113826  0.77  24707  15.8
  4   22.504  99.80   0.14  167188   2385   164786  0.75  29353  12.3
  8   44.441  99.86   0.11  389153   1616   387401  0.67  38190   7.6

Elapsed time improves by 8 to 36%, and CPU busy utilization is up
by 5 to 22% hitting 99% for 2 or more groups (80 or more tasks).
The cost is at most 0.4% more find time.

Additional performance results follow.  A negative "speedup" is a
regression.  Note: for all hackbench runs, sched_wakeup_granularity_ns
is set to 15 msec.  Otherwise, preemptions increase at higher loads and
distort the comparison between baseline and new.

------------------ 1 Socket Results ------------------

X6-2: 1 socket * 10 cores * 2 hyperthreads = 20 CPUs
Intel(R) Xeon(R) CPU E5-2630 v4 @ 2.20GHz
Average of 10 runs of: hackbench <groups> process 100000

            --- base --    --- new ---
  groups    time %stdev    time %stdev  %speedup
       1   8.008    0.1   5.905    0.2      35.6
       2  13.814    0.2  11.438    0.1      20.7
       3  19.488    0.2  16.919    0.1      15.1
       4  25.059    0.1  22.409    0.1      11.8
       8  47.478    0.1  44.221    0.1       7.3

X6-2: 1 socket * 22 cores * 2 hyperthreads = 44 CPUs
Intel(R) Xeon(R) CPU E5-2699 v4 @ 2.20GHz
Average of 10 runs of: hackbench <groups> process 100000

            --- base --    --- new ---
  groups    time %stdev    time %stdev  %speedup
       1   4.586    0.8   4.596    0.6      -0.3
       2   7.693    0.2   5.775    1.3      33.2
       3  10.442    0.3   8.288    0.3      25.9
       4  13.087    0.2  11.057    0.1      18.3
       8  24.145    0.2  22.076    0.3       9.3
      16  43.779    0.1  41.741    0.2       4.8

KVM 4-cpu
Intel(R) Xeon(R) CPU E5-2699 v3 @ 2.30GHz
tbench, average of 11 runs

  clients    %speedup
        1        16.2
        2        11.7
        4         9.9
        8        12.8
       16        13.7

KVM 2-cpu
Intel(R) Xeon(R) CPU E5-2699 v3 @ 2.30GHz

  Benchmark                     %speedup
  specjbb2015_critical_jops          5.7
  mysql_sysb1.0.14_mutex_2          40.6
  mysql_sysb1.0.14_oltp_2            3.9

------------------ 2 Socket Results ------------------

X6-2: 2 sockets * 10 cores * 2 hyperthreads = 40 CPUs
Intel(R) Xeon(R) CPU E5-2630 v4 @ 2.20GHz
Average of 10 runs of: hackbench <groups> process 100000

            --- base --    --- new ---
  groups    time %stdev    time %stdev  %speedup
       1   7.945    0.2   7.219    8.7      10.0
       2   8.444    0.4   6.689    1.5      26.2
       3  12.100    1.1   9.962    2.0      21.4
       4  15.001    0.4  13.109    1.1      14.4
       8  27.960    0.2  26.127    0.3       7.0

X6-2: 2 sockets * 22 cores * 2 hyperthreads = 88 CPUs
Intel(R) Xeon(R) CPU E5-2699 v4 @ 2.20GHz
Average of 10 runs of: hackbench <groups> process 100000

            --- base --    --- new ---
  groups    time %stdev    time %stdev  %speedup
       1   5.826    5.4   5.840    5.0      -0.3
       2   5.041    5.3   6.171   23.4     -18.4
       3   6.839    2.1   6.324    3.8       8.1
       4   8.177    0.6   7.318    3.6      11.7
       8  14.429    0.7  13.966    1.3       3.3
      16  26.401    0.3  25.149    1.5       4.9

X6-2: 2 sockets * 22 cores * 2 hyperthreads = 88 CPUs
Intel(R) Xeon(R) CPU E5-2699 v4 @ 2.20GHz
Oracle database OLTP, logging disabled, NVRAM storage

  Customers   Users   %speedup
    1200000      40       -1.2
    2400000      80        2.7
    3600000     120        8.9
    4800000     160        4.4
    6000000     200        3.0

X6-2: 2 sockets * 14 cores * 2 hyperthreads = 56 CPUs
Intel(R) Xeon(R) CPU E5-2690 v4 @ 2.60GHz
Results from the Oracle "Performance PIT".

  Benchmark                                           %speedup

  mysql_sysb1.0.14_fileio_56_rndrd                        19.6
  mysql_sysb1.0.14_fileio_56_seqrd                        12.1
  mysql_sysb1.0.14_fileio_56_rndwr                         0.4
  mysql_sysb1.0.14_fileio_56_seqrewr                      -0.3

  pgsql_sysb1.0.14_fileio_56_rndrd                        19.5
  pgsql_sysb1.0.14_fileio_56_seqrd                         8.6
  pgsql_sysb1.0.14_fileio_56_rndwr                         1.0
  pgsql_sysb1.0.14_fileio_56_seqrewr                       0.5

  opatch_time_ASM_12.2.0.1.0_HP2M                          7.5
  select-1_users-warm_asmm_ASM_12.2.0.1.0_HP2M             5.1
  select-1_users_asmm_ASM_12.2.0.1.0_HP2M                  4.4
  swingbenchv3_asmm_soebench_ASM_12.2.0.1.0_HP2M           5.8

  lm3_memlat_L2                                            4.8
  lm3_memlat_L1                                            0.0

  ub_gcc_56CPUs-56copies_Pipe-based_Context_Switching     60.1
  ub_gcc_56CPUs-56copies_Shell_Scripts_1_concurrent        5.2
  ub_gcc_56CPUs-56copies_Shell_Scripts_8_concurrent       -3.0
  ub_gcc_56CPUs-56copies_File_Copy_1024_bufsize_2000_maxblocks 2.4

X5-2: 2 sockets * 18 cores * 2 hyperthreads = 72 CPUs
Intel(R) Xeon(R) CPU E5-2699 v3 @ 2.30GHz

  NAS_OMP
  bench class   ncpu    %improved(Mops)
  dc    B       72      1.3
  is    C       72      0.9
  is    D       72      0.7

  sysbench mysql, average of 24 runs
          --- base ---     --- new ---
  nthr   events  %stdev   events  %stdev %speedup
     1    331.0    0.25    331.0    0.24     -0.1
     2    661.3    0.22    661.8    0.22      0.0
     4   1297.0    0.88   1300.5    0.82      0.2
     8   2420.8    0.04   2420.5    0.04     -0.1
    16   4826.3    0.07   4825.4    0.05     -0.1
    32   8815.3    0.27   8830.2    0.18      0.1
    64  12823.0    0.24  12823.6    0.26      0.0

-------------------------------------------------------------

Signed-off-by: Steve Sistare <steven.sistare@oracle.com>
---
 kernel/sched/fair.c     | 169 ++++++++++++++++++++++++++++++++++++++++++++++--
 kernel/sched/features.h |   6 ++
 2 files changed, 170 insertions(+), 5 deletions(-)

diff --git a/kernel/sched/fair.c b/kernel/sched/fair.c
index 3804156..1476ae8 100644
--- a/kernel/sched/fair.c
+++ b/kernel/sched/fair.c
@@ -3739,6 +3739,9 @@ static void overload_clear(struct rq *rq)
 {
 	struct sparsemask *overload_cpus;
 
+	if (!sched_feat(STEAL))
+		return;
+
 	rcu_read_lock();
 	overload_cpus = rcu_dereference(rq->cfs_overload_cpus);
 	if (overload_cpus)
@@ -3750,6 +3753,9 @@ static void overload_set(struct rq *rq)
 {
 	struct sparsemask *overload_cpus;
 
+	if (!sched_feat(STEAL))
+		return;
+
 	rcu_read_lock();
 	overload_cpus = rcu_dereference(rq->cfs_overload_cpus);
 	if (overload_cpus)
@@ -3757,6 +3763,8 @@ static void overload_set(struct rq *rq)
 	rcu_read_unlock();
 }
 
+static int try_steal(struct rq *this_rq, struct rq_flags *rf);
+
 #else /* CONFIG_SMP */
 
 #define UPDATE_TG	0x0
@@ -3793,6 +3801,11 @@ static inline void overload_set(struct rq *rq) {}
 		 bool task_sleep) {}
 static inline void update_misfit_status(struct task_struct *p, struct rq *rq) {}
 
+static inline int try_steal(struct rq *this_rq, struct rq_flags *rf)
+{
+	return 0;
+}
+
 #endif /* CONFIG_SMP */
 
 static void check_spread(struct cfs_rq *cfs_rq, struct sched_entity *se)
@@ -6778,20 +6791,22 @@ static void check_preempt_wakeup(struct rq *rq, struct task_struct *p, int wake_
 	update_misfit_status(NULL, rq);
 
 	/*
-	 * We must set idle_stamp _before_ calling idle_balance(), such that we
-	 * measure the duration of idle_balance() as idle time.
+	 * We must set idle_stamp _before_ calling try_steal() or
+	 * idle_balance(), such that we measure the duration as idle time.
 	 */
 	rq_idle_stamp_update(rq);
 
 	new_tasks = idle_balance(rq, rf);
+	if (new_tasks == 0)
+		new_tasks = try_steal(rq, rf);
 
 	if (new_tasks)
 		rq_idle_stamp_clear(rq);
 
 	/*
-	 * Because idle_balance() releases (and re-acquires) rq->lock, it is
-	 * possible for any higher priority task to appear. In that case we
-	 * must re-start the pick_next_entity() loop.
+	 * Because try_steal() and idle_balance() release (and re-acquire)
+	 * rq->lock, it is possible for any higher priority task to appear.
+	 * In that case we must re-start the pick_next_entity() loop.
 	 */
 	if (new_tasks < 0)
 		return RETRY_TASK;
@@ -9797,6 +9812,150 @@ void trigger_load_balance(struct rq *rq)
 	nohz_balancer_kick(rq);
 }
 
+/*
+ * Search the runnable tasks in @cfs_rq in order of next to run, and find
+ * the first one that can be migrated to @dst_rq.  @cfs_rq is locked on entry.
+ * On success, dequeue the task from @cfs_rq and return it, else return NULL.
+ */
+static struct task_struct *
+detach_next_task(struct cfs_rq *cfs_rq, struct rq *dst_rq)
+{
+	int dst_cpu = dst_rq->cpu;
+	struct task_struct *p;
+	struct rq *rq = rq_of(cfs_rq);
+
+	lockdep_assert_held(&rq_of(cfs_rq)->lock);
+
+	list_for_each_entry_reverse(p, &rq->cfs_tasks, se.group_node) {
+		if (can_migrate_task_llc(p, rq, dst_rq)) {
+			detach_task(p, rq, dst_cpu);
+			return p;
+		}
+	}
+	return NULL;
+}
+
+/*
+ * Attempt to migrate a CFS task from @src_cpu to @dst_rq.  @locked indicates
+ * whether @dst_rq is already locked on entry.  This function may lock or
+ * unlock @dst_rq, and updates @locked to indicate the locked state on return.
+ * The locking protocol is based on idle_balance().
+ * Returns 1 on success and 0 on failure.
+ */
+static int steal_from(struct rq *dst_rq, struct rq_flags *dst_rf, bool *locked,
+		      int src_cpu)
+{
+	struct task_struct *p;
+	struct rq_flags rf;
+	int stolen = 0;
+	int dst_cpu = dst_rq->cpu;
+	struct rq *src_rq = cpu_rq(src_cpu);
+
+	if (dst_cpu == src_cpu || src_rq->cfs.h_nr_running < 2)
+		return 0;
+
+	if (*locked) {
+		rq_unpin_lock(dst_rq, dst_rf);
+		raw_spin_unlock(&dst_rq->lock);
+		*locked = false;
+	}
+	rq_lock_irqsave(src_rq, &rf);
+	update_rq_clock(src_rq);
+
+	if (src_rq->cfs.h_nr_running < 2 || !cpu_active(src_cpu))
+		p = NULL;
+	else
+		p = detach_next_task(&src_rq->cfs, dst_rq);
+
+	rq_unlock(src_rq, &rf);
+
+	if (p) {
+		raw_spin_lock(&dst_rq->lock);
+		rq_repin_lock(dst_rq, dst_rf);
+		*locked = true;
+		update_rq_clock(dst_rq);
+		attach_task(dst_rq, p);
+		stolen = 1;
+	}
+	local_irq_restore(rf.flags);
+
+	return stolen;
+}
+
+/*
+ * Conservative upper bound on the max cost of a steal, in nsecs (the typical
+ * cost is 1-2 microsec).  Do not steal if average idle time is less.
+ */
+#define SCHED_STEAL_COST 10000
+
+/*
+ * Try to steal a runnable CFS task from a CPU in the same LLC as @dst_rq,
+ * and migrate it to @dst_rq.  rq_lock is held on entry and return, but
+ * may be dropped in between.  Return 1 on success, 0 on failure, and -1
+ * if a task in a different scheduling class has become runnable on @dst_rq.
+ */
+static int try_steal(struct rq *dst_rq, struct rq_flags *dst_rf)
+{
+	int src_cpu;
+	int dst_cpu = dst_rq->cpu;
+	bool locked = true;
+	int stolen = 0;
+	struct sparsemask *overload_cpus;
+
+	if (!sched_feat(STEAL))
+		return 0;
+
+	if (!cpu_active(dst_cpu))
+		return 0;
+
+	if (dst_rq->avg_idle < SCHED_STEAL_COST)
+		return 0;
+
+	/* Get bitmap of overloaded CPUs in the same LLC as @dst_rq */
+
+	rcu_read_lock();
+	overload_cpus = rcu_dereference(dst_rq->cfs_overload_cpus);
+	if (!overload_cpus) {
+		rcu_read_unlock();
+		return 0;
+	}
+
+#ifdef CONFIG_SCHED_SMT
+	/*
+	 * First try overloaded CPUs on the same core to preserve cache warmth.
+	 */
+	if (static_branch_likely(&sched_smt_present)) {
+		for_each_cpu(src_cpu, cpu_smt_mask(dst_cpu)) {
+			if (sparsemask_test_elem(overload_cpus, src_cpu) &&
+			    steal_from(dst_rq, dst_rf, &locked, src_cpu)) {
+				stolen = 1;
+				goto out;
+			}
+		}
+	}
+#endif	/* CONFIG_SCHED_SMT */
+
+	/* Accept any suitable task in the LLC */
+
+	sparsemask_for_each(overload_cpus, dst_cpu, src_cpu) {
+		if (steal_from(dst_rq, dst_rf, &locked, src_cpu)) {
+			stolen = 1;
+			goto out;
+		}
+	}
+
+out:
+	rcu_read_unlock();
+	if (!locked) {
+		raw_spin_lock(&dst_rq->lock);
+		rq_repin_lock(dst_rq, dst_rf);
+	}
+	stolen |= (dst_rq->cfs.h_nr_running > 0);
+	if (dst_rq->nr_running != dst_rq->cfs.h_nr_running)
+		stolen = -1;
+	return stolen;
+}
+
 static void rq_online_fair(struct rq *rq)
 {
 	update_sysctl();
diff --git a/kernel/sched/features.h b/kernel/sched/features.h
index 858589b..a665a9e 100644
--- a/kernel/sched/features.h
+++ b/kernel/sched/features.h
@@ -59,6 +59,12 @@
 SCHED_FEAT(SIS_PROP, true)
 
 /*
+ * Steal a CFS task from another CPU when going idle.
+ * Improves CPU utilization.
+ */
+SCHED_FEAT(STEAL, true)
+
+/*
  * Issue a WARN when we do multiple update_rq_clock() calls
  * in a single rq->lock section. Default disabled because the
  * annotations are not complete.
-- 
1.8.3.1
```

## [PATCH v4 09/10] sched/fair: disable stealing if too many NUMA nodes

```patch
The STEAL feature causes regressions on hackbench on larger NUMA systems,
so disable it on systems with more than sched_steal_node_limit nodes
(default 2).  Note that the feature remains enabled as seen in features.h
and /sys/kernel/debug/sched_features, but stealing is only performed if
nodes <= sched_steal_node_limit.  This arrangement allows users to activate
stealing on reboot by setting the kernel parameter sched_steal_node_limit
on kernels built without CONFIG_SCHED_DEBUG.  The parameter is temporary
and will be deleted when the regression is fixed.

Details of the regression follow.  With the STEAL feature set, hackbench
is slower on many-node systems:

X5-8: 8 sockets * 18 cores * 2 hyperthreads = 288 CPUs
Intel(R) Xeon(R) CPU E7-8895 v3 @ 2.60GHz
Average of 10 runs of: hackbench <groups> processes 50000

          --- base --    --- new ---
groups    time %stdev    time %stdev  %speedup
     1   3.627   15.8   3.876    7.3      -6.5
     2   4.545   24.7   5.583   16.7     -18.6
     3   5.716   25.0   7.367   14.2     -22.5
     4   6.901   32.9   7.718   14.5     -10.6
     8   8.604   38.5   9.111   16.0      -5.6
    16   7.734    6.8  11.007    8.2     -29.8

Total CPU time increases.  Profiling shows that CPU time increases
uniformly across all functions, suggesting a systemic increase in cache
or memory latency.  This may be due to NUMA migrations, as they cause
loss of LLC cache footprint and remote memory latencies.

The domains for this system and their flags are:

  domain0 (SMT) : 1 core
    SD_LOAD_BALANCE SD_BALANCE_NEWIDLE SD_BALANCE_EXEC SD_BALANCE_FORK
    SD_SHARE_PKG_RESOURCES SD_PREFER_SIBLING SD_SHARE_CPUCAPACITY
    SD_WAKE_AFFINE

  domain1 (MC) : 1 socket
    SD_LOAD_BALANCE SD_BALANCE_NEWIDLE SD_BALANCE_EXEC SD_BALANCE_FORK
    SD_SHARE_PKG_RESOURCES SD_PREFER_SIBLING
    SD_WAKE_AFFINE

  domain2 (NUMA) : 4 sockets
    SD_LOAD_BALANCE SD_BALANCE_NEWIDLE SD_BALANCE_EXEC SD_BALANCE_FORK
    SD_SERIALIZE SD_OVERLAP SD_NUMA
    SD_WAKE_AFFINE

  domain3 (NUMA) : 8 sockets
    SD_LOAD_BALANCE SD_BALANCE_NEWIDLE
    SD_SERIALIZE SD_OVERLAP SD_NUMA

Schedstats point to the root cause of the regression.  hackbench is run
10 times per group and the average schedstat accumulation per-run and
per-cpu is shown below.  Note that domain3 moves are zero because
SD_WAKE_AFFINE is not set there.

NO_STEAL
                                         --- domain2 ---   --- domain3 ---
grp time %busy sched  idle   wake steal remote  move pull remote  move pull
 1 20.3 10.3  28710  14346  14366     0    490  3378    0   4039     0    0
 2 26.4 18.8  56721  28258  28469     0    792  7026   12   9229     0    7
 3 29.9 28.3  90191  44933  45272     0   5380  7204   19  16481     0    3
 4 30.2 35.8 121324  60409  60933     0   7012  9372   27  21438     0    5
 8 27.7 64.2 229174 111917 117272     0  11991  1837  168  44006     0   32
16 32.6 74.0 334615 146784 188043     0   3404  1468   49  61405     0    8

STEAL
                                         --- domain2 ---   --- domain3 ---
grp time %busy sched  idle   wake steal remote  move pull remote  move pull
 1 20.6 10.2  28490  14232  14261    18      3  3525    0   4254     0    0
 2 27.9 18.8  56757  28203  28562   303   1675  7839    5   9690     0    2
 3 35.3 27.7  87337  43274  44085   698    741 12785   14  15689     0    3
 4 36.8 36.0 118630  58437  60216  1579   2973 14101   28  18732     0    7
 8 48.1 73.8 289374 133681 155600 18646  35340 10179  171  65889     0   34
16 41.4 82.5 268925  91908 177172 47498  17206  6940  176  71776     0   20

Cross-numa-node migrations are caused by load balancing pulls and
wake_affine moves.  Pulls are small and similar for no_steal and steal.
However, moves are significantly higher for steal, and rows above with the
highest moves have the worst regressions for time; see for example grp=8.

Moves increase for steal due to the following logic in wake_affine_idle()
for synchronous wakeup:

    if (sync && cpu_rq(this_cpu)->nr_running == 1)
        return this_cpu;        // move the task

The steal feature does a better job of smoothing the load between idle
and busy CPUs, so nr_running is 1 more often, and moves are performed
more often.  For hackbench, cross-node affine moves early in the run are
good because they colocate wakers and wakees from the same group on the
same node, but continued moves later in the run are bad, because the wakee
is moved away from peers on its previous node.  Note that even no_steal
is far from optimal; binding an instance of "hackbench 2" to each of the
8 NUMA nodes runs much faster than running "hackbench 16" with no binding.

Clearing SD_WAKE_AFFINE for domain2 eliminates the affine cross-node
migrations and eliminates the difference between no_steal and steal
performance.  However, overall performance is lower than WA_IDLE because
some migrations are helpful as explained above.

I have tried many heuristics in a attempt to optimize the number of
cross-node moves in all conditions, with limited success.  The fundamental
problem is that the scheduler does not track which groups of tasks talk to
each other.  Parts of several groups become entrenched on the same node,
filling it to capacity, leaving no room for either group to pull its peers
over, and there is neither data nor mechanism for the scheduler to evict
one group to make room for the other.

For now, disable STEAL on such systems until we can do better, or it is
shown that hackbench is atypical and most workloads benefit from stealing.

Signed-off-by: Steve Sistare <steven.sistare@oracle.com>
---
 kernel/sched/fair.c     | 16 +++++++++++++---
 kernel/sched/sched.h    |  2 +-
 kernel/sched/topology.c | 25 +++++++++++++++++++++++++
 3 files changed, 39 insertions(+), 4 deletions(-)

diff --git a/kernel/sched/fair.c b/kernel/sched/fair.c
index 1476ae8..1efd9c4 100644
--- a/kernel/sched/fair.c
+++ b/kernel/sched/fair.c
@@ -3735,11 +3735,21 @@ static inline void rq_idle_stamp_clear(struct rq *rq)
 	rq->idle_stamp = 0;
 }
 
+static inline bool steal_enabled(void)
+{
+#ifdef CONFIG_NUMA
+	bool allow = static_branch_likely(&sched_steal_allow);
+#else
+	bool allow = true;
+#endif
+	return sched_feat(STEAL) && allow;
+}
+
 static void overload_clear(struct rq *rq)
 {
 	struct sparsemask *overload_cpus;
 
-	if (!sched_feat(STEAL))
+	if (!steal_enabled())
 		return;
 
 	rcu_read_lock();
@@ -3753,7 +3763,7 @@ static void overload_set(struct rq *rq)
 {
 	struct sparsemask *overload_cpus;
 
-	if (!sched_feat(STEAL))
+	if (!steal_enabled())
 		return;
 
 	rcu_read_lock();
@@ -9902,7 +9912,7 @@ static int try_steal(struct rq *dst_rq, struct rq_flags *dst_rf)
 	int stolen = 0;
 	struct sparsemask *overload_cpus;
 
-	if (!sched_feat(STEAL))
+	if (!steal_enabled())
 		return 0;
 
 	if (!cpu_active(dst_cpu))
diff --git a/kernel/sched/sched.h b/kernel/sched/sched.h
index eacf5db..2a28340 100644
--- a/kernel/sched/sched.h
+++ b/kernel/sched/sched.h
@@ -936,7 +936,6 @@ static inline int cpu_of(struct rq *rq)
 #endif
 }
 
-
 #ifdef CONFIG_SCHED_SMT
 
 extern struct static_key_false sched_smt_present;
@@ -1185,6 +1184,7 @@ enum numa_topology_type {
 #endif
 
 #ifdef CONFIG_NUMA
+extern struct static_key_true sched_steal_allow;
 extern void sched_init_numa(void);
 extern void sched_domains_numa_masks_set(unsigned int cpu);
 extern void sched_domains_numa_masks_clear(unsigned int cpu);
diff --git a/kernel/sched/topology.c b/kernel/sched/topology.c
index 89a78ce..259d659 100644
--- a/kernel/sched/topology.c
+++ b/kernel/sched/topology.c
@@ -1344,6 +1344,30 @@ static void init_numa_topology_type(void)
 	}
 }
 
+DEFINE_STATIC_KEY_TRUE(sched_steal_allow);
+static int sched_steal_node_limit;
+#define SCHED_STEAL_NODE_LIMIT_DEFAULT 2
+
+static int __init steal_node_limit_setup(char *buf)
+{
+	get_option(&buf, &sched_steal_node_limit);
+	return 0;
+}
+
+early_param("sched_steal_node_limit", steal_node_limit_setup);
+
+static void check_node_limit(void)
+{
+	int n = num_possible_nodes();
+
+	if (sched_steal_node_limit == 0)
+		sched_steal_node_limit = SCHED_STEAL_NODE_LIMIT_DEFAULT;
+	if (n > sched_steal_node_limit) {
+		static_branch_disable(&sched_steal_allow);
+		pr_debug("Suppressing sched STEAL. To enable, reboot with sched_steal_node_limit=%d", n);
+	}
+}
+
 void sched_init_numa(void)
 {
 	int next_distance, curr_distance = node_distance(0, 0);
@@ -1492,6 +1516,7 @@ void sched_init_numa(void)
 	sched_max_numa_distance = sched_domains_numa_distance[level - 1];
 
 	init_numa_topology_type();
+	check_node_limit();
 }
 
 void sched_domains_numa_masks_set(unsigned int cpu)
-- 
1.8.3.1
```
## [PATCH v4 10/10] sched/fair: Provide idle search schedstats

```patch
Add schedstats to measure the effectiveness of searching for idle CPUs
and stealing tasks.  This is a temporary patch intended for use during
development only.  SCHEDSTAT_VERSION is bumped to 16, and the following
fields are added to the per-CPU statistics of /proc/schedstat:

field 10: # of times select_idle_sibling "easily" found an idle CPU --
          prev or target is idle.
field 11: # of times select_idle_sibling searched and found an idle cpu.
field 12: # of times select_idle_sibling searched and found an idle core.
field 13: # of times select_idle_sibling failed to find anything idle.
field 14: time in nanoseconds spent in functions that search for idle
          CPUs and search for tasks to steal.
field 15: # of times an idle CPU steals a task from another CPU.
field 16: # of times try_steal finds overloaded CPUs but no task is
           migratable.

Signed-off-by: Steve Sistare <steven.sistare@oracle.com>
---
 kernel/sched/core.c  | 31 ++++++++++++++++++++++++++++--
 kernel/sched/fair.c  | 54 ++++++++++++++++++++++++++++++++++++++++++++++------
 kernel/sched/sched.h |  9 +++++++++
 kernel/sched/stats.c | 11 ++++++++++-
 kernel/sched/stats.h | 13 +++++++++++++
 5 files changed, 109 insertions(+), 9 deletions(-)

diff --git a/kernel/sched/core.c b/kernel/sched/core.c
index f12225f..14ee88b 100644
--- a/kernel/sched/core.c
+++ b/kernel/sched/core.c
@@ -2220,17 +2220,44 @@ int sysctl_numa_balancing(struct ctl_table *table, int write,
 DEFINE_STATIC_KEY_FALSE(sched_schedstats);
 static bool __initdata __sched_schedstats = false;
 
+unsigned long schedstat_skid;
+
+static void compute_skid(void)
+{
+	int i, n = 0;
+	s64 t;
+	int skid = 0;
+
+	for (i = 0; i < 100; i++) {
+		t = local_clock();
+		t = local_clock() - t;
+		if (t > 0 && t < 1000) {	/* only use sane samples */
+			skid += (int) t;
+			n++;
+		}
+	}
+
+	if (n > 0)
+		schedstat_skid = skid / n;
+	else
+		schedstat_skid = 0;
+	pr_info("schedstat_skid = %lu\n", schedstat_skid);
+}
+
 static void set_schedstats(bool enabled)
 {
-	if (enabled)
+	if (enabled) {
+		compute_skid();
 		static_branch_enable(&sched_schedstats);
-	else
+	} else {
 		static_branch_disable(&sched_schedstats);
+	}
 }
 
 void force_schedstat_enabled(void)
 {
 	if (!schedstat_enabled()) {
+		compute_skid();
 		pr_info("kernel profiling enabled schedstats, disable via kernel.sched_schedstats.\n");
 		static_branch_enable(&sched_schedstats);
 	}
diff --git a/kernel/sched/fair.c b/kernel/sched/fair.c
index 1efd9c4..73e9873 100644
--- a/kernel/sched/fair.c
+++ b/kernel/sched/fair.c
@@ -3748,29 +3748,35 @@ static inline bool steal_enabled(void)
 static void overload_clear(struct rq *rq)
 {
 	struct sparsemask *overload_cpus;
+	unsigned long time;
 
 	if (!steal_enabled())
 		return;
 
+	time = schedstat_start_time();
 	rcu_read_lock();
 	overload_cpus = rcu_dereference(rq->cfs_overload_cpus);
 	if (overload_cpus)
 		sparsemask_clear_elem(overload_cpus, rq->cpu);
 	rcu_read_unlock();
+	schedstat_end_time(rq->find_time, time);
 }
 
 static void overload_set(struct rq *rq)
 {
 	struct sparsemask *overload_cpus;
+	unsigned long time;
 
 	if (!steal_enabled())
 		return;
 
+	time = schedstat_start_time();
 	rcu_read_lock();
 	overload_cpus = rcu_dereference(rq->cfs_overload_cpus);
 	if (overload_cpus)
 		sparsemask_set_elem(overload_cpus, rq->cpu);
 	rcu_read_unlock();
+	schedstat_end_time(rq->find_time, time);
 }
 
 static int try_steal(struct rq *this_rq, struct rq_flags *rf);
@@ -6191,6 +6197,16 @@ static int select_idle_cpu(struct task_struct *p, struct sched_domain *sd, int t
 	return cpu;
 }
 
+#define SET_STAT(STAT)							\
+	do {								\
+		if (schedstat_enabled()) {				\
+			struct rq *rq = this_rq();			\
+									\
+			if (rq)						\
+				__schedstat_inc(rq->STAT);		\
+		}							\
+	} while (0)
+
 /*
  * Try and locate an idle core/thread in the LLC cache domain.
  */
@@ -6199,14 +6215,18 @@ static int select_idle_sibling(struct task_struct *p, int prev, int target)
 	struct sched_domain *sd;
 	int i, recent_used_cpu;
 
-	if (available_idle_cpu(target))
+	if (available_idle_cpu(target)) {
+		SET_STAT(found_idle_cpu_easy);
 		return target;
+	}
 
 	/*
 	 * If the previous CPU is cache affine and idle, don't be stupid:
 	 */
-	if (prev != target && cpus_share_cache(prev, target) && available_idle_cpu(prev))
+	if (prev != target && cpus_share_cache(prev, target) && available_idle_cpu(prev)) {
+		SET_STAT(found_idle_cpu_easy);
 		return prev;
+	}
 
 	/* Check a recently used CPU as a potential idle candidate: */
 	recent_used_cpu = p->recent_used_cpu;
@@ -6219,26 +6239,36 @@ static int select_idle_sibling(struct task_struct *p, int prev, int target)
 		 * Replace recent_used_cpu with prev as it is a potential
 		 * candidate for the next wake:
 		 */
+		SET_STAT(found_idle_cpu_easy);
 		p->recent_used_cpu = prev;
 		return recent_used_cpu;
 	}
 
 	sd = rcu_dereference(per_cpu(sd_llc, target));
-	if (!sd)
+	if (!sd) {
+		SET_STAT(nofound_idle_cpu);
 		return target;
+	}
 
 	i = select_idle_core(p, sd, target);
-	if ((unsigned)i < nr_cpumask_bits)
+	if ((unsigned)i < nr_cpumask_bits) {
+		SET_STAT(found_idle_core);
 		return i;
+	}
 
 	i = select_idle_cpu(p, sd, target);
-	if ((unsigned)i < nr_cpumask_bits)
+	if ((unsigned)i < nr_cpumask_bits) {
+		SET_STAT(found_idle_cpu);
 		return i;
+	}
 
 	i = select_idle_smt(p, sd, target);
-	if ((unsigned)i < nr_cpumask_bits)
+	if ((unsigned)i < nr_cpumask_bits) {
+		SET_STAT(found_idle_cpu);
 		return i;
+	}
 
+	SET_STAT(nofound_idle_cpu);
 	return target;
 }
 
@@ -6392,6 +6422,7 @@ static int wake_cap(struct task_struct *p, int cpu, int prev_cpu)
 static int
 select_task_rq_fair(struct task_struct *p, int prev_cpu, int sd_flag, int wake_flags)
 {
+	unsigned long time = schedstat_start_time();
 	struct sched_domain *tmp, *sd = NULL;
 	int cpu = smp_processor_id();
 	int new_cpu = prev_cpu;
@@ -6440,6 +6471,7 @@ static int wake_cap(struct task_struct *p, int cpu, int prev_cpu)
 			current->recent_used_cpu = cpu;
 	}
 	rcu_read_unlock();
+	schedstat_end_time(cpu_rq(cpu)->find_time, time);
 
 	return new_cpu;
 }
@@ -6686,6 +6718,7 @@ static void check_preempt_wakeup(struct rq *rq, struct task_struct *p, int wake_
 	struct sched_entity *se;
 	struct task_struct *p;
 	int new_tasks;
+	unsigned long time;
 
 again:
 	if (!cfs_rq->nr_running)
@@ -6800,6 +6833,8 @@ static void check_preempt_wakeup(struct rq *rq, struct task_struct *p, int wake_
 idle:
 	update_misfit_status(NULL, rq);
 
+	time = schedstat_start_time();
+
 	/*
 	 * We must set idle_stamp _before_ calling try_steal() or
 	 * idle_balance(), such that we measure the duration as idle time.
@@ -6813,6 +6848,8 @@ static void check_preempt_wakeup(struct rq *rq, struct task_struct *p, int wake_
 	if (new_tasks)
 		rq_idle_stamp_clear(rq);
 
+	schedstat_end_time(rq->find_time, time);
+
 	/*
 	 * Because try_steal() and idle_balance() release (and re-acquire)
 	 * rq->lock, it is possible for any higher priority task to appear.
@@ -9886,6 +9923,7 @@ static int steal_from(struct rq *dst_rq, struct rq_flags *dst_rf, bool *locked,
 		update_rq_clock(dst_rq);
 		attach_task(dst_rq, p);
 		stolen = 1;
+		schedstat_inc(dst_rq->steal);
 	}
 	local_irq_restore(rf.flags);
 
@@ -9910,6 +9948,7 @@ static int try_steal(struct rq *dst_rq, struct rq_flags *dst_rf)
 	int dst_cpu = dst_rq->cpu;
 	bool locked = true;
 	int stolen = 0;
+	bool any_overload = false;
 	struct sparsemask *overload_cpus;
 
 	if (!steal_enabled())
@@ -9952,6 +9991,7 @@ static int try_steal(struct rq *dst_rq, struct rq_flags *dst_rf)
 			stolen = 1;
 			goto out;
 		}
+		any_overload = true;
 	}
 
 out:
@@ -9963,6 +10003,8 @@ static int try_steal(struct rq *dst_rq, struct rq_flags *dst_rf)
 	stolen |= (dst_rq->cfs.h_nr_running > 0);
 	if (dst_rq->nr_running != dst_rq->cfs.h_nr_running)
 		stolen = -1;
+	if (!stolen && any_overload)
+		schedstat_inc(dst_rq->steal_fail);
 	return stolen;
 }
 
diff --git a/kernel/sched/sched.h b/kernel/sched/sched.h
index 2a28340..f61f640 100644
--- a/kernel/sched/sched.h
+++ b/kernel/sched/sched.h
@@ -915,6 +915,15 @@ struct rq {
 	/* try_to_wake_up() stats */
 	unsigned int		ttwu_count;
 	unsigned int		ttwu_local;
+
+	/* Idle search stats */
+	unsigned int		found_idle_core;
+	unsigned int		found_idle_cpu;
+	unsigned int		found_idle_cpu_easy;
+	unsigned int		nofound_idle_cpu;
+	unsigned long		find_time;
+	unsigned int		steal;
+	unsigned int		steal_fail;
 #endif
 
 #ifdef CONFIG_SMP
diff --git a/kernel/sched/stats.c b/kernel/sched/stats.c
index 750fb3c..00b3de5 100644
--- a/kernel/sched/stats.c
+++ b/kernel/sched/stats.c
@@ -10,7 +10,7 @@
  * Bump this up when changing the output format or the meaning of an existing
  * format, so that tools can adapt (or abort)
  */
-#define SCHEDSTAT_VERSION 15
+#define SCHEDSTAT_VERSION 16
 
 static int show_schedstat(struct seq_file *seq, void *v)
 {
@@ -37,6 +37,15 @@ static int show_schedstat(struct seq_file *seq, void *v)
 		    rq->rq_cpu_time,
 		    rq->rq_sched_info.run_delay, rq->rq_sched_info.pcount);
 
+		seq_printf(seq, " %u %u %u %u %lu %u %u",
+			   rq->found_idle_cpu_easy,
+			   rq->found_idle_cpu,
+			   rq->found_idle_core,
+			   rq->nofound_idle_cpu,
+			   rq->find_time,
+			   rq->steal,
+			   rq->steal_fail);
+
 		seq_printf(seq, "\n");
 
 #ifdef CONFIG_SMP
diff --git a/kernel/sched/stats.h b/kernel/sched/stats.h
index 4904c46..63ba3c2 100644
--- a/kernel/sched/stats.h
+++ b/kernel/sched/stats.h
@@ -39,6 +39,17 @@
 #define   schedstat_set(var, val)	do { if (schedstat_enabled()) { var = (val); } } while (0)
 #define   schedstat_val(var)		(var)
 #define   schedstat_val_or_zero(var)	((schedstat_enabled()) ? (var) : 0)
+#define   schedstat_start_time()	schedstat_val_or_zero(local_clock())
+#define   schedstat_end_time(stat, time)			\
+	do {							\
+		unsigned long endtime;				\
+								\
+		if (schedstat_enabled() && (time)) {		\
+			endtime = local_clock() - (time) - schedstat_skid; \
+			schedstat_add((stat), endtime);		\
+		}						\
+	} while (0)
+extern unsigned long schedstat_skid;
 
 #else /* !CONFIG_SCHEDSTATS: */
 static inline void rq_sched_info_arrive  (struct rq *rq, unsigned long long delta) { }
@@ -53,6 +64,8 @@ static inline void rq_sched_info_depart  (struct rq *rq, unsigned long long delt
 # define   schedstat_set(var, val)	do { } while (0)
 # define   schedstat_val(var)		0
 # define   schedstat_val_or_zero(var)	0
+# define   schedstat_start_time()	0
+# define   schedstat_end_time(stat, t)	do { } while (0)
 #endif /* CONFIG_SCHEDSTATS */
 
 #ifdef CONFIG_PSI
-- 
1.8.3.1
```