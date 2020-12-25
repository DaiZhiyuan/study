<!-- TOC -->

- [Performance: Add NUMA-awareness to qspinlock](#performance-add-numa-awareness-to-qspinlock)
    - [[PATCH v13 0/6] Add NUMA-awareness to qspinlock](#patch-v13-06-add-numa-awareness-to-qspinlock)
    - [[PATCH v13 1/6] locking/qspinlock: Rename mcs lock/unlock macros and make them more generic](#patch-v13-16-lockingqspinlock-rename-mcs-lockunlock-macros-and-make-them-more-generic)
    - [[PATCH v13 2/6] locking/qspinlock: Refactor the qspinlock slow path](#patch-v13-26-lockingqspinlock-refactor-the-qspinlock-slow-path)
    - [[PATCH v13 3/6] locking/qspinlock: Introduce CNA into the slow path of qspinlock](#patch-v13-36-lockingqspinlock-introduce-cna-into-the-slow-path-of-qspinlock)
    - [[PATCH v13 4/6] locking/qspinlock: Introduce starvation avoidance into CNA](#patch-v13-46-lockingqspinlock-introduce-starvation-avoidance-into-cna)
    - [[PATCH v13 5/6] locking/qspinlock: Avoid moving certain threads between waiting queues in CNA](#patch-v13-56-lockingqspinlock-avoid-moving-certain-threads-between-waiting-queues-in-cna)
    - [[PATCH v13 6/6] locking/qspinlock: Introduce the shuffle reduction optimization into CNA](#patch-v13-66-lockingqspinlock-introduce-the-shuffle-reduction-optimization-into-cna)

<!-- /TOC -->

# Performance: Add NUMA-awareness to qspinlock

## [PATCH v13 0/6] Add NUMA-awareness to qspinlock

```patch
Change from v12:
----------------

Added a shuffle reduction optimization (SRO, last patch in the series)
in order to address the regression in unixbench.
Reported-by: kernel test robot <oliver.sang@intel.com>

I note that despite my initial experiments, a more thorough testing 
on our system did not reproduce the regression.

The rest of the series remains unchanged.

Summary
-------

Lock throughput can be increased by handing a lock to a waiter on the
same NUMA node as the lock holder, provided care is taken to avoid
starvation of waiters on other NUMA nodes. This patch introduces CNA
(compact NUMA-aware lock) as the slow path for qspinlock. It is
enabled through a configuration option (NUMA_AWARE_SPINLOCKS).

CNA is a NUMA-aware version of the MCS lock. Spinning threads are
organized in two queues, a primary queue for threads running on the same
node as the current lock holder, and a secondary queue for threads
running on other nodes. Threads store the ID of the node on which
they are running in their queue nodes. After acquiring the MCS lock and
before acquiring the spinlock, the MCS lock holder checks whether the next
waiter in the primary queue (if exists) is running on the same NUMA node.
If it is not, that waiter is detached from the main queue and moved into
the tail of the secondary queue. This way, we gradually filter the primary
queue, leaving only waiters running on the same preferred NUMA node. Note
that certain priortized waiters (e.g., in irq and nmi contexts) are
excluded from being moved to the secondary queue. We change the NUMA node
preference after a waiter at the head of the secondary queue spins for a
certain amount of time. We do that by flushing the secondary queue into
the head of the primary queue, effectively changing the preference to the
NUMA node of the waiter at the head of the secondary queue at the time of
the flush.

More details are available at https://arxiv.org/abs/1810.05600.

We have done some performance evaluation with the locktorture module
as well as with several benchmarks from the will-it-scale repo.
The following locktorture results are from an Oracle X5-4 server
(four Intel Xeon E7-8895 v3 @ 2.60GHz sockets with 18 hyperthreaded
cores each). Each number represents an average (over 25 runs) of the
total number of ops (x10^7) reported at the end of each run. The 
standard deviation is also reported in (), and in general is about 3%
from the average. The 'stock' kernel is v5.10.0-rc7,
commit ca4bbdaf1716, compiled in the default configuration. 
'CNA' is the modified kernel with NUMA_AWARE_SPINLOCKS set;
'CNA-wo-SRO' is the modified kernel with NUMA_AWARE_SPINLOCKS set 
and without the last patch in the series (the SRO optimization).
The speedup is calculated by dividing the result of the corresponding 
variant by the result achieved with 'stock'.

#thr  	 stock     CNA-wo-SRO / speedup     CNA / speedup
  1  2.707 (0.127) 2.693 (0.100) / 0.995  2.718 (0.101) / 1.004
  2  3.262 (0.075) 3.250 (0.132) / 0.996  3.246 (0.098) / 0.995
  4  4.331 (0.125) 4.804 (0.184) / 1.109  4.733 (0.143) / 1.093
  8  5.092 (0.148) 6.996 (0.206) / 1.374  7.000 (0.194) / 1.375
 16  5.865 (0.119) 8.763 (0.161) / 1.494  8.778 (0.217) / 1.497
 32  6.314 (0.098) 9.837 (0.256) / 1.558  9.720 (0.167) / 1.539
 36  6.434 (0.101) 9.929 (0.259) / 1.543  9.988 (0.208) / 1.552
 72  6.342 (0.080) 10.416 (0.244) / 1.642  10.224 (0.203) / 1.612
108  6.168 (0.080) 10.490 (0.199) / 1.701  10.334 (0.173) / 1.675
142  5.895 (0.119) 10.480 (0.171) / 1.778  10.424 (0.222) / 1.768

The following tables contain throughput results (ops/us) from the same
setup for will-it-scale/open1_threads: 

#thr  	 stock     CNA-wo-SRO / speedup     CNA / speedup
  1  0.508 (0.001) 0.507 (0.001) / 0.997  0.508 (0.001) / 0.999
  2  0.755 (0.021) 0.764 (0.018) / 1.012  0.757 (0.017) / 1.002
  4  1.409 (0.027) 1.417 (0.024) / 1.006  1.387 (0.027) / 0.984
  8  1.726 (0.092) 1.657 (0.129) / 0.960  1.654 (0.135) / 0.959
 16  1.878 (0.099) 1.811 (0.100) / 0.964  1.761 (0.087) / 0.938
 32  1.012 (0.040) 1.705 (0.086) / 1.685  1.685 (0.081) / 1.666
 36  0.930 (0.088) 1.726 (0.090) / 1.855  1.727 (0.086) / 1.856
 72  0.826 (0.037) 1.645 (0.079) / 1.991  1.621 (0.076) / 1.962
108  0.845 (0.028) 1.685 (0.072) / 1.993  1.688 (0.073) / 1.997
142  0.827 (0.035) 1.712 (0.069) / 2.070  1.696 (0.064) / 2.052

and will-it-scale/lock2_threads:

#thr  	 stock     CNA-wo-SRO / speedup     CNA / speedup
  1  1.587 (0.004) 1.564 (0.003) / 0.985  1.577 (0.002) / 0.994
  2  2.802 (0.057) 2.752 (0.049) / 0.982  2.776 (0.065) / 0.991
  4  5.365 (0.352) 5.368 (0.196) / 1.001  5.348 (0.297) / 0.997
  8  4.161 (0.270) 4.001 (0.402) / 0.962  4.032 (0.389) / 0.969
 16  4.144 (0.130) 3.940 (0.159) / 0.951  3.917 (0.133) / 0.945
 32  2.444 (0.097) 3.996 (0.102) / 1.635  3.969 (0.130) / 1.624
 36  2.429 (0.070) 3.891 (0.087) / 1.602  3.894 (0.096) / 1.603
 72  1.847 (0.095) 3.929 (0.108) / 2.128  3.942 (0.094) / 2.135
108  1.903 (0.117) 3.898 (0.108) / 2.048  3.901 (0.105) / 2.050
142  1.841 (0.124) 3.929 (0.097) / 2.135  3.921 (0.105) / 2.130

Our evaluation shows that CNA also improves performance of user 
applications that have hot pthread mutexes. Those mutexes are 
blocking, and waiting threads park and unpark via the futex 
mechanism in the kernel. Given that kernel futex chains, which
are hashed by the mutex address, are each protected by a 
chain-specific spin lock, the contention on a user-mode mutex 
translates into contention on a kernel level spinlock. 

Here are the throughput results (ops/us) for the leveldb ‘readrandom’
benchmark:

#thr  	 stock     CNA-wo-SRO / speedup     CNA / speedup
  1  0.535 (0.017) 0.531 (0.022) / 0.991  0.532 (0.020) / 0.993
  2  0.845 (0.046) 0.837 (0.034) / 0.991  0.840 (0.048) / 0.994
  4  1.097 (0.133) 1.055 (0.130) / 0.962  1.127 (0.119) / 1.027
  8  1.091 (0.190) 1.109 (0.187) / 1.017  1.082 (0.213) / 0.992
 16  0.986 (0.161) 1.139 (0.145) / 1.155  1.073 (0.170) / 1.088
 32  0.739 (0.032) 1.154 (0.016) / 1.562  1.155 (0.022) / 1.564
 36  0.693 (0.022) 1.164 (0.020) / 1.680  1.147 (0.021) / 1.655
 72  0.623 (0.015) 1.136 (0.021) / 1.824  1.128 (0.021) / 1.811
108  0.610 (0.017) 1.135 (0.018) / 1.861  1.123 (0.020) / 1.842
142  0.602 (0.010) 1.118 (0.021) / 1.855  1.115 (0.022) / 1.850

Further comments are welcome and appreciated.

Alex Kogan (6):
  locking/qspinlock: Rename mcs lock/unlock macros and make them more
    generic
  locking/qspinlock: Refactor the qspinlock slow path
  locking/qspinlock: Introduce CNA into the slow path of qspinlock
  locking/qspinlock: Introduce starvation avoidance into CNA
  locking/qspinlock: Avoid moving certain threads between waiting queues
    in CNA
  locking/qspinlock: Introduce the shuffle reduction optimization into
    CNA

 .../admin-guide/kernel-parameters.txt         |  19 +
 arch/arm/include/asm/mcs_spinlock.h           |   6 +-
 arch/x86/Kconfig                              |  20 +
 arch/x86/include/asm/qspinlock.h              |   4 +
 arch/x86/kernel/alternative.c                 |   4 +
 include/asm-generic/mcs_spinlock.h            |   4 +-
 kernel/locking/mcs_spinlock.h                 |  20 +-
 kernel/locking/qspinlock.c                    |  82 +++-
 kernel/locking/qspinlock_cna.h                | 458 ++++++++++++++++++
 kernel/locking/qspinlock_paravirt.h           |   2 +-
 10 files changed, 596 insertions(+), 23 deletions(-)
 create mode 100644 kernel/locking/qspinlock_cna.h

-- 
2.24.3 (Apple Git-128)
```

## [PATCH v13 1/6] locking/qspinlock: Rename mcs lock/unlock macros and make them more generic

```patch
The mcs unlock macro (arch_mcs_lock_handoff) should accept the value to be
stored into the lock argument as another argument. This allows using the
same macro in cases where the value to be stored when passing the lock is
different from 1.

Signed-off-by: Alex Kogan <alex.kogan@oracle.com>
Reviewed-by: Steve Sistare <steven.sistare@oracle.com>
Reviewed-by: Waiman Long <longman@redhat.com>
---
 arch/arm/include/asm/mcs_spinlock.h |  6 +++---
 include/asm-generic/mcs_spinlock.h  |  4 ++--
 kernel/locking/mcs_spinlock.h       | 18 +++++++++---------
 kernel/locking/qspinlock.c          |  4 ++--
 kernel/locking/qspinlock_paravirt.h |  2 +-
 5 files changed, 17 insertions(+), 17 deletions(-)

diff --git a/arch/arm/include/asm/mcs_spinlock.h b/arch/arm/include/asm/mcs_spinlock.h
index 529d2cf4d06f..1eb4d733459c 100644
--- a/arch/arm/include/asm/mcs_spinlock.h
+++ b/arch/arm/include/asm/mcs_spinlock.h
@@ -6,7 +6,7 @@
 #include <asm/spinlock.h>
 
 /* MCS spin-locking. */
-#define arch_mcs_spin_lock_contended(lock)				\
+#define arch_mcs_spin_wait(lock)					\
 do {									\
 	/* Ensure prior stores are observed before we enter wfe. */	\
 	smp_mb();							\
@@ -14,9 +14,9 @@ do {									\
 		wfe();							\
 } while (0)								\
 
-#define arch_mcs_spin_unlock_contended(lock)				\
+#define arch_mcs_lock_handoff(lock, val)				\
 do {									\
-	smp_store_release(lock, 1);					\
+	smp_store_release((lock), (val));				\
 	dsb_sev();							\
 } while (0)
 
diff --git a/include/asm-generic/mcs_spinlock.h b/include/asm-generic/mcs_spinlock.h
index 10cd4ffc6ba2..f933d99c63e0 100644
--- a/include/asm-generic/mcs_spinlock.h
+++ b/include/asm-generic/mcs_spinlock.h
@@ -4,8 +4,8 @@
 /*
  * Architectures can define their own:
  *
- *   arch_mcs_spin_lock_contended(l)
- *   arch_mcs_spin_unlock_contended(l)
+ *   arch_mcs_spin_wait(l)
+ *   arch_mcs_lock_handoff(l, val)
  *
  * See kernel/locking/mcs_spinlock.c.
  */
diff --git a/kernel/locking/mcs_spinlock.h b/kernel/locking/mcs_spinlock.h
index 5e10153b4d3c..904ba5d0f3f4 100644
--- a/kernel/locking/mcs_spinlock.h
+++ b/kernel/locking/mcs_spinlock.h
@@ -21,7 +21,7 @@ struct mcs_spinlock {
 	int count;  /* nesting count, see qspinlock.c */
 };
 
-#ifndef arch_mcs_spin_lock_contended
+#ifndef arch_mcs_spin_wait
 /*
  * Using smp_cond_load_acquire() provides the acquire semantics
  * required so that subsequent operations happen after the
@@ -29,20 +29,20 @@ struct mcs_spinlock {
  * ARM64 would like to do spin-waiting instead of purely
  * spinning, and smp_cond_load_acquire() provides that behavior.
  */
-#define arch_mcs_spin_lock_contended(l)					\
-do {									\
-	smp_cond_load_acquire(l, VAL);					\
+#define arch_mcs_spin_wait(l)					\
+do {								\
+	smp_cond_load_acquire(l, VAL);				\
 } while (0)
 #endif
 
-#ifndef arch_mcs_spin_unlock_contended
+#ifndef arch_mcs_lock_handoff
 /*
  * smp_store_release() provides a memory barrier to ensure all
  * operations in the critical section has been completed before
  * unlocking.
  */
-#define arch_mcs_spin_unlock_contended(l)				\
-	smp_store_release((l), 1)
+#define arch_mcs_lock_handoff(l, val)				\
+	smp_store_release((l), (val))
 #endif
 
 /*
@@ -91,7 +91,7 @@ void mcs_spin_lock(struct mcs_spinlock **lock, struct mcs_spinlock *node)
 	WRITE_ONCE(prev->next, node);
 
 	/* Wait until the lock holder passes the lock down. */
-	arch_mcs_spin_lock_contended(&node->locked);
+	arch_mcs_spin_wait(&node->locked);
 }
 
 /*
@@ -115,7 +115,7 @@ void mcs_spin_unlock(struct mcs_spinlock **lock, struct mcs_spinlock *node)
 	}
 
 	/* Pass lock to next waiter. */
-	arch_mcs_spin_unlock_contended(&next->locked);
+	arch_mcs_lock_handoff(&next->locked, 1);
 }
 
 #endif /* __LINUX_MCS_SPINLOCK_H */
diff --git a/kernel/locking/qspinlock.c b/kernel/locking/qspinlock.c
index cbff6ba53d56..435d696f9250 100644
--- a/kernel/locking/qspinlock.c
+++ b/kernel/locking/qspinlock.c
@@ -471,7 +471,7 @@ void queued_spin_lock_slowpath(struct qspinlock *lock, u32 val)
 		WRITE_ONCE(prev->next, node);
 
 		pv_wait_node(node, prev);
-		arch_mcs_spin_lock_contended(&node->locked);
+		arch_mcs_spin_wait(&node->locked);
 
 		/*
 		 * While waiting for the MCS lock, the next pointer may have
@@ -550,7 +550,7 @@ void queued_spin_lock_slowpath(struct qspinlock *lock, u32 val)
 	if (!next)
 		next = smp_cond_load_relaxed(&node->next, (VAL));
 
-	arch_mcs_spin_unlock_contended(&next->locked);
+	arch_mcs_lock_handoff(&next->locked, 1);
 	pv_kick_node(lock, next);
 
 release:
diff --git a/kernel/locking/qspinlock_paravirt.h b/kernel/locking/qspinlock_paravirt.h
index e84d21aa0722..619d80fd5ea8 100644
--- a/kernel/locking/qspinlock_paravirt.h
+++ b/kernel/locking/qspinlock_paravirt.h
@@ -368,7 +368,7 @@ static void pv_kick_node(struct qspinlock *lock, struct mcs_spinlock *node)
 	 *
 	 * Matches with smp_store_mb() and cmpxchg() in pv_wait_node()
 	 *
-	 * The write to next->locked in arch_mcs_spin_unlock_contended()
+	 * The write to next->locked in arch_mcs_lock_handoff()
 	 * must be ordered before the read of pn->state in the cmpxchg()
 	 * below for the code to work correctly. To guarantee full ordering
 	 * irrespective of the success or failure of the cmpxchg(),
-- 
2.24.3 (Apple Git-128)
```

## [PATCH v13 2/6] locking/qspinlock: Refactor the qspinlock slow path

```patch
Move some of the code manipulating the spin lock into separate functions.
This would allow easier integration of alternative ways to manipulate
that lock.

Signed-off-by: Alex Kogan <alex.kogan@oracle.com>
Reviewed-by: Steve Sistare <steven.sistare@oracle.com>
Reviewed-by: Waiman Long <longman@redhat.com>
---
 kernel/locking/qspinlock.c | 38 ++++++++++++++++++++++++++++++++++++--
 1 file changed, 36 insertions(+), 2 deletions(-)

diff --git a/kernel/locking/qspinlock.c b/kernel/locking/qspinlock.c
index 435d696f9250..e3518709ffdc 100644
--- a/kernel/locking/qspinlock.c
+++ b/kernel/locking/qspinlock.c
@@ -289,6 +289,34 @@ static __always_inline u32  __pv_wait_head_or_lock(struct qspinlock *lock,
 #define queued_spin_lock_slowpath	native_queued_spin_lock_slowpath
 #endif
 
+/*
+ * __try_clear_tail - try to clear tail by setting the lock value to
+ * _Q_LOCKED_VAL.
+ * @lock: Pointer to the queued spinlock structure
+ * @val: Current value of the lock
+ * @node: Pointer to the MCS node of the lock holder
+ */
+static __always_inline bool __try_clear_tail(struct qspinlock *lock,
+					     u32 val,
+					     struct mcs_spinlock *node)
+{
+	return atomic_try_cmpxchg_relaxed(&lock->val, &val, _Q_LOCKED_VAL);
+}
+
+/*
+ * __mcs_lock_handoff - pass the MCS lock to the next waiter
+ * @node: Pointer to the MCS node of the lock holder
+ * @next: Pointer to the MCS node of the first waiter in the MCS queue
+ */
+static __always_inline void __mcs_lock_handoff(struct mcs_spinlock *node,
+					       struct mcs_spinlock *next)
+{
+	arch_mcs_lock_handoff(&next->locked, 1);
+}
+
+#define try_clear_tail		__try_clear_tail
+#define mcs_lock_handoff	__mcs_lock_handoff
+
 #endif /* _GEN_PV_LOCK_SLOWPATH */
 
 /**
@@ -533,7 +561,7 @@ void queued_spin_lock_slowpath(struct qspinlock *lock, u32 val)
 	 *       PENDING will make the uncontended transition fail.
 	 */
 	if ((val & _Q_TAIL_MASK) == tail) {
-		if (atomic_try_cmpxchg_relaxed(&lock->val, &val, _Q_LOCKED_VAL))
+		if (try_clear_tail(lock, val, node))
 			goto release; /* No contention */
 	}
 
@@ -550,7 +578,7 @@ void queued_spin_lock_slowpath(struct qspinlock *lock, u32 val)
 	if (!next)
 		next = smp_cond_load_relaxed(&node->next, (VAL));
 
-	arch_mcs_lock_handoff(&next->locked, 1);
+	mcs_lock_handoff(node, next);
 	pv_kick_node(lock, next);
 
 release:
@@ -575,6 +603,12 @@ EXPORT_SYMBOL(queued_spin_lock_slowpath);
 #undef pv_kick_node
 #undef pv_wait_head_or_lock
 
+#undef try_clear_tail
+#define try_clear_tail		__try_clear_tail
+
+#undef mcs_lock_handoff
+#define mcs_lock_handoff			__mcs_lock_handoff
+
 #undef  queued_spin_lock_slowpath
 #define queued_spin_lock_slowpath	__pv_queued_spin_lock_slowpath
 
-- 
2.24.3 (Apple Git-128)
```

## [PATCH v13 3/6] locking/qspinlock: Introduce CNA into the slow path of qspinlock

```patch
In CNA, spinning threads are organized in two queues, a primary queue for
threads running on the same node as the current lock holder, and a
secondary queue for threads running on other nodes. After acquiring the
MCS lock and before acquiring the spinlock, the MCS lock
holder checks whether the next waiter in the primary queue (if exists) is
running on the same NUMA node. If it is not, that waiter is detached from
the main queue and moved into the tail of the secondary queue. This way,
we gradually filter the primary queue, leaving only waiters running on
the same preferred NUMA node. For more details, see
https://arxiv.org/abs/1810.05600.

Note that this variant of CNA may introduce starvation by continuously
passing the lock between waiters in the main queue. This issue will be
addressed later in the series.

Enabling CNA is controlled via a new configuration option
(NUMA_AWARE_SPINLOCKS). By default, the CNA variant is patched in at the
boot time only if we run on a multi-node machine in native environment and
the new config is enabled. (For the time being, the patching requires
CONFIG_PARAVIRT_SPINLOCKS to be enabled as well. However, this should be
resolved once static_call() is available.) This default behavior can be
overridden with the new kernel boot command-line option
"numa_spinlock=on/off" (default is "auto").

Signed-off-by: Alex Kogan <alex.kogan@oracle.com>
Reviewed-by: Steve Sistare <steven.sistare@oracle.com>
Reviewed-by: Waiman Long <longman@redhat.com>
---
 .../admin-guide/kernel-parameters.txt         |  10 +
 arch/x86/Kconfig                              |  20 ++
 arch/x86/include/asm/qspinlock.h              |   4 +
 arch/x86/kernel/alternative.c                 |   4 +
 kernel/locking/mcs_spinlock.h                 |   2 +-
 kernel/locking/qspinlock.c                    |  42 ++-
 kernel/locking/qspinlock_cna.h                | 336 ++++++++++++++++++
 7 files changed, 413 insertions(+), 5 deletions(-)
 create mode 100644 kernel/locking/qspinlock_cna.h

diff --git a/Documentation/admin-guide/kernel-parameters.txt b/Documentation/admin-guide/kernel-parameters.txt
index 44fde25bb221..a6ae826c6076 100644
--- a/Documentation/admin-guide/kernel-parameters.txt
+++ b/Documentation/admin-guide/kernel-parameters.txt
@@ -3430,6 +3430,16 @@
 	numa_balancing=	[KNL,X86] Enable or disable automatic NUMA balancing.
 			Allowed values are enable and disable
 
+	numa_spinlock=	[NUMA, PV_OPS] Select the NUMA-aware variant
+			of spinlock. The options are:
+			auto - Enable this variant if running on a multi-node
+			machine in native environment.
+			on  - Unconditionally enable this variant.
+			off - Unconditionally disable this variant.
+
+			Not specifying this option is equivalent to
+			numa_spinlock=auto.
+
 	numa_zonelist_order= [KNL, BOOT] Select zonelist order for NUMA.
 			'node', 'default' can be specified
 			This can be set from sysctl after boot.
diff --git a/arch/x86/Kconfig b/arch/x86/Kconfig
index fbf26e0f7a6a..8c5ecbe5bcd6 100644
--- a/arch/x86/Kconfig
+++ b/arch/x86/Kconfig
@@ -1565,6 +1565,26 @@ config NUMA
 
 	  Otherwise, you should say N.
 
+config NUMA_AWARE_SPINLOCKS
+	bool "Numa-aware spinlocks"
+	depends on NUMA
+	depends on QUEUED_SPINLOCKS
+	depends on 64BIT
+	# For now, we depend on PARAVIRT_SPINLOCKS to make the patching work.
+	# This is awkward, but hopefully would be resolved once static_call()
+	# is available.
+	depends on PARAVIRT_SPINLOCKS
+	default y
+	help
+	  Introduce NUMA (Non Uniform Memory Access) awareness into
+	  the slow path of spinlocks.
+
+	  In this variant of qspinlock, the kernel will try to keep the lock
+	  on the same node, thus reducing the number of remote cache misses,
+	  while trading some of the short term fairness for better performance.
+
+	  Say N if you want absolute first come first serve fairness.
+
 config AMD_NUMA
 	def_bool y
 	prompt "Old style AMD Opteron NUMA detection"
diff --git a/arch/x86/include/asm/qspinlock.h b/arch/x86/include/asm/qspinlock.h
index d86ab942219c..21d09e8db979 100644
--- a/arch/x86/include/asm/qspinlock.h
+++ b/arch/x86/include/asm/qspinlock.h
@@ -27,6 +27,10 @@ static __always_inline u32 queued_fetch_set_pending_acquire(struct qspinlock *lo
 	return val;
 }
 
+#ifdef CONFIG_NUMA_AWARE_SPINLOCKS
+extern void cna_configure_spin_lock_slowpath(void);
+#endif
+
 #ifdef CONFIG_PARAVIRT_SPINLOCKS
 extern void native_queued_spin_lock_slowpath(struct qspinlock *lock, u32 val);
 extern void __pv_init_lock_hash(void);
diff --git a/arch/x86/kernel/alternative.c b/arch/x86/kernel/alternative.c
index 2400ad62f330..e04f48c2191d 100644
--- a/arch/x86/kernel/alternative.c
+++ b/arch/x86/kernel/alternative.c
@@ -741,6 +741,10 @@ void __init alternative_instructions(void)
 	}
 #endif
 
+#if defined(CONFIG_NUMA_AWARE_SPINLOCKS)
+	cna_configure_spin_lock_slowpath();
+#endif
+
 	apply_paravirt(__parainstructions, __parainstructions_end);
 
 	restart_nmi();
diff --git a/kernel/locking/mcs_spinlock.h b/kernel/locking/mcs_spinlock.h
index 904ba5d0f3f4..5e47ffb3f08b 100644
--- a/kernel/locking/mcs_spinlock.h
+++ b/kernel/locking/mcs_spinlock.h
@@ -17,7 +17,7 @@
 
 struct mcs_spinlock {
 	struct mcs_spinlock *next;
-	int locked; /* 1 if lock acquired */
+	unsigned int locked; /* 1 if lock acquired */
 	int count;  /* nesting count, see qspinlock.c */
 };
 
diff --git a/kernel/locking/qspinlock.c b/kernel/locking/qspinlock.c
index e3518709ffdc..8c1a21b53913 100644
--- a/kernel/locking/qspinlock.c
+++ b/kernel/locking/qspinlock.c
@@ -11,7 +11,7 @@
  *          Peter Zijlstra <peterz@infradead.org>
  */
 
-#ifndef _GEN_PV_LOCK_SLOWPATH
+#if !defined(_GEN_PV_LOCK_SLOWPATH) && !defined(_GEN_CNA_LOCK_SLOWPATH)
 
 #include <linux/smp.h>
 #include <linux/bug.h>
@@ -71,7 +71,8 @@
 /*
  * On 64-bit architectures, the mcs_spinlock structure will be 16 bytes in
  * size and four of them will fit nicely in one 64-byte cacheline. For
- * pvqspinlock, however, we need more space for extra data. To accommodate
+ * pvqspinlock, however, we need more space for extra data. The same also
+ * applies for the NUMA-aware variant of spinlocks (CNA). To accommodate
  * that, we insert two more long words to pad it up to 32 bytes. IOW, only
  * two of them can fit in a cacheline in this case. That is OK as it is rare
  * to have more than 2 levels of slowpath nesting in actual use. We don't
@@ -80,7 +81,7 @@
  */
 struct qnode {
 	struct mcs_spinlock mcs;
-#ifdef CONFIG_PARAVIRT_SPINLOCKS
+#if defined(CONFIG_PARAVIRT_SPINLOCKS) || defined(CONFIG_NUMA_AWARE_SPINLOCKS)
 	long reserved[2];
 #endif
 };
@@ -104,6 +105,8 @@ struct qnode {
  * Exactly fits one 64-byte cacheline on a 64-bit architecture.
  *
  * PV doubles the storage and uses the second cacheline for PV state.
+ * CNA also doubles the storage and uses the second cacheline for
+ * CNA-specific state.
  */
 static DEFINE_PER_CPU_ALIGNED(struct qnode, qnodes[MAX_NODES]);
 
@@ -317,7 +320,7 @@ static __always_inline void __mcs_lock_handoff(struct mcs_spinlock *node,
 #define try_clear_tail		__try_clear_tail
 #define mcs_lock_handoff	__mcs_lock_handoff
 
-#endif /* _GEN_PV_LOCK_SLOWPATH */
+#endif /* _GEN_PV_LOCK_SLOWPATH && _GEN_CNA_LOCK_SLOWPATH */
 
 /**
  * queued_spin_lock_slowpath - acquire the queued spinlock
@@ -589,6 +592,37 @@ void queued_spin_lock_slowpath(struct qspinlock *lock, u32 val)
 }
 EXPORT_SYMBOL(queued_spin_lock_slowpath);
 
+/*
+ * Generate the code for NUMA-aware spinlocks
+ */
+#if !defined(_GEN_CNA_LOCK_SLOWPATH) && defined(CONFIG_NUMA_AWARE_SPINLOCKS)
+#define _GEN_CNA_LOCK_SLOWPATH
+
+#undef pv_init_node
+#define pv_init_node			cna_init_node
+
+#undef pv_wait_head_or_lock
+#define pv_wait_head_or_lock		cna_wait_head_or_lock
+
+#undef try_clear_tail
+#define try_clear_tail			cna_try_clear_tail
+
+#undef mcs_lock_handoff
+#define mcs_lock_handoff			cna_lock_handoff
+
+#undef  queued_spin_lock_slowpath
+/*
+ * defer defining queued_spin_lock_slowpath until after the include to
+ * avoid a name clash with the identically named field in pv_ops.lock
+ * (see cna_configure_spin_lock_slowpath())
+ */
+#include "qspinlock_cna.h"
+#define queued_spin_lock_slowpath	__cna_queued_spin_lock_slowpath
+
+#include "qspinlock.c"
+
+#endif
+
 /*
  * Generate the paravirt code for queued_spin_unlock_slowpath().
  */
diff --git a/kernel/locking/qspinlock_cna.h b/kernel/locking/qspinlock_cna.h
new file mode 100644
index 000000000000..590402ad69ef
--- /dev/null
+++ b/kernel/locking/qspinlock_cna.h
@@ -0,0 +1,336 @@
+/* SPDX-License-Identifier: GPL-2.0 */
+#ifndef _GEN_CNA_LOCK_SLOWPATH
+#error "do not include this file"
+#endif
+
+#include <linux/topology.h>
+
+/*
+ * Implement a NUMA-aware version of MCS (aka CNA, or compact NUMA-aware lock).
+ *
+ * In CNA, spinning threads are organized in two queues, a primary queue for
+ * threads running on the same NUMA node as the current lock holder, and a
+ * secondary queue for threads running on other nodes. Schematically, it
+ * looks like this:
+ *
+ *    cna_node
+ *   +----------+     +--------+         +--------+
+ *   |mcs:next  | --> |mcs:next| --> ... |mcs:next| --> NULL  [Primary queue]
+ *   |mcs:locked| -.  +--------+         +--------+
+ *   +----------+  |
+ *                 `----------------------.
+ *                                        v
+ *                 +--------+         +--------+
+ *                 |mcs:next| --> ... |mcs:next|            [Secondary queue]
+ *                 +--------+         +--------+
+ *                     ^                    |
+ *                     `--------------------'
+ *
+ * N.B. locked := 1 if secondary queue is absent. Otherwise, it contains the
+ * encoded pointer to the tail of the secondary queue, which is organized as a
+ * circular list.
+ *
+ * After acquiring the MCS lock and before acquiring the spinlock, the MCS lock
+ * holder checks whether the next waiter in the primary queue (if exists) is
+ * running on the same NUMA node. If it is not, that waiter is detached from the
+ * main queue and moved into the tail of the secondary queue. This way, we
+ * gradually filter the primary queue, leaving only waiters running on the same
+ * preferred NUMA node.
+ *
+ * For more details, see https://arxiv.org/abs/1810.05600.
+ *
+ * Authors: Alex Kogan <alex.kogan@oracle.com>
+ *          Dave Dice <dave.dice@oracle.com>
+ */
+
+struct cna_node {
+	struct mcs_spinlock	mcs;
+	u16			numa_node;
+	u16			real_numa_node;
+	u32			encoded_tail;	/* self */
+	u32			partial_order;	/* enum val */
+};
+
+enum {
+	LOCAL_WAITER_FOUND,
+	LOCAL_WAITER_NOT_FOUND,
+};
+
+static void __init cna_init_nodes_per_cpu(unsigned int cpu)
+{
+	struct mcs_spinlock *base = per_cpu_ptr(&qnodes[0].mcs, cpu);
+	int numa_node = cpu_to_node(cpu);
+	int i;
+
+	for (i = 0; i < MAX_NODES; i++) {
+		struct cna_node *cn = (struct cna_node *)grab_mcs_node(base, i);
+
+		cn->real_numa_node = numa_node;
+		cn->encoded_tail = encode_tail(cpu, i);
+		/*
+		 * make sure @encoded_tail is not confused with other valid
+		 * values for @locked (0 or 1)
+		 */
+		WARN_ON(cn->encoded_tail <= 1);
+	}
+}
+
+static int __init cna_init_nodes(void)
+{
+	unsigned int cpu;
+
+	/*
+	 * this will break on 32bit architectures, so we restrict
+	 * the use of CNA to 64bit only (see arch/x86/Kconfig)
+	 */
+	BUILD_BUG_ON(sizeof(struct cna_node) > sizeof(struct qnode));
+	/* we store an ecoded tail word in the node's @locked field */
+	BUILD_BUG_ON(sizeof(u32) > sizeof(unsigned int));
+
+	for_each_possible_cpu(cpu)
+		cna_init_nodes_per_cpu(cpu);
+
+	return 0;
+}
+
+static __always_inline void cna_init_node(struct mcs_spinlock *node)
+{
+	struct cna_node *cn = (struct cna_node *)node;
+
+	cn->numa_node = cn->real_numa_node;
+}
+
+/*
+ * cna_splice_head -- splice the entire secondary queue onto the head of the
+ * primary queue.
+ *
+ * Returns the new primary head node or NULL on failure.
+ */
+static struct mcs_spinlock *
+cna_splice_head(struct qspinlock *lock, u32 val,
+		struct mcs_spinlock *node, struct mcs_spinlock *next)
+{
+	struct mcs_spinlock *head_2nd, *tail_2nd;
+	u32 new;
+
+	tail_2nd = decode_tail(node->locked);
+	head_2nd = tail_2nd->next;
+
+	if (next) {
+		/*
+		 * If the primary queue is not empty, the primary tail doesn't
+		 * need to change and we can simply link the secondary tail to
+		 * the old primary head.
+		 */
+		tail_2nd->next = next;
+	} else {
+		/*
+		 * When the primary queue is empty, the secondary tail becomes
+		 * the primary tail.
+		 */
+
+		/*
+		 * Speculatively break the secondary queue's circular link such
+		 * that when the secondary tail becomes the primary tail it all
+		 * works out.
+		 */
+		tail_2nd->next = NULL;
+
+		/*
+		 * tail_2nd->next = NULL;	old = xchg_tail(lock, tail);
+		 *				prev = decode_tail(old);
+		 * try_cmpxchg_release(...);	WRITE_ONCE(prev->next, node);
+		 *
+		 * If the following cmpxchg() succeeds, our stores will not
+		 * collide.
+		 */
+		new = ((struct cna_node *)tail_2nd)->encoded_tail |
+			_Q_LOCKED_VAL;
+		if (!atomic_try_cmpxchg_release(&lock->val, &val, new)) {
+			/* Restore the secondary queue's circular link. */
+			tail_2nd->next = head_2nd;
+			return NULL;
+		}
+	}
+
+	/* The primary queue head now is what was the secondary queue head. */
+	return head_2nd;
+}
+
+static inline bool cna_try_clear_tail(struct qspinlock *lock, u32 val,
+				      struct mcs_spinlock *node)
+{
+	/*
+	 * We're here because the primary queue is empty; check the secondary
+	 * queue for remote waiters.
+	 */
+	if (node->locked > 1) {
+		struct mcs_spinlock *next;
+
+		/*
+		 * When there are waiters on the secondary queue, try to move
+		 * them back onto the primary queue and let them rip.
+		 */
+		next = cna_splice_head(lock, val, node, NULL);
+		if (next) {
+			arch_mcs_lock_handoff(&next->locked, 1);
+			return true;
+		}
+
+		return false;
+	}
+
+	/* Both queues are empty. Do what MCS does. */
+	return __try_clear_tail(lock, val, node);
+}
+
+/*
+ * cna_splice_tail -- splice the next node from the primary queue onto
+ * the secondary queue.
+ */
+static void cna_splice_next(struct mcs_spinlock *node,
+			    struct mcs_spinlock *next,
+			    struct mcs_spinlock *nnext)
+{
+	/* remove 'next' from the main queue */
+	node->next = nnext;
+
+	/* stick `next` on the secondary queue tail */
+	if (node->locked <= 1) { /* if secondary queue is empty */
+		/* create secondary queue */
+		next->next = next;
+	} else {
+		/* add to the tail of the secondary queue */
+		struct mcs_spinlock *tail_2nd = decode_tail(node->locked);
+		struct mcs_spinlock *head_2nd = tail_2nd->next;
+
+		tail_2nd->next = next;
+		next->next = head_2nd;
+	}
+
+	node->locked = ((struct cna_node *)next)->encoded_tail;
+}
+
+/*
+ * cna_order_queue - check whether the next waiter in the main queue is on
+ * the same NUMA node as the lock holder; if not, and it has a waiter behind
+ * it in the main queue, move the former onto the secondary queue.
+ */
+static u32 cna_order_queue(struct mcs_spinlock *node)
+{
+	struct mcs_spinlock *next = READ_ONCE(node->next);
+	int numa_node, next_numa_node;
+
+	if (!next)
+		return LOCAL_WAITER_NOT_FOUND;
+
+	numa_node = ((struct cna_node *)node)->numa_node;
+	next_numa_node = ((struct cna_node *)next)->numa_node;
+
+	if (next_numa_node != numa_node) {
+		struct mcs_spinlock *nnext = READ_ONCE(next->next);
+
+		if (nnext) {
+			cna_splice_next(node, next, nnext);
+			next = nnext;
+		}
+		/*
+		 * Inherit NUMA node id of primary queue, to maintain the
+		 * preference even if the next waiter is on a different node.
+		 */
+		((struct cna_node *)next)->numa_node = numa_node;
+	}
+	return LOCAL_WAITER_FOUND;
+}
+
+/* Abuse the pv_wait_head_or_lock() hook to get some work done */
+static __always_inline u32 cna_wait_head_or_lock(struct qspinlock *lock,
+						 struct mcs_spinlock *node)
+{
+	struct cna_node *cn = (struct cna_node *)node;
+
+	/*
+	 * Try and put the time otherwise spent spin waiting on
+	 * _Q_LOCKED_PENDING_MASK to use by sorting our lists.
+	 */
+	cn->partial_order = cna_order_queue(node);
+
+	return 0; /* we lied; we didn't wait, go do so now */
+}
+
+static inline void cna_lock_handoff(struct mcs_spinlock *node,
+				 struct mcs_spinlock *next)
+{
+	struct cna_node *cn = (struct cna_node *)node;
+	u32 val = 1;
+
+	u32 partial_order = cn->partial_order;
+
+	if (partial_order == LOCAL_WAITER_NOT_FOUND)
+		partial_order = cna_order_queue(node);
+
+	/*
+	 * At this point, we must have a successor in the main queue, so if we call
+	 * cna_order_queue() above, we will find a local waiter, either real or
+	 * fake one.
+	 */
+	WARN_ON(partial_order == LOCAL_WAITER_NOT_FOUND);
+
+	/*
+	 * We found a local waiter; reload @next in case it was changed by
+	 * cna_order_queue().
+	 */
+	next = node->next;
+	if (node->locked > 1)
+		val = node->locked;	/* preseve secondary queue */
+
+	arch_mcs_lock_handoff(&next->locked, val);
+}
+
+/*
+ * Constant (boot-param configurable) flag selecting the NUMA-aware variant
+ * of spinlock.  Possible values: -1 (off) / 0 (auto, default) / 1 (on).
+ */
+static int numa_spinlock_flag;
+
+static int __init numa_spinlock_setup(char *str)
+{
+	if (!strcmp(str, "auto")) {
+		numa_spinlock_flag = 0;
+		return 1;
+	} else if (!strcmp(str, "on")) {
+		numa_spinlock_flag = 1;
+		return 1;
+	} else if (!strcmp(str, "off")) {
+		numa_spinlock_flag = -1;
+		return 1;
+	}
+
+	return 0;
+}
+__setup("numa_spinlock=", numa_spinlock_setup);
+
+void __cna_queued_spin_lock_slowpath(struct qspinlock *lock, u32 val);
+
+/*
+ * Switch to the NUMA-friendly slow path for spinlocks when we have
+ * multiple NUMA nodes in native environment, unless the user has
+ * overridden this default behavior by setting the numa_spinlock flag.
+ */
+void __init cna_configure_spin_lock_slowpath(void)
+{
+
+	if (numa_spinlock_flag < 0)
+		return;
+
+	if (numa_spinlock_flag == 0 && (nr_node_ids < 2 ||
+		    pv_ops.lock.queued_spin_lock_slowpath !=
+			native_queued_spin_lock_slowpath))
+		return;
+
+	cna_init_nodes();
+
+	pv_ops.lock.queued_spin_lock_slowpath = __cna_queued_spin_lock_slowpath;
+
+	pr_info("Enabling CNA spinlock\n");
+}
-- 
2.24.3 (Apple Git-128)
```

## [PATCH v13 4/6] locking/qspinlock: Introduce starvation avoidance into CNA
```patch
Keep track of the time the thread at the head of the secondary queue
has been waiting, and force inter-node handoff once this time passes
a preset threshold. The default value for the threshold (10ms) can be
overridden with the new kernel boot command-line option
"numa_spinlock_threshold". The ms value is translated internally to the
nearest rounded-up jiffies.

Signed-off-by: Alex Kogan <alex.kogan@oracle.com>
Reviewed-by: Steve Sistare <steven.sistare@oracle.com>
Reviewed-by: Waiman Long <longman@redhat.com>
---
 .../admin-guide/kernel-parameters.txt         |  9 ++
 kernel/locking/qspinlock_cna.h                | 95 ++++++++++++++++---
 2 files changed, 92 insertions(+), 12 deletions(-)

diff --git a/Documentation/admin-guide/kernel-parameters.txt b/Documentation/admin-guide/kernel-parameters.txt
index a6ae826c6076..fffd31089db0 100644
--- a/Documentation/admin-guide/kernel-parameters.txt
+++ b/Documentation/admin-guide/kernel-parameters.txt
@@ -3440,6 +3440,15 @@
 			Not specifying this option is equivalent to
 			numa_spinlock=auto.
 
+	numa_spinlock_threshold=	[NUMA, PV_OPS]
+			Set the time threshold in milliseconds for the
+			number of intra-node lock hand-offs before the
+			NUMA-aware spinlock is forced to be passed to
+			a thread on another NUMA node.	Valid values
+			are in the [1..100] range. Smaller values result
+			in a more fair, but less performant spinlock,
+			and vice versa. The default value is 10.
+
 	numa_zonelist_order= [KNL, BOOT] Select zonelist order for NUMA.
 			'node', 'default' can be specified
 			This can be set from sysctl after boot.
diff --git a/kernel/locking/qspinlock_cna.h b/kernel/locking/qspinlock_cna.h
index 590402ad69ef..d3e27549c769 100644
--- a/kernel/locking/qspinlock_cna.h
+++ b/kernel/locking/qspinlock_cna.h
@@ -37,6 +37,12 @@
  * gradually filter the primary queue, leaving only waiters running on the same
  * preferred NUMA node.
  *
+ * We change the NUMA node preference after a waiter at the head of the
+ * secondary queue spins for a certain amount of time (10ms, by default).
+ * We do that by flushing the secondary queue into the head of the primary queue,
+ * effectively changing the preference to the NUMA node of the waiter at the head
+ * of the secondary queue at the time of the flush.
+ *
  * For more details, see https://arxiv.org/abs/1810.05600.
  *
  * Authors: Alex Kogan <alex.kogan@oracle.com>
@@ -49,13 +55,33 @@ struct cna_node {
 	u16			real_numa_node;
 	u32			encoded_tail;	/* self */
 	u32			partial_order;	/* enum val */
+	s32			start_time;
 };
 
 enum {
 	LOCAL_WAITER_FOUND,
 	LOCAL_WAITER_NOT_FOUND,
+	FLUSH_SECONDARY_QUEUE
 };
 
+/*
+ * Controls the threshold time in ms (default = 10) for intra-node lock
+ * hand-offs before the NUMA-aware variant of spinlock is forced to be
+ * passed to a thread on another NUMA node. The default setting can be
+ * changed with the "numa_spinlock_threshold" boot option.
+ */
+#define MSECS_TO_JIFFIES(m)	\
+	(((m) + (MSEC_PER_SEC / HZ) - 1) / (MSEC_PER_SEC / HZ))
+static int intra_node_handoff_threshold __ro_after_init = MSECS_TO_JIFFIES(10);
+
+static inline bool intra_node_threshold_reached(struct cna_node *cn)
+{
+	s32 current_time = (s32)jiffies;
+	s32 threshold = cn->start_time + intra_node_handoff_threshold;
+
+	return current_time - threshold > 0;
+}
+
 static void __init cna_init_nodes_per_cpu(unsigned int cpu)
 {
 	struct mcs_spinlock *base = per_cpu_ptr(&qnodes[0].mcs, cpu);
@@ -98,6 +124,7 @@ static __always_inline void cna_init_node(struct mcs_spinlock *node)
 	struct cna_node *cn = (struct cna_node *)node;
 
 	cn->numa_node = cn->real_numa_node;
+	cn->start_time = 0;
 }
 
 /*
@@ -197,8 +224,15 @@ static void cna_splice_next(struct mcs_spinlock *node,
 
 	/* stick `next` on the secondary queue tail */
 	if (node->locked <= 1) { /* if secondary queue is empty */
+		struct cna_node *cn = (struct cna_node *)node;
+
 		/* create secondary queue */
 		next->next = next;
+
+		cn->start_time = (s32)jiffies;
+		/* make sure start_time != 0 iff secondary queue is not empty */
+		if (!cn->start_time)
+			cn->start_time = 1;
 	} else {
 		/* add to the tail of the secondary queue */
 		struct mcs_spinlock *tail_2nd = decode_tail(node->locked);
@@ -249,11 +283,15 @@ static __always_inline u32 cna_wait_head_or_lock(struct qspinlock *lock,
 {
 	struct cna_node *cn = (struct cna_node *)node;
 
-	/*
-	 * Try and put the time otherwise spent spin waiting on
-	 * _Q_LOCKED_PENDING_MASK to use by sorting our lists.
-	 */
-	cn->partial_order = cna_order_queue(node);
+	if (!cn->start_time || !intra_node_threshold_reached(cn)) {
+		/*
+		 * Try and put the time otherwise spent spin waiting on
+		 * _Q_LOCKED_PENDING_MASK to use by sorting our lists.
+		 */
+		cn->partial_order = cna_order_queue(node);
+	} else {
+		cn->partial_order = FLUSH_SECONDARY_QUEUE;
+	}
 
 	return 0; /* we lied; we didn't wait, go do so now */
 }
@@ -276,13 +314,29 @@ static inline void cna_lock_handoff(struct mcs_spinlock *node,
 	 */
 	WARN_ON(partial_order == LOCAL_WAITER_NOT_FOUND);
 
-	/*
-	 * We found a local waiter; reload @next in case it was changed by
-	 * cna_order_queue().
-	 */
-	next = node->next;
-	if (node->locked > 1)
-		val = node->locked;	/* preseve secondary queue */
+	if (partial_order == LOCAL_WAITER_FOUND) {
+		/*
+		 * We found a local waiter; reload @next in case it
+		 * was changed by cna_order_queue().
+		 */
+		next = node->next;
+		if (node->locked > 1) {
+			val = node->locked;     /* preseve secondary queue */
+			((struct cna_node *)next)->start_time = cn->start_time;
+		}
+	} else {
+		WARN_ON(partial_order != FLUSH_SECONDARY_QUEUE);
+		/*
+		 * We decided to flush the secondary queue;
+		 * this can only happen if that queue is not empty.
+		 */
+		WARN_ON(node->locked <= 1);
+		/*
+		 * Splice the secondary queue onto the primary queue and pass the lock
+		 * to the longest waiting remote waiter.
+		 */
+		next = cna_splice_head(NULL, 0, node, next);
+	}
 
 	arch_mcs_lock_handoff(&next->locked, val);
 }
@@ -334,3 +388,20 @@ void __init cna_configure_spin_lock_slowpath(void)
 
 	pr_info("Enabling CNA spinlock\n");
 }
+
+static int __init numa_spinlock_threshold_setup(char *str)
+{
+	int param;
+
+	if (get_option(&str, &param)) {
+		/* valid value is between 1 and 100 */
+		if (param <= 0 || param > 100)
+			return 0;
+
+		intra_node_handoff_threshold = msecs_to_jiffies(param);
+		return 1;
+	}
+
+	return 0;
+}
+__setup("numa_spinlock_threshold=", numa_spinlock_threshold_setup);
-- 
2.24.3 (Apple Git-128)
```
## [PATCH v13 5/6] locking/qspinlock: Avoid moving certain threads between waiting queues in CNA

```patch
Prohibit moving certain threads (e.g., in irq and nmi contexts)
to the secondary queue. Those prioritized threads will always stay
in the primary queue, and so will have a shorter wait time for the lock.

Signed-off-by: Alex Kogan <alex.kogan@oracle.com>
Reviewed-by: Steve Sistare <steven.sistare@oracle.com>
Reviewed-by: Waiman Long <longman@redhat.com>
---
 kernel/locking/qspinlock_cna.h | 26 ++++++++++++++++++++------
 1 file changed, 20 insertions(+), 6 deletions(-)

diff --git a/kernel/locking/qspinlock_cna.h b/kernel/locking/qspinlock_cna.h
index d3e27549c769..ac3109ab0a84 100644
--- a/kernel/locking/qspinlock_cna.h
+++ b/kernel/locking/qspinlock_cna.h
@@ -4,6 +4,7 @@
 #endif
 
 #include <linux/topology.h>
+#include <linux/sched/rt.h>
 
 /*
  * Implement a NUMA-aware version of MCS (aka CNA, or compact NUMA-aware lock).
@@ -35,7 +36,8 @@
  * running on the same NUMA node. If it is not, that waiter is detached from the
  * main queue and moved into the tail of the secondary queue. This way, we
  * gradually filter the primary queue, leaving only waiters running on the same
- * preferred NUMA node.
+ * preferred NUMA node. Note that certain priortized waiters (e.g., in
+ * irq and nmi contexts) are excluded from being moved to the secondary queue.
  *
  * We change the NUMA node preference after a waiter at the head of the
  * secondary queue spins for a certain amount of time (10ms, by default).
@@ -49,6 +51,8 @@
  *          Dave Dice <dave.dice@oracle.com>
  */
 
+#define CNA_PRIORITY_NODE      0xffff
+
 struct cna_node {
 	struct mcs_spinlock	mcs;
 	u16			numa_node;
@@ -121,9 +125,10 @@ static int __init cna_init_nodes(void)
 
 static __always_inline void cna_init_node(struct mcs_spinlock *node)
 {
+	bool priority = !in_task() || irqs_disabled() || rt_task(current);
 	struct cna_node *cn = (struct cna_node *)node;
 
-	cn->numa_node = cn->real_numa_node;
+	cn->numa_node = priority ? CNA_PRIORITY_NODE : cn->real_numa_node;
 	cn->start_time = 0;
 }
 
@@ -262,11 +267,13 @@ static u32 cna_order_queue(struct mcs_spinlock *node)
 	next_numa_node = ((struct cna_node *)next)->numa_node;
 
 	if (next_numa_node != numa_node) {
-		struct mcs_spinlock *nnext = READ_ONCE(next->next);
+		if (next_numa_node != CNA_PRIORITY_NODE) {
+			struct mcs_spinlock *nnext = READ_ONCE(next->next);
 
-		if (nnext) {
-			cna_splice_next(node, next, nnext);
-			next = nnext;
+			if (nnext) {
+				cna_splice_next(node, next, nnext);
+				next = nnext;
+			}
 		}
 		/*
 		 * Inherit NUMA node id of primary queue, to maintain the
@@ -284,6 +291,13 @@ static __always_inline u32 cna_wait_head_or_lock(struct qspinlock *lock,
 	struct cna_node *cn = (struct cna_node *)node;
 
 	if (!cn->start_time || !intra_node_threshold_reached(cn)) {
+		/*
+		 * We are at the head of the wait queue, no need to use
+		 * the fake NUMA node ID.
+		 */
+		if (cn->numa_node == CNA_PRIORITY_NODE)
+			cn->numa_node = cn->real_numa_node;
+
 		/*
 		 * Try and put the time otherwise spent spin waiting on
 		 * _Q_LOCKED_PENDING_MASK to use by sorting our lists.
-- 
2.24.3 (Apple Git-128)
```

## [PATCH v13 6/6] locking/qspinlock: Introduce the shuffle reduction optimization into CNA

```patch
This performance optimization chooses probabilistically to avoid moving
threads from the main queue into the secondary one when the secondary queue
is empty.

It is helpful when the lock is only lightly contended. In particular, it
makes CNA less eager to create a secondary queue, but does not introduce
any extra delays for threads waiting in that queue once it is created.

Signed-off-by: Alex Kogan <alex.kogan@oracle.com>
Reviewed-by: Steve Sistare <steven.sistare@oracle.com>
Reviewed-by: Waiman Long <longman@redhat.com>
---
 kernel/locking/qspinlock_cna.h | 39 +++++++++++++++++++++++++++++++++-
 1 file changed, 38 insertions(+), 1 deletion(-)

diff --git a/kernel/locking/qspinlock_cna.h b/kernel/locking/qspinlock_cna.h
index ac3109ab0a84..621399242735 100644
--- a/kernel/locking/qspinlock_cna.h
+++ b/kernel/locking/qspinlock_cna.h
@@ -5,6 +5,7 @@
 
 #include <linux/topology.h>
 #include <linux/sched/rt.h>
+#include <linux/random.h>
 
 /*
  * Implement a NUMA-aware version of MCS (aka CNA, or compact NUMA-aware lock).
@@ -86,6 +87,34 @@ static inline bool intra_node_threshold_reached(struct cna_node *cn)
 	return current_time - threshold > 0;
 }
 
+/*
+ * Controls the probability for enabling the ordering of the main queue
+ * when the secondary queue is empty. The chosen value reduces the amount
+ * of unnecessary shuffling of threads between the two waiting queues
+ * when the contention is low, while responding fast enough and enabling
+ * the shuffling when the contention is high.
+ */
+#define SHUFFLE_REDUCTION_PROB_ARG  (7)
+
+/* Per-CPU pseudo-random number seed */
+static DEFINE_PER_CPU(u32, seed);
+
+/*
+ * Return false with probability 1 / 2^@num_bits.
+ * Intuitively, the larger @num_bits the less likely false is to be returned.
+ * @num_bits must be a number between 0 and 31.
+ */
+static bool probably(unsigned int num_bits)
+{
+	u32 s;
+
+	s = this_cpu_read(seed);
+	s = next_pseudo_random32(s);
+	this_cpu_write(seed, s);
+
+	return s & ((1 << num_bits) - 1);
+}
+
 static void __init cna_init_nodes_per_cpu(unsigned int cpu)
 {
 	struct mcs_spinlock *base = per_cpu_ptr(&qnodes[0].mcs, cpu);
@@ -290,7 +319,15 @@ static __always_inline u32 cna_wait_head_or_lock(struct qspinlock *lock,
 {
 	struct cna_node *cn = (struct cna_node *)node;
 
-	if (!cn->start_time || !intra_node_threshold_reached(cn)) {
+	if (node->locked <= 1 && probably(SHUFFLE_REDUCTION_PROB_ARG)) {
+		/*
+		 * When the secondary queue is empty, skip the call to
+		 * cna_order_queue() with high probability. This optimization
+		 * reduces the overhead of unnecessary shuffling of threads
+		 * between waiting queues when the lock is only lightly contended.
+		 */
+		cn->partial_order = LOCAL_WAITER_FOUND;
+	} else if (!cn->start_time || !intra_node_threshold_reached(cn)) {
 		/*
 		 * We are at the head of the wait queue, no need to use
 		 * the fake NUMA node ID.
-- 
2.24.3 (Apple Git-128)
```