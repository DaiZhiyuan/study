<!-- TOC -->

- [Performance: Rework REFCOUNT_FULL using atomic_fetch_* operations](#performance-rework-refcount_full-using-atomic_fetch_-operations)
    - [[RESEND PATCH v4 00/10] Rework REFCOUNT_FULL using atomic_fetch_* operations](#resend-patch-v4-0010-rework-refcount_full-using-atomic_fetch_-operations)
    - [[RESEND PATCH v4 01/10] lib/refcount: Define constants for saturation and max refcount values](#resend-patch-v4-0110-librefcount-define-constants-for-saturation-and-max-refcount-values)
    - [[RESEND PATCH v4 02/10] lib/refcount: Ensure integer operands are treated as signed](#resend-patch-v4-0210-librefcount-ensure-integer-operands-are-treated-as-signed)
    - [[RESEND PATCH v4 03/10] lib/refcount: Remove unused refcount_*_checked() variants](#resend-patch-v4-0310-librefcount-remove-unused-refcount__checked-variants)
    - [[RESEND PATCH v4 04/10] lib/refcount: Move bulk of REFCOUNT_FULL implementation into header](#resend-patch-v4-0410-librefcount-move-bulk-of-refcount_full-implementation-into-header)
    - [[RESEND PATCH v4 05/10] lib/refcount: Improve performance of generic REFCOUNT_FULL code](#resend-patch-v4-0510-librefcount-improve-performance-of-generic-refcount_full-code)
    - [[RESEND PATCH v4 06/10] lib/refcount: Move saturation warnings out of line](#resend-patch-v4-0610-librefcount-move-saturation-warnings-out-of-line)
    - [[RESEND PATCH v4 07/10] lib/refcount: Consolidate REFCOUNT_{MAX,SATURATED} definitions](#resend-patch-v4-0710-librefcount-consolidate-refcount_maxsaturated-definitions)
    - [[RESEND PATCH v4 08/10] refcount: Consolidate implementations of refcount_t](#resend-patch-v4-0810-refcount-consolidate-implementations-of-refcount_t)
    - [[RESEND PATCH v4 09/10] lib/refcount: Remove unused 'refcount_error_report()' function](#resend-patch-v4-0910-librefcount-remove-unused-refcount_error_report-function)
    - [[RESEND PATCH v4 10/10] drivers/lkdtm: Remove references to CONFIG_REFCOUNT_FULL](#resend-patch-v4-1010-driverslkdtm-remove-references-to-config_refcount_full)

<!-- /TOC -->

# Performance: Rework REFCOUNT_FULL using atomic_fetch_* operations
## [RESEND PATCH v4 00/10] Rework REFCOUNT_FULL using atomic_fetch_* operations

```patch
Hi everybody,

This is a resend of version four of the patches I posted here:

  v4: https://lore.kernel.org/lkml/20191030143035.19440-1-will@kernel.org

Previous versions can be found at:

  v1: https://lkml.kernel.org/r/20190802101000.12958-1-will@kernel.org
  v2: https://lkml.kernel.org/r/20190827163204.29903-1-will@kernel.org
  v3: https://lkml.kernel.org/r/20191007154703.5574-1-will@kernel.org

I didn't receive any feedback last time around, other than some positive
noises from Kees, so please consider this for inclusion in mainline.

Thanks,

Will

Cc: Kees Cook <keescook@chromium.org>
Cc: Ingo Molnar <mingo@kernel.org>
Cc: Elena Reshetova <elena.reshetova@intel.com>
Cc: Peter Zijlstra <peterz@infradead.org>
Cc: Ard Biesheuvel <ard.biesheuvel@linaro.org>
Cc: Hanjun Guo <guohanjun@huawei.com>

--->8

Will Deacon (10):
  lib/refcount: Define constants for saturation and max refcount values
  lib/refcount: Ensure integer operands are treated as signed
  lib/refcount: Remove unused refcount_*_checked() variants
  lib/refcount: Move bulk of REFCOUNT_FULL implementation into header
  lib/refcount: Improve performance of generic REFCOUNT_FULL code
  lib/refcount: Move saturation warnings out of line
  lib/refcount: Consolidate REFCOUNT_{MAX,SATURATED} definitions
  refcount: Consolidate implementations of refcount_t
  lib/refcount: Remove unused 'refcount_error_report()' function
  drivers/lkdtm: Remove references to CONFIG_REFCOUNT_FULL

 arch/Kconfig                       |  21 ---
 arch/arm/Kconfig                   |   1 -
 arch/arm64/Kconfig                 |   1 -
 arch/s390/configs/debug_defconfig  |   1 -
 arch/x86/Kconfig                   |   1 -
 arch/x86/include/asm/asm.h         |   6 -
 arch/x86/include/asm/refcount.h    | 126 --------------
 arch/x86/mm/extable.c              |  49 ------
 drivers/gpu/drm/i915/Kconfig.debug |   1 -
 drivers/misc/lkdtm/refcount.c      |  11 +-
 include/linux/kernel.h             |   7 -
 include/linux/refcount.h           | 269 ++++++++++++++++++++++++-----
 kernel/panic.c                     |  11 --
 lib/refcount.c                     | 255 +++------------------------
 14 files changed, 257 insertions(+), 503 deletions(-)
 delete mode 100644 arch/x86/include/asm/refcount.h

-- 
2.24.0.432.g9d3f5f5b63-goog
```

## [RESEND PATCH v4 01/10] lib/refcount: Define constants for saturation and max refcount values

```patch
The REFCOUNT_FULL implementation uses a different saturation point than
the x86 implementation, which means that the shared refcount code in
lib/refcount.c (e.g. refcount_dec_not_one()) needs to be aware of the
difference.

Rather than duplicate the definitions from the lkdtm driver, instead
move them into linux/refcount.h and update all references accordingly.

Cc: Ingo Molnar <mingo@kernel.org>
Cc: Elena Reshetova <elena.reshetova@intel.com>
Cc: Peter Zijlstra <peterz@infradead.org>
Cc: Ard Biesheuvel <ard.biesheuvel@linaro.org>
Reviewed-by: Kees Cook <keescook@chromium.org>
Signed-off-by: Will Deacon <will@kernel.org>
---
 drivers/misc/lkdtm/refcount.c |  8 --------
 include/linux/refcount.h      | 10 +++++++++-
 lib/refcount.c                | 37 +++++++++++++++++++----------------
 3 files changed, 29 insertions(+), 26 deletions(-)

diff --git a/drivers/misc/lkdtm/refcount.c b/drivers/misc/lkdtm/refcount.c
index 0a146b32da13..abf3b7c1f686 100644
--- a/drivers/misc/lkdtm/refcount.c
+++ b/drivers/misc/lkdtm/refcount.c
@@ -6,14 +6,6 @@
 #include "lkdtm.h"
 #include <linux/refcount.h>
 
-#ifdef CONFIG_REFCOUNT_FULL
-#define REFCOUNT_MAX		(UINT_MAX - 1)
-#define REFCOUNT_SATURATED	UINT_MAX
-#else
-#define REFCOUNT_MAX		INT_MAX
-#define REFCOUNT_SATURATED	(INT_MIN / 2)
-#endif
-
 static void overflow_check(refcount_t *ref)
 {
 	switch (refcount_read(ref)) {
diff --git a/include/linux/refcount.h b/include/linux/refcount.h
index e28cce21bad6..79f62e8d2256 100644
--- a/include/linux/refcount.h
+++ b/include/linux/refcount.h
@@ -4,6 +4,7 @@
 
 #include <linux/atomic.h>
 #include <linux/compiler.h>
+#include <linux/limits.h>
 #include <linux/spinlock_types.h>
 
 struct mutex;
@@ -12,7 +13,7 @@ struct mutex;
  * struct refcount_t - variant of atomic_t specialized for reference counts
  * @refs: atomic_t counter field
  *
- * The counter saturates at UINT_MAX and will not move once
+ * The counter saturates at REFCOUNT_SATURATED and will not move once
  * there. This avoids wrapping the counter and causing 'spurious'
  * use-after-free bugs.
  */
@@ -56,6 +57,9 @@ extern void refcount_dec_checked(refcount_t *r);
 
 #ifdef CONFIG_REFCOUNT_FULL
 
+#define REFCOUNT_MAX		(UINT_MAX - 1)
+#define REFCOUNT_SATURATED	UINT_MAX
+
 #define refcount_add_not_zero	refcount_add_not_zero_checked
 #define refcount_add		refcount_add_checked
 
@@ -68,6 +72,10 @@ extern void refcount_dec_checked(refcount_t *r);
 #define refcount_dec		refcount_dec_checked
 
 #else
+
+#define REFCOUNT_MAX		INT_MAX
+#define REFCOUNT_SATURATED	(INT_MIN / 2)
+
 # ifdef CONFIG_ARCH_HAS_REFCOUNT
 #  include <asm/refcount.h>
 # else
diff --git a/lib/refcount.c b/lib/refcount.c
index 6e904af0fb3e..48b78a423d7d 100644
--- a/lib/refcount.c
+++ b/lib/refcount.c
@@ -5,8 +5,8 @@
  * The interface matches the atomic_t interface (to aid in porting) but only
  * provides the few functions one should use for reference counting.
  *
- * It differs in that the counter saturates at UINT_MAX and will not move once
- * there. This avoids wrapping the counter and causing 'spurious'
+ * It differs in that the counter saturates at REFCOUNT_SATURATED and will not
+ * move once there. This avoids wrapping the counter and causing 'spurious'
  * use-after-free issues.
  *
  * Memory ordering rules are slightly relaxed wrt regular atomic_t functions
@@ -48,7 +48,7 @@
  * @i: the value to add to the refcount
  * @r: the refcount
  *
- * Will saturate at UINT_MAX and WARN.
+ * Will saturate at REFCOUNT_SATURATED and WARN.
  *
  * Provides no memory ordering, it is assumed the caller has guaranteed the
  * object memory to be stable (RCU, etc.). It does provide a control dependency
@@ -69,16 +69,17 @@ bool refcount_add_not_zero_checked(unsigned int i, refcount_t *r)
 		if (!val)
 			return false;
 
-		if (unlikely(val == UINT_MAX))
+		if (unlikely(val == REFCOUNT_SATURATED))
 			return true;
 
 		new = val + i;
 		if (new < val)
-			new = UINT_MAX;
+			new = REFCOUNT_SATURATED;
 
 	} while (!atomic_try_cmpxchg_relaxed(&r->refs, &val, new));
 
-	WARN_ONCE(new == UINT_MAX, "refcount_t: saturated; leaking memory.\n");
+	WARN_ONCE(new == REFCOUNT_SATURATED,
+		  "refcount_t: saturated; leaking memory.\n");
 
 	return true;
 }
@@ -89,7 +90,7 @@ EXPORT_SYMBOL(refcount_add_not_zero_checked);
  * @i: the value to add to the refcount
  * @r: the refcount
  *
- * Similar to atomic_add(), but will saturate at UINT_MAX and WARN.
+ * Similar to atomic_add(), but will saturate at REFCOUNT_SATURATED and WARN.
  *
  * Provides no memory ordering, it is assumed the caller has guaranteed the
  * object memory to be stable (RCU, etc.). It does provide a control dependency
@@ -110,7 +111,8 @@ EXPORT_SYMBOL(refcount_add_checked);
  * refcount_inc_not_zero_checked - increment a refcount unless it is 0
  * @r: the refcount to increment
  *
- * Similar to atomic_inc_not_zero(), but will saturate at UINT_MAX and WARN.
+ * Similar to atomic_inc_not_zero(), but will saturate at REFCOUNT_SATURATED
+ * and WARN.
  *
  * Provides no memory ordering, it is assumed the caller has guaranteed the
  * object memory to be stable (RCU, etc.). It does provide a control dependency
@@ -133,7 +135,8 @@ bool refcount_inc_not_zero_checked(refcount_t *r)
 
 	} while (!atomic_try_cmpxchg_relaxed(&r->refs, &val, new));
 
-	WARN_ONCE(new == UINT_MAX, "refcount_t: saturated; leaking memory.\n");
+	WARN_ONCE(new == REFCOUNT_SATURATED,
+		  "refcount_t: saturated; leaking memory.\n");
 
 	return true;
 }
@@ -143,7 +146,7 @@ EXPORT_SYMBOL(refcount_inc_not_zero_checked);
  * refcount_inc_checked - increment a refcount
  * @r: the refcount to increment
  *
- * Similar to atomic_inc(), but will saturate at UINT_MAX and WARN.
+ * Similar to atomic_inc(), but will saturate at REFCOUNT_SATURATED and WARN.
  *
  * Provides no memory ordering, it is assumed the caller already has a
  * reference on the object.
@@ -164,7 +167,7 @@ EXPORT_SYMBOL(refcount_inc_checked);
  *
  * Similar to atomic_dec_and_test(), but it will WARN, return false and
  * ultimately leak on underflow and will fail to decrement when saturated
- * at UINT_MAX.
+ * at REFCOUNT_SATURATED.
  *
  * Provides release memory ordering, such that prior loads and stores are done
  * before, and provides an acquire ordering on success such that free()
@@ -182,7 +185,7 @@ bool refcount_sub_and_test_checked(unsigned int i, refcount_t *r)
 	unsigned int new, val = atomic_read(&r->refs);
 
 	do {
-		if (unlikely(val == UINT_MAX))
+		if (unlikely(val == REFCOUNT_SATURATED))
 			return false;
 
 		new = val - i;
@@ -207,7 +210,7 @@ EXPORT_SYMBOL(refcount_sub_and_test_checked);
  * @r: the refcount
  *
  * Similar to atomic_dec_and_test(), it will WARN on underflow and fail to
- * decrement when saturated at UINT_MAX.
+ * decrement when saturated at REFCOUNT_SATURATED.
  *
  * Provides release memory ordering, such that prior loads and stores are done
  * before, and provides an acquire ordering on success such that free()
@@ -226,7 +229,7 @@ EXPORT_SYMBOL(refcount_dec_and_test_checked);
  * @r: the refcount
  *
  * Similar to atomic_dec(), it will WARN on underflow and fail to decrement
- * when saturated at UINT_MAX.
+ * when saturated at REFCOUNT_SATURATED.
  *
  * Provides release memory ordering, such that prior loads and stores are done
  * before.
@@ -277,7 +280,7 @@ bool refcount_dec_not_one(refcount_t *r)
 	unsigned int new, val = atomic_read(&r->refs);
 
 	do {
-		if (unlikely(val == UINT_MAX))
+		if (unlikely(val == REFCOUNT_SATURATED))
 			return true;
 
 		if (val == 1)
@@ -302,7 +305,7 @@ EXPORT_SYMBOL(refcount_dec_not_one);
  * @lock: the mutex to be locked
  *
  * Similar to atomic_dec_and_mutex_lock(), it will WARN on underflow and fail
- * to decrement when saturated at UINT_MAX.
+ * to decrement when saturated at REFCOUNT_SATURATED.
  *
  * Provides release memory ordering, such that prior loads and stores are done
  * before, and provides a control dependency such that free() must come after.
@@ -333,7 +336,7 @@ EXPORT_SYMBOL(refcount_dec_and_mutex_lock);
  * @lock: the spinlock to be locked
  *
  * Similar to atomic_dec_and_lock(), it will WARN on underflow and fail to
- * decrement when saturated at UINT_MAX.
+ * decrement when saturated at REFCOUNT_SATURATED.
  *
  * Provides release memory ordering, such that prior loads and stores are done
  * before, and provides a control dependency such that free() must come after.
-- 
2.24.0.432.g9d3f5f5b63-goog
```

## [RESEND PATCH v4 02/10] lib/refcount: Ensure integer operands are treated as signed

```patch
In preparation for changing the saturation point of REFCOUNT_FULL to
INT_MIN / 2, change the type of integer operands passed into the API
from 'unsigned int' to 'int' so that we can avoid casting during
comparisons when we don't want to fall foul of C integral conversion
rules for signed and unsigned types.

Since the kernel is compiled with '-fno-strict-overflow', we don't need
to worry about the UB introduced by signed overflow here. Furthermore,
we're already making heavy use of the atomic_t API, which operates
exclusively on signed types.

Cc: Ingo Molnar <mingo@kernel.org>
Cc: Elena Reshetova <elena.reshetova@intel.com>
Cc: Peter Zijlstra <peterz@infradead.org>
Cc: Ard Biesheuvel <ard.biesheuvel@linaro.org>
Reviewed-by: Kees Cook <keescook@chromium.org>
Signed-off-by: Will Deacon <will@kernel.org>
---
 include/linux/refcount.h | 14 +++++++-------
 lib/refcount.c           |  6 +++---
 2 files changed, 10 insertions(+), 10 deletions(-)

diff --git a/include/linux/refcount.h b/include/linux/refcount.h
index 79f62e8d2256..89066a1471dd 100644
--- a/include/linux/refcount.h
+++ b/include/linux/refcount.h
@@ -28,7 +28,7 @@ typedef struct refcount_struct {
  * @r: the refcount
  * @n: value to which the refcount will be set
  */
-static inline void refcount_set(refcount_t *r, unsigned int n)
+static inline void refcount_set(refcount_t *r, int n)
 {
 	atomic_set(&r->refs, n);
 }
@@ -44,13 +44,13 @@ static inline unsigned int refcount_read(const refcount_t *r)
 	return atomic_read(&r->refs);
 }
 
-extern __must_check bool refcount_add_not_zero_checked(unsigned int i, refcount_t *r);
-extern void refcount_add_checked(unsigned int i, refcount_t *r);
+extern __must_check bool refcount_add_not_zero_checked(int i, refcount_t *r);
+extern void refcount_add_checked(int i, refcount_t *r);
 
 extern __must_check bool refcount_inc_not_zero_checked(refcount_t *r);
 extern void refcount_inc_checked(refcount_t *r);
 
-extern __must_check bool refcount_sub_and_test_checked(unsigned int i, refcount_t *r);
+extern __must_check bool refcount_sub_and_test_checked(int i, refcount_t *r);
 
 extern __must_check bool refcount_dec_and_test_checked(refcount_t *r);
 extern void refcount_dec_checked(refcount_t *r);
@@ -79,12 +79,12 @@ extern void refcount_dec_checked(refcount_t *r);
 # ifdef CONFIG_ARCH_HAS_REFCOUNT
 #  include <asm/refcount.h>
 # else
-static inline __must_check bool refcount_add_not_zero(unsigned int i, refcount_t *r)
+static inline __must_check bool refcount_add_not_zero(int i, refcount_t *r)
 {
 	return atomic_add_unless(&r->refs, i, 0);
 }
 
-static inline void refcount_add(unsigned int i, refcount_t *r)
+static inline void refcount_add(int i, refcount_t *r)
 {
 	atomic_add(i, &r->refs);
 }
@@ -99,7 +99,7 @@ static inline void refcount_inc(refcount_t *r)
 	atomic_inc(&r->refs);
 }
 
-static inline __must_check bool refcount_sub_and_test(unsigned int i, refcount_t *r)
+static inline __must_check bool refcount_sub_and_test(int i, refcount_t *r)
 {
 	return atomic_sub_and_test(i, &r->refs);
 }
diff --git a/lib/refcount.c b/lib/refcount.c
index 48b78a423d7d..719b0bc42ab1 100644
--- a/lib/refcount.c
+++ b/lib/refcount.c
@@ -61,7 +61,7 @@
  *
  * Return: false if the passed refcount is 0, true otherwise
  */
-bool refcount_add_not_zero_checked(unsigned int i, refcount_t *r)
+bool refcount_add_not_zero_checked(int i, refcount_t *r)
 {
 	unsigned int new, val = atomic_read(&r->refs);
 
@@ -101,7 +101,7 @@ EXPORT_SYMBOL(refcount_add_not_zero_checked);
  * cases, refcount_inc(), or one of its variants, should instead be used to
  * increment a reference count.
  */
-void refcount_add_checked(unsigned int i, refcount_t *r)
+void refcount_add_checked(int i, refcount_t *r)
 {
 	WARN_ONCE(!refcount_add_not_zero_checked(i, r), "refcount_t: addition on 0; use-after-free.\n");
 }
@@ -180,7 +180,7 @@ EXPORT_SYMBOL(refcount_inc_checked);
  *
  * Return: true if the resulting refcount is 0, false otherwise
  */
-bool refcount_sub_and_test_checked(unsigned int i, refcount_t *r)
+bool refcount_sub_and_test_checked(int i, refcount_t *r)
 {
 	unsigned int new, val = atomic_read(&r->refs);
 
-- 
2.24.0.432.g9d3f5f5b63-goog
```

## [RESEND PATCH v4 03/10] lib/refcount: Remove unused refcount_*_checked() variants
```patch
The full-fat refcount implementation is exposed via a set of functions
suffixed with "_checked()", the idea being that code can choose to use
the more expensive, yet more secure implementation on a case-by-case
basis.

In reality, this hasn't happened, so with a grand total of zero users,
let's remove the checked variants for now by simply dropping the suffix
and predicating the out-of-line functions on CONFIG_REFCOUNT_FULL=y.

Cc: Ingo Molnar <mingo@kernel.org>
Cc: Elena Reshetova <elena.reshetova@intel.com>
Cc: Peter Zijlstra <peterz@infradead.org>
Cc: Ard Biesheuvel <ard.biesheuvel@linaro.org>
Reviewed-by: Kees Cook <keescook@chromium.org>
Signed-off-by: Will Deacon <will@kernel.org>
---
 include/linux/refcount.h | 25 ++++++-------------
 lib/refcount.c           | 54 +++++++++++++++++++++-------------------
 2 files changed, 36 insertions(+), 43 deletions(-)

diff --git a/include/linux/refcount.h b/include/linux/refcount.h
index 89066a1471dd..edd505d1a23b 100644
--- a/include/linux/refcount.h
+++ b/include/linux/refcount.h
@@ -44,32 +44,21 @@ static inline unsigned int refcount_read(const refcount_t *r)
 	return atomic_read(&r->refs);
 }
 
-extern __must_check bool refcount_add_not_zero_checked(int i, refcount_t *r);
-extern void refcount_add_checked(int i, refcount_t *r);
-
-extern __must_check bool refcount_inc_not_zero_checked(refcount_t *r);
-extern void refcount_inc_checked(refcount_t *r);
-
-extern __must_check bool refcount_sub_and_test_checked(int i, refcount_t *r);
-
-extern __must_check bool refcount_dec_and_test_checked(refcount_t *r);
-extern void refcount_dec_checked(refcount_t *r);
-
 #ifdef CONFIG_REFCOUNT_FULL
 
 #define REFCOUNT_MAX		(UINT_MAX - 1)
 #define REFCOUNT_SATURATED	UINT_MAX
 
-#define refcount_add_not_zero	refcount_add_not_zero_checked
-#define refcount_add		refcount_add_checked
+extern __must_check bool refcount_add_not_zero(int i, refcount_t *r);
+extern void refcount_add(int i, refcount_t *r);
 
-#define refcount_inc_not_zero	refcount_inc_not_zero_checked
-#define refcount_inc		refcount_inc_checked
+extern __must_check bool refcount_inc_not_zero(refcount_t *r);
+extern void refcount_inc(refcount_t *r);
 
-#define refcount_sub_and_test	refcount_sub_and_test_checked
+extern __must_check bool refcount_sub_and_test(int i, refcount_t *r);
 
-#define refcount_dec_and_test	refcount_dec_and_test_checked
-#define refcount_dec		refcount_dec_checked
+extern __must_check bool refcount_dec_and_test(refcount_t *r);
+extern void refcount_dec(refcount_t *r);
 
 #else
 
diff --git a/lib/refcount.c b/lib/refcount.c
index 719b0bc42ab1..a2f670998cee 100644
--- a/lib/refcount.c
+++ b/lib/refcount.c
@@ -43,8 +43,10 @@
 #include <linux/spinlock.h>
 #include <linux/bug.h>
 
+#ifdef CONFIG_REFCOUNT_FULL
+
 /**
- * refcount_add_not_zero_checked - add a value to a refcount unless it is 0
+ * refcount_add_not_zero - add a value to a refcount unless it is 0
  * @i: the value to add to the refcount
  * @r: the refcount
  *
@@ -61,7 +63,7 @@
  *
  * Return: false if the passed refcount is 0, true otherwise
  */
-bool refcount_add_not_zero_checked(int i, refcount_t *r)
+bool refcount_add_not_zero(int i, refcount_t *r)
 {
 	unsigned int new, val = atomic_read(&r->refs);
 
@@ -83,10 +85,10 @@ bool refcount_add_not_zero_checked(int i, refcount_t *r)
 
 	return true;
 }
-EXPORT_SYMBOL(refcount_add_not_zero_checked);
+EXPORT_SYMBOL(refcount_add_not_zero);
 
 /**
- * refcount_add_checked - add a value to a refcount
+ * refcount_add - add a value to a refcount
  * @i: the value to add to the refcount
  * @r: the refcount
  *
@@ -101,14 +103,14 @@ EXPORT_SYMBOL(refcount_add_not_zero_checked);
  * cases, refcount_inc(), or one of its variants, should instead be used to
  * increment a reference count.
  */
-void refcount_add_checked(int i, refcount_t *r)
+void refcount_add(int i, refcount_t *r)
 {
-	WARN_ONCE(!refcount_add_not_zero_checked(i, r), "refcount_t: addition on 0; use-after-free.\n");
+	WARN_ONCE(!refcount_add_not_zero(i, r), "refcount_t: addition on 0; use-after-free.\n");
 }
-EXPORT_SYMBOL(refcount_add_checked);
+EXPORT_SYMBOL(refcount_add);
 
 /**
- * refcount_inc_not_zero_checked - increment a refcount unless it is 0
+ * refcount_inc_not_zero - increment a refcount unless it is 0
  * @r: the refcount to increment
  *
  * Similar to atomic_inc_not_zero(), but will saturate at REFCOUNT_SATURATED
@@ -120,7 +122,7 @@ EXPORT_SYMBOL(refcount_add_checked);
  *
  * Return: true if the increment was successful, false otherwise
  */
-bool refcount_inc_not_zero_checked(refcount_t *r)
+bool refcount_inc_not_zero(refcount_t *r)
 {
 	unsigned int new, val = atomic_read(&r->refs);
 
@@ -140,10 +142,10 @@ bool refcount_inc_not_zero_checked(refcount_t *r)
 
 	return true;
 }
-EXPORT_SYMBOL(refcount_inc_not_zero_checked);
+EXPORT_SYMBOL(refcount_inc_not_zero);
 
 /**
- * refcount_inc_checked - increment a refcount
+ * refcount_inc - increment a refcount
  * @r: the refcount to increment
  *
  * Similar to atomic_inc(), but will saturate at REFCOUNT_SATURATED and WARN.
@@ -154,14 +156,14 @@ EXPORT_SYMBOL(refcount_inc_not_zero_checked);
  * Will WARN if the refcount is 0, as this represents a possible use-after-free
  * condition.
  */
-void refcount_inc_checked(refcount_t *r)
+void refcount_inc(refcount_t *r)
 {
-	WARN_ONCE(!refcount_inc_not_zero_checked(r), "refcount_t: increment on 0; use-after-free.\n");
+	WARN_ONCE(!refcount_inc_not_zero(r), "refcount_t: increment on 0; use-after-free.\n");
 }
-EXPORT_SYMBOL(refcount_inc_checked);
+EXPORT_SYMBOL(refcount_inc);
 
 /**
- * refcount_sub_and_test_checked - subtract from a refcount and test if it is 0
+ * refcount_sub_and_test - subtract from a refcount and test if it is 0
  * @i: amount to subtract from the refcount
  * @r: the refcount
  *
@@ -180,7 +182,7 @@ EXPORT_SYMBOL(refcount_inc_checked);
  *
  * Return: true if the resulting refcount is 0, false otherwise
  */
-bool refcount_sub_and_test_checked(int i, refcount_t *r)
+bool refcount_sub_and_test(int i, refcount_t *r)
 {
 	unsigned int new, val = atomic_read(&r->refs);
 
@@ -203,10 +205,10 @@ bool refcount_sub_and_test_checked(int i, refcount_t *r)
 	return false;
 
 }
-EXPORT_SYMBOL(refcount_sub_and_test_checked);
+EXPORT_SYMBOL(refcount_sub_and_test);
 
 /**
- * refcount_dec_and_test_checked - decrement a refcount and test if it is 0
+ * refcount_dec_and_test - decrement a refcount and test if it is 0
  * @r: the refcount
  *
  * Similar to atomic_dec_and_test(), it will WARN on underflow and fail to
@@ -218,14 +220,14 @@ EXPORT_SYMBOL(refcount_sub_and_test_checked);
  *
  * Return: true if the resulting refcount is 0, false otherwise
  */
-bool refcount_dec_and_test_checked(refcount_t *r)
+bool refcount_dec_and_test(refcount_t *r)
 {
-	return refcount_sub_and_test_checked(1, r);
+	return refcount_sub_and_test(1, r);
 }
-EXPORT_SYMBOL(refcount_dec_and_test_checked);
+EXPORT_SYMBOL(refcount_dec_and_test);
 
 /**
- * refcount_dec_checked - decrement a refcount
+ * refcount_dec - decrement a refcount
  * @r: the refcount
  *
  * Similar to atomic_dec(), it will WARN on underflow and fail to decrement
@@ -234,11 +236,13 @@ EXPORT_SYMBOL(refcount_dec_and_test_checked);
  * Provides release memory ordering, such that prior loads and stores are done
  * before.
  */
-void refcount_dec_checked(refcount_t *r)
+void refcount_dec(refcount_t *r)
 {
-	WARN_ONCE(refcount_dec_and_test_checked(r), "refcount_t: decrement hit 0; leaking memory.\n");
+	WARN_ONCE(refcount_dec_and_test(r), "refcount_t: decrement hit 0; leaking memory.\n");
 }
-EXPORT_SYMBOL(refcount_dec_checked);
+EXPORT_SYMBOL(refcount_dec);
+
+#endif /* CONFIG_REFCOUNT_FULL */
 
 /**
  * refcount_dec_if_one - decrement a refcount if it is 1
-- 
2.24.0.432.g9d3f5f5b63-goog
```

## [RESEND PATCH v4 04/10] lib/refcount: Move bulk of REFCOUNT_FULL implementation into header

```patch
In an effort to improve performance of the REFCOUNT_FULL implementation,
move the bulk of its functions into linux/refcount.h. This allows them
to be inlined in the same way as if they had been provided via
CONFIG_ARCH_HAS_REFCOUNT.

Cc: Ingo Molnar <mingo@kernel.org>
Cc: Elena Reshetova <elena.reshetova@intel.com>
Cc: Peter Zijlstra <peterz@infradead.org>
Cc: Ard Biesheuvel <ard.biesheuvel@linaro.org>
Reviewed-by: Kees Cook <keescook@chromium.org>
Signed-off-by: Will Deacon <will@kernel.org>
---
 include/linux/refcount.h | 237 ++++++++++++++++++++++++++++++++++++--
 lib/refcount.c           | 238 +--------------------------------------
 2 files changed, 229 insertions(+), 246 deletions(-)

diff --git a/include/linux/refcount.h b/include/linux/refcount.h
index edd505d1a23b..e719b5b1220e 100644
--- a/include/linux/refcount.h
+++ b/include/linux/refcount.h
@@ -45,22 +45,241 @@ static inline unsigned int refcount_read(const refcount_t *r)
 }
 
 #ifdef CONFIG_REFCOUNT_FULL
+#include <linux/bug.h>
 
 #define REFCOUNT_MAX		(UINT_MAX - 1)
 #define REFCOUNT_SATURATED	UINT_MAX
 
-extern __must_check bool refcount_add_not_zero(int i, refcount_t *r);
-extern void refcount_add(int i, refcount_t *r);
+/*
+ * Variant of atomic_t specialized for reference counts.
+ *
+ * The interface matches the atomic_t interface (to aid in porting) but only
+ * provides the few functions one should use for reference counting.
+ *
+ * It differs in that the counter saturates at REFCOUNT_SATURATED and will not
+ * move once there. This avoids wrapping the counter and causing 'spurious'
+ * use-after-free issues.
+ *
+ * Memory ordering rules are slightly relaxed wrt regular atomic_t functions
+ * and provide only what is strictly required for refcounts.
+ *
+ * The increments are fully relaxed; these will not provide ordering. The
+ * rationale is that whatever is used to obtain the object we're increasing the
+ * reference count on will provide the ordering. For locked data structures,
+ * its the lock acquire, for RCU/lockless data structures its the dependent
+ * load.
+ *
+ * Do note that inc_not_zero() provides a control dependency which will order
+ * future stores against the inc, this ensures we'll never modify the object
+ * if we did not in fact acquire a reference.
+ *
+ * The decrements will provide release order, such that all the prior loads and
+ * stores will be issued before, it also provides a control dependency, which
+ * will order us against the subsequent free().
+ *
+ * The control dependency is against the load of the cmpxchg (ll/sc) that
+ * succeeded. This means the stores aren't fully ordered, but this is fine
+ * because the 1->0 transition indicates no concurrency.
+ *
+ * Note that the allocator is responsible for ordering things between free()
+ * and alloc().
+ *
+ * The decrements dec_and_test() and sub_and_test() also provide acquire
+ * ordering on success.
+ *
+ */
+
+/**
+ * refcount_add_not_zero - add a value to a refcount unless it is 0
+ * @i: the value to add to the refcount
+ * @r: the refcount
+ *
+ * Will saturate at REFCOUNT_SATURATED and WARN.
+ *
+ * Provides no memory ordering, it is assumed the caller has guaranteed the
+ * object memory to be stable (RCU, etc.). It does provide a control dependency
+ * and thereby orders future stores. See the comment on top.
+ *
+ * Use of this function is not recommended for the normal reference counting
+ * use case in which references are taken and released one at a time.  In these
+ * cases, refcount_inc(), or one of its variants, should instead be used to
+ * increment a reference count.
+ *
+ * Return: false if the passed refcount is 0, true otherwise
+ */
+static inline __must_check bool refcount_add_not_zero(int i, refcount_t *r)
+{
+	unsigned int new, val = atomic_read(&r->refs);
+
+	do {
+		if (!val)
+			return false;
+
+		if (unlikely(val == REFCOUNT_SATURATED))
+			return true;
+
+		new = val + i;
+		if (new < val)
+			new = REFCOUNT_SATURATED;
+
+	} while (!atomic_try_cmpxchg_relaxed(&r->refs, &val, new));
+
+	WARN_ONCE(new == REFCOUNT_SATURATED,
+		  "refcount_t: saturated; leaking memory.\n");
+
+	return true;
+}
+
+/**
+ * refcount_add - add a value to a refcount
+ * @i: the value to add to the refcount
+ * @r: the refcount
+ *
+ * Similar to atomic_add(), but will saturate at REFCOUNT_SATURATED and WARN.
+ *
+ * Provides no memory ordering, it is assumed the caller has guaranteed the
+ * object memory to be stable (RCU, etc.). It does provide a control dependency
+ * and thereby orders future stores. See the comment on top.
+ *
+ * Use of this function is not recommended for the normal reference counting
+ * use case in which references are taken and released one at a time.  In these
+ * cases, refcount_inc(), or one of its variants, should instead be used to
+ * increment a reference count.
+ */
+static inline void refcount_add(int i, refcount_t *r)
+{
+	WARN_ONCE(!refcount_add_not_zero(i, r), "refcount_t: addition on 0; use-after-free.\n");
+}
+
+/**
+ * refcount_inc_not_zero - increment a refcount unless it is 0
+ * @r: the refcount to increment
+ *
+ * Similar to atomic_inc_not_zero(), but will saturate at REFCOUNT_SATURATED
+ * and WARN.
+ *
+ * Provides no memory ordering, it is assumed the caller has guaranteed the
+ * object memory to be stable (RCU, etc.). It does provide a control dependency
+ * and thereby orders future stores. See the comment on top.
+ *
+ * Return: true if the increment was successful, false otherwise
+ */
+static inline __must_check bool refcount_inc_not_zero(refcount_t *r)
+{
+	unsigned int new, val = atomic_read(&r->refs);
+
+	do {
+		new = val + 1;
 
-extern __must_check bool refcount_inc_not_zero(refcount_t *r);
-extern void refcount_inc(refcount_t *r);
+		if (!val)
+			return false;
 
-extern __must_check bool refcount_sub_and_test(int i, refcount_t *r);
+		if (unlikely(!new))
+			return true;
 
-extern __must_check bool refcount_dec_and_test(refcount_t *r);
-extern void refcount_dec(refcount_t *r);
+	} while (!atomic_try_cmpxchg_relaxed(&r->refs, &val, new));
+
+	WARN_ONCE(new == REFCOUNT_SATURATED,
+		  "refcount_t: saturated; leaking memory.\n");
+
+	return true;
+}
+
+/**
+ * refcount_inc - increment a refcount
+ * @r: the refcount to increment
+ *
+ * Similar to atomic_inc(), but will saturate at REFCOUNT_SATURATED and WARN.
+ *
+ * Provides no memory ordering, it is assumed the caller already has a
+ * reference on the object.
+ *
+ * Will WARN if the refcount is 0, as this represents a possible use-after-free
+ * condition.
+ */
+static inline void refcount_inc(refcount_t *r)
+{
+	WARN_ONCE(!refcount_inc_not_zero(r), "refcount_t: increment on 0; use-after-free.\n");
+}
+
+/**
+ * refcount_sub_and_test - subtract from a refcount and test if it is 0
+ * @i: amount to subtract from the refcount
+ * @r: the refcount
+ *
+ * Similar to atomic_dec_and_test(), but it will WARN, return false and
+ * ultimately leak on underflow and will fail to decrement when saturated
+ * at REFCOUNT_SATURATED.
+ *
+ * Provides release memory ordering, such that prior loads and stores are done
+ * before, and provides an acquire ordering on success such that free()
+ * must come after.
+ *
+ * Use of this function is not recommended for the normal reference counting
+ * use case in which references are taken and released one at a time.  In these
+ * cases, refcount_dec(), or one of its variants, should instead be used to
+ * decrement a reference count.
+ *
+ * Return: true if the resulting refcount is 0, false otherwise
+ */
+static inline __must_check bool refcount_sub_and_test(int i, refcount_t *r)
+{
+	unsigned int new, val = atomic_read(&r->refs);
+
+	do {
+		if (unlikely(val == REFCOUNT_SATURATED))
+			return false;
+
+		new = val - i;
+		if (new > val) {
+			WARN_ONCE(new > val, "refcount_t: underflow; use-after-free.\n");
+			return false;
+		}
+
+	} while (!atomic_try_cmpxchg_release(&r->refs, &val, new));
+
+	if (!new) {
+		smp_acquire__after_ctrl_dep();
+		return true;
+	}
+	return false;
+
+}
+
+/**
+ * refcount_dec_and_test - decrement a refcount and test if it is 0
+ * @r: the refcount
+ *
+ * Similar to atomic_dec_and_test(), it will WARN on underflow and fail to
+ * decrement when saturated at REFCOUNT_SATURATED.
+ *
+ * Provides release memory ordering, such that prior loads and stores are done
+ * before, and provides an acquire ordering on success such that free()
+ * must come after.
+ *
+ * Return: true if the resulting refcount is 0, false otherwise
+ */
+static inline __must_check bool refcount_dec_and_test(refcount_t *r)
+{
+	return refcount_sub_and_test(1, r);
+}
+
+/**
+ * refcount_dec - decrement a refcount
+ * @r: the refcount
+ *
+ * Similar to atomic_dec(), it will WARN on underflow and fail to decrement
+ * when saturated at REFCOUNT_SATURATED.
+ *
+ * Provides release memory ordering, such that prior loads and stores are done
+ * before.
+ */
+static inline void refcount_dec(refcount_t *r)
+{
+	WARN_ONCE(refcount_dec_and_test(r), "refcount_t: decrement hit 0; leaking memory.\n");
+}
 
-#else
+#else /* CONFIG_REFCOUNT_FULL */
 
 #define REFCOUNT_MAX		INT_MAX
 #define REFCOUNT_SATURATED	(INT_MIN / 2)
@@ -103,7 +322,7 @@ static inline void refcount_dec(refcount_t *r)
 	atomic_dec(&r->refs);
 }
 # endif /* !CONFIG_ARCH_HAS_REFCOUNT */
-#endif /* CONFIG_REFCOUNT_FULL */
+#endif /* !CONFIG_REFCOUNT_FULL */
 
 extern __must_check bool refcount_dec_if_one(refcount_t *r);
 extern __must_check bool refcount_dec_not_one(refcount_t *r);
diff --git a/lib/refcount.c b/lib/refcount.c
index a2f670998cee..3a534fbebdcc 100644
--- a/lib/refcount.c
+++ b/lib/refcount.c
@@ -1,41 +1,6 @@
 // SPDX-License-Identifier: GPL-2.0
 /*
- * Variant of atomic_t specialized for reference counts.
- *
- * The interface matches the atomic_t interface (to aid in porting) but only
- * provides the few functions one should use for reference counting.
- *
- * It differs in that the counter saturates at REFCOUNT_SATURATED and will not
- * move once there. This avoids wrapping the counter and causing 'spurious'
- * use-after-free issues.
- *
- * Memory ordering rules are slightly relaxed wrt regular atomic_t functions
- * and provide only what is strictly required for refcounts.
- *
- * The increments are fully relaxed; these will not provide ordering. The
- * rationale is that whatever is used to obtain the object we're increasing the
- * reference count on will provide the ordering. For locked data structures,
- * its the lock acquire, for RCU/lockless data structures its the dependent
- * load.
- *
- * Do note that inc_not_zero() provides a control dependency which will order
- * future stores against the inc, this ensures we'll never modify the object
- * if we did not in fact acquire a reference.
- *
- * The decrements will provide release order, such that all the prior loads and
- * stores will be issued before, it also provides a control dependency, which
- * will order us against the subsequent free().
- *
- * The control dependency is against the load of the cmpxchg (ll/sc) that
- * succeeded. This means the stores aren't fully ordered, but this is fine
- * because the 1->0 transition indicates no concurrency.
- *
- * Note that the allocator is responsible for ordering things between free()
- * and alloc().
- *
- * The decrements dec_and_test() and sub_and_test() also provide acquire
- * ordering on success.
- *
+ * Out-of-line refcount functions common to all refcount implementations.
  */
 
 #include <linux/mutex.h>
@@ -43,207 +8,6 @@
 #include <linux/spinlock.h>
 #include <linux/bug.h>
 
-#ifdef CONFIG_REFCOUNT_FULL
-
-/**
- * refcount_add_not_zero - add a value to a refcount unless it is 0
- * @i: the value to add to the refcount
- * @r: the refcount
- *
- * Will saturate at REFCOUNT_SATURATED and WARN.
- *
- * Provides no memory ordering, it is assumed the caller has guaranteed the
- * object memory to be stable (RCU, etc.). It does provide a control dependency
- * and thereby orders future stores. See the comment on top.
- *
- * Use of this function is not recommended for the normal reference counting
- * use case in which references are taken and released one at a time.  In these
- * cases, refcount_inc(), or one of its variants, should instead be used to
- * increment a reference count.
- *
- * Return: false if the passed refcount is 0, true otherwise
- */
-bool refcount_add_not_zero(int i, refcount_t *r)
-{
-	unsigned int new, val = atomic_read(&r->refs);
-
-	do {
-		if (!val)
-			return false;
-
-		if (unlikely(val == REFCOUNT_SATURATED))
-			return true;
-
-		new = val + i;
-		if (new < val)
-			new = REFCOUNT_SATURATED;
-
-	} while (!atomic_try_cmpxchg_relaxed(&r->refs, &val, new));
-
-	WARN_ONCE(new == REFCOUNT_SATURATED,
-		  "refcount_t: saturated; leaking memory.\n");
-
-	return true;
-}
-EXPORT_SYMBOL(refcount_add_not_zero);
-
-/**
- * refcount_add - add a value to a refcount
- * @i: the value to add to the refcount
- * @r: the refcount
- *
- * Similar to atomic_add(), but will saturate at REFCOUNT_SATURATED and WARN.
- *
- * Provides no memory ordering, it is assumed the caller has guaranteed the
- * object memory to be stable (RCU, etc.). It does provide a control dependency
- * and thereby orders future stores. See the comment on top.
- *
- * Use of this function is not recommended for the normal reference counting
- * use case in which references are taken and released one at a time.  In these
- * cases, refcount_inc(), or one of its variants, should instead be used to
- * increment a reference count.
- */
-void refcount_add(int i, refcount_t *r)
-{
-	WARN_ONCE(!refcount_add_not_zero(i, r), "refcount_t: addition on 0; use-after-free.\n");
-}
-EXPORT_SYMBOL(refcount_add);
-
-/**
- * refcount_inc_not_zero - increment a refcount unless it is 0
- * @r: the refcount to increment
- *
- * Similar to atomic_inc_not_zero(), but will saturate at REFCOUNT_SATURATED
- * and WARN.
- *
- * Provides no memory ordering, it is assumed the caller has guaranteed the
- * object memory to be stable (RCU, etc.). It does provide a control dependency
- * and thereby orders future stores. See the comment on top.
- *
- * Return: true if the increment was successful, false otherwise
- */
-bool refcount_inc_not_zero(refcount_t *r)
-{
-	unsigned int new, val = atomic_read(&r->refs);
-
-	do {
-		new = val + 1;
-
-		if (!val)
-			return false;
-
-		if (unlikely(!new))
-			return true;
-
-	} while (!atomic_try_cmpxchg_relaxed(&r->refs, &val, new));
-
-	WARN_ONCE(new == REFCOUNT_SATURATED,
-		  "refcount_t: saturated; leaking memory.\n");
-
-	return true;
-}
-EXPORT_SYMBOL(refcount_inc_not_zero);
-
-/**
- * refcount_inc - increment a refcount
- * @r: the refcount to increment
- *
- * Similar to atomic_inc(), but will saturate at REFCOUNT_SATURATED and WARN.
- *
- * Provides no memory ordering, it is assumed the caller already has a
- * reference on the object.
- *
- * Will WARN if the refcount is 0, as this represents a possible use-after-free
- * condition.
- */
-void refcount_inc(refcount_t *r)
-{
-	WARN_ONCE(!refcount_inc_not_zero(r), "refcount_t: increment on 0; use-after-free.\n");
-}
-EXPORT_SYMBOL(refcount_inc);
-
-/**
- * refcount_sub_and_test - subtract from a refcount and test if it is 0
- * @i: amount to subtract from the refcount
- * @r: the refcount
- *
- * Similar to atomic_dec_and_test(), but it will WARN, return false and
- * ultimately leak on underflow and will fail to decrement when saturated
- * at REFCOUNT_SATURATED.
- *
- * Provides release memory ordering, such that prior loads and stores are done
- * before, and provides an acquire ordering on success such that free()
- * must come after.
- *
- * Use of this function is not recommended for the normal reference counting
- * use case in which references are taken and released one at a time.  In these
- * cases, refcount_dec(), or one of its variants, should instead be used to
- * decrement a reference count.
- *
- * Return: true if the resulting refcount is 0, false otherwise
- */
-bool refcount_sub_and_test(int i, refcount_t *r)
-{
-	unsigned int new, val = atomic_read(&r->refs);
-
-	do {
-		if (unlikely(val == REFCOUNT_SATURATED))
-			return false;
-
-		new = val - i;
-		if (new > val) {
-			WARN_ONCE(new > val, "refcount_t: underflow; use-after-free.\n");
-			return false;
-		}
-
-	} while (!atomic_try_cmpxchg_release(&r->refs, &val, new));
-
-	if (!new) {
-		smp_acquire__after_ctrl_dep();
-		return true;
-	}
-	return false;
-
-}
-EXPORT_SYMBOL(refcount_sub_and_test);
-
-/**
- * refcount_dec_and_test - decrement a refcount and test if it is 0
- * @r: the refcount
- *
- * Similar to atomic_dec_and_test(), it will WARN on underflow and fail to
- * decrement when saturated at REFCOUNT_SATURATED.
- *
- * Provides release memory ordering, such that prior loads and stores are done
- * before, and provides an acquire ordering on success such that free()
- * must come after.
- *
- * Return: true if the resulting refcount is 0, false otherwise
- */
-bool refcount_dec_and_test(refcount_t *r)
-{
-	return refcount_sub_and_test(1, r);
-}
-EXPORT_SYMBOL(refcount_dec_and_test);
-
-/**
- * refcount_dec - decrement a refcount
- * @r: the refcount
- *
- * Similar to atomic_dec(), it will WARN on underflow and fail to decrement
- * when saturated at REFCOUNT_SATURATED.
- *
- * Provides release memory ordering, such that prior loads and stores are done
- * before.
- */
-void refcount_dec(refcount_t *r)
-{
-	WARN_ONCE(refcount_dec_and_test(r), "refcount_t: decrement hit 0; leaking memory.\n");
-}
-EXPORT_SYMBOL(refcount_dec);
-
-#endif /* CONFIG_REFCOUNT_FULL */
-
 /**
  * refcount_dec_if_one - decrement a refcount if it is 1
  * @r: the refcount
-- 
2.24.0.432.g9d3f5f5b63-goog
```

## [RESEND PATCH v4 05/10] lib/refcount: Improve performance of generic REFCOUNT_FULL code

```patch
Rewrite the generic REFCOUNT_FULL implementation so that the saturation
point is moved to INT_MIN / 2. This allows us to defer the sanity checks
until after the atomic operation, which removes many uses of cmpxchg()
in favour of atomic_fetch_{add,sub}().

Some crude perf results obtained from lkdtm show substantially less
overhead, despite the checking:

 $ perf stat -r 3 -B -- echo {ATOMIC,REFCOUNT}_TIMING >/sys/kernel/debug/provoke-crash/DIRECT

 # arm64
 ATOMIC_TIMING:                                      46.50451 +- 0.00134 seconds time elapsed  ( +-  0.00% )
 REFCOUNT_TIMING (REFCOUNT_FULL, mainline):          77.57522 +- 0.00982 seconds time elapsed  ( +-  0.01% )
 REFCOUNT_TIMING (REFCOUNT_FULL, this series):       48.7181 +- 0.0256 seconds time elapsed  ( +-  0.05% )

 # x86
 ATOMIC_TIMING:                                      31.6225 +- 0.0776 seconds time elapsed  ( +-  0.25% )
 REFCOUNT_TIMING (!REFCOUNT_FULL, mainline/x86 asm): 31.6689 +- 0.0901 seconds time elapsed  ( +-  0.28% )
 REFCOUNT_TIMING (REFCOUNT_FULL, mainline):          53.203 +- 0.138 seconds time elapsed  ( +-  0.26% )
 REFCOUNT_TIMING (REFCOUNT_FULL, this series):       31.7408 +- 0.0486 seconds time elapsed  ( +-  0.15% )

Cc: Ingo Molnar <mingo@kernel.org>
Cc: Elena Reshetova <elena.reshetova@intel.com>
Cc: Peter Zijlstra <peterz@infradead.org>
Cc: Ard Biesheuvel <ard.biesheuvel@linaro.org>
Tested-by: Hanjun Guo <guohanjun@huawei.com>
Tested-by: Jan Glauber <jglauber@marvell.com>
Reviewed-by: Kees Cook <keescook@chromium.org>
Signed-off-by: Will Deacon <will@kernel.org>
---
 include/linux/refcount.h | 131 ++++++++++++++++++++++-----------------
 1 file changed, 75 insertions(+), 56 deletions(-)

diff --git a/include/linux/refcount.h b/include/linux/refcount.h
index e719b5b1220e..e3b218d669ce 100644
--- a/include/linux/refcount.h
+++ b/include/linux/refcount.h
@@ -47,8 +47,8 @@ static inline unsigned int refcount_read(const refcount_t *r)
 #ifdef CONFIG_REFCOUNT_FULL
 #include <linux/bug.h>
 
-#define REFCOUNT_MAX		(UINT_MAX - 1)
-#define REFCOUNT_SATURATED	UINT_MAX
+#define REFCOUNT_MAX		INT_MAX
+#define REFCOUNT_SATURATED	(INT_MIN / 2)
 
 /*
  * Variant of atomic_t specialized for reference counts.
@@ -56,9 +56,47 @@ static inline unsigned int refcount_read(const refcount_t *r)
  * The interface matches the atomic_t interface (to aid in porting) but only
  * provides the few functions one should use for reference counting.
  *
- * It differs in that the counter saturates at REFCOUNT_SATURATED and will not
- * move once there. This avoids wrapping the counter and causing 'spurious'
- * use-after-free issues.
+ * Saturation semantics
+ * ====================
+ *
+ * refcount_t differs from atomic_t in that the counter saturates at
+ * REFCOUNT_SATURATED and will not move once there. This avoids wrapping the
+ * counter and causing 'spurious' use-after-free issues. In order to avoid the
+ * cost associated with introducing cmpxchg() loops into all of the saturating
+ * operations, we temporarily allow the counter to take on an unchecked value
+ * and then explicitly set it to REFCOUNT_SATURATED on detecting that underflow
+ * or overflow has occurred. Although this is racy when multiple threads
+ * access the refcount concurrently, by placing REFCOUNT_SATURATED roughly
+ * equidistant from 0 and INT_MAX we minimise the scope for error:
+ *
+ * 	                           INT_MAX     REFCOUNT_SATURATED   UINT_MAX
+ *   0                          (0x7fff_ffff)    (0xc000_0000)    (0xffff_ffff)
+ *   +--------------------------------+----------------+----------------+
+ *                                     <---------- bad value! ---------->
+ *
+ * (in a signed view of the world, the "bad value" range corresponds to
+ * a negative counter value).
+ *
+ * As an example, consider a refcount_inc() operation that causes the counter
+ * to overflow:
+ *
+ * 	int old = atomic_fetch_add_relaxed(r);
+ *	// old is INT_MAX, refcount now INT_MIN (0x8000_0000)
+ *	if (old < 0)
+ *		atomic_set(r, REFCOUNT_SATURATED);
+ *
+ * If another thread also performs a refcount_inc() operation between the two
+ * atomic operations, then the count will continue to edge closer to 0. If it
+ * reaches a value of 1 before /any/ of the threads reset it to the saturated
+ * value, then a concurrent refcount_dec_and_test() may erroneously free the
+ * underlying object. Given the precise timing details involved with the
+ * round-robin scheduling of each thread manipulating the refcount and the need
+ * to hit the race multiple times in succession, there doesn't appear to be a
+ * practical avenue of attack even if using refcount_add() operations with
+ * larger increments.
+ *
+ * Memory ordering
+ * ===============
  *
  * Memory ordering rules are slightly relaxed wrt regular atomic_t functions
  * and provide only what is strictly required for refcounts.
@@ -109,25 +147,19 @@ static inline unsigned int refcount_read(const refcount_t *r)
  */
 static inline __must_check bool refcount_add_not_zero(int i, refcount_t *r)
 {
-	unsigned int new, val = atomic_read(&r->refs);
+	int old = refcount_read(r);
 
 	do {
-		if (!val)
-			return false;
-
-		if (unlikely(val == REFCOUNT_SATURATED))
-			return true;
-
-		new = val + i;
-		if (new < val)
-			new = REFCOUNT_SATURATED;
+		if (!old)
+			break;
+	} while (!atomic_try_cmpxchg_relaxed(&r->refs, &old, old + i));
 
-	} while (!atomic_try_cmpxchg_relaxed(&r->refs, &val, new));
-
-	WARN_ONCE(new == REFCOUNT_SATURATED,
-		  "refcount_t: saturated; leaking memory.\n");
+	if (unlikely(old < 0 || old + i < 0)) {
+		refcount_set(r, REFCOUNT_SATURATED);
+		WARN_ONCE(1, "refcount_t: saturated; leaking memory.\n");
+	}
 
-	return true;
+	return old;
 }
 
 /**
@@ -148,7 +180,13 @@ static inline __must_check bool refcount_add_not_zero(int i, refcount_t *r)
  */
 static inline void refcount_add(int i, refcount_t *r)
 {
-	WARN_ONCE(!refcount_add_not_zero(i, r), "refcount_t: addition on 0; use-after-free.\n");
+	int old = atomic_fetch_add_relaxed(i, &r->refs);
+
+	WARN_ONCE(!old, "refcount_t: addition on 0; use-after-free.\n");
+	if (unlikely(old <= 0 || old + i <= 0)) {
+		refcount_set(r, REFCOUNT_SATURATED);
+		WARN_ONCE(old, "refcount_t: saturated; leaking memory.\n");
+	}
 }
 
 /**
@@ -166,23 +204,7 @@ static inline void refcount_add(int i, refcount_t *r)
  */
 static inline __must_check bool refcount_inc_not_zero(refcount_t *r)
 {
-	unsigned int new, val = atomic_read(&r->refs);
-
-	do {
-		new = val + 1;
-
-		if (!val)
-			return false;
-
-		if (unlikely(!new))
-			return true;
-
-	} while (!atomic_try_cmpxchg_relaxed(&r->refs, &val, new));
-
-	WARN_ONCE(new == REFCOUNT_SATURATED,
-		  "refcount_t: saturated; leaking memory.\n");
-
-	return true;
+	return refcount_add_not_zero(1, r);
 }
 
 /**
@@ -199,7 +221,7 @@ static inline __must_check bool refcount_inc_not_zero(refcount_t *r)
  */
 static inline void refcount_inc(refcount_t *r)
 {
-	WARN_ONCE(!refcount_inc_not_zero(r), "refcount_t: increment on 0; use-after-free.\n");
+	refcount_add(1, r);
 }
 
 /**
@@ -224,26 +246,19 @@ static inline void refcount_inc(refcount_t *r)
  */
 static inline __must_check bool refcount_sub_and_test(int i, refcount_t *r)
 {
-	unsigned int new, val = atomic_read(&r->refs);
-
-	do {
-		if (unlikely(val == REFCOUNT_SATURATED))
-			return false;
+	int old = atomic_fetch_sub_release(i, &r->refs);
 
-		new = val - i;
-		if (new > val) {
-			WARN_ONCE(new > val, "refcount_t: underflow; use-after-free.\n");
-			return false;
-		}
-
-	} while (!atomic_try_cmpxchg_release(&r->refs, &val, new));
-
-	if (!new) {
+	if (old == i) {
 		smp_acquire__after_ctrl_dep();
 		return true;
 	}
-	return false;
 
+	if (unlikely(old < 0 || old - i < 0)) {
+		refcount_set(r, REFCOUNT_SATURATED);
+		WARN_ONCE(1, "refcount_t: underflow; use-after-free.\n");
+	}
+
+	return false;
 }
 
 /**
@@ -276,9 +291,13 @@ static inline __must_check bool refcount_dec_and_test(refcount_t *r)
  */
 static inline void refcount_dec(refcount_t *r)
 {
-	WARN_ONCE(refcount_dec_and_test(r), "refcount_t: decrement hit 0; leaking memory.\n");
-}
+	int old = atomic_fetch_sub_release(1, &r->refs);
 
+	if (unlikely(old <= 1)) {
+		refcount_set(r, REFCOUNT_SATURATED);
+		WARN_ONCE(1, "refcount_t: decrement hit 0; leaking memory.\n");
+	}
+}
 #else /* CONFIG_REFCOUNT_FULL */
 
 #define REFCOUNT_MAX		INT_MAX
-- 
2.24.0.432.g9d3f5f5b63-goog
```

## [RESEND PATCH v4 06/10] lib/refcount: Move saturation warnings out of line

```patch
Having the refcount saturation and warnings inline bloats the text,
despite the fact that these paths should never be executed in normal
operation.

Move the refcount saturation and warnings out of line to reduce the
image size when refcount_t checking is enabled. Relative to an x86_64
defconfig, the sizes reported by bloat-o-meter are:

 # defconfig+REFCOUNT_FULL, inline saturation (i.e. before this patch)
 Total: Before=14762076, After=14915442, chg +1.04%

 # defconfig+REFCOUNT_FULL, out-of-line saturation (i.e. after this patch)
 Total: Before=14762076, After=14835497, chg +0.50%

A side-effect of this change is that we now only get one warning per
refcount saturation type, rather than one per problematic call-site.

Cc: Ingo Molnar <mingo@kernel.org>
Cc: Elena Reshetova <elena.reshetova@intel.com>
Cc: Peter Zijlstra <peterz@infradead.org>
Cc: Ard Biesheuvel <ard.biesheuvel@linaro.org>
Reviewed-by: Kees Cook <keescook@chromium.org>
Signed-off-by: Will Deacon <will@kernel.org>
---
 include/linux/refcount.h | 39 ++++++++++++++++++++-------------------
 lib/refcount.c           | 28 ++++++++++++++++++++++++++++
 2 files changed, 48 insertions(+), 19 deletions(-)

diff --git a/include/linux/refcount.h b/include/linux/refcount.h
index e3b218d669ce..1cd0a876a789 100644
--- a/include/linux/refcount.h
+++ b/include/linux/refcount.h
@@ -23,6 +23,16 @@ typedef struct refcount_struct {
 
 #define REFCOUNT_INIT(n)	{ .refs = ATOMIC_INIT(n), }
 
+enum refcount_saturation_type {
+	REFCOUNT_ADD_NOT_ZERO_OVF,
+	REFCOUNT_ADD_OVF,
+	REFCOUNT_ADD_UAF,
+	REFCOUNT_SUB_UAF,
+	REFCOUNT_DEC_LEAK,
+};
+
+void refcount_warn_saturate(refcount_t *r, enum refcount_saturation_type t);
+
 /**
  * refcount_set - set a refcount's value
  * @r: the refcount
@@ -154,10 +164,8 @@ static inline __must_check bool refcount_add_not_zero(int i, refcount_t *r)
 			break;
 	} while (!atomic_try_cmpxchg_relaxed(&r->refs, &old, old + i));
 
-	if (unlikely(old < 0 || old + i < 0)) {
-		refcount_set(r, REFCOUNT_SATURATED);
-		WARN_ONCE(1, "refcount_t: saturated; leaking memory.\n");
-	}
+	if (unlikely(old < 0 || old + i < 0))
+		refcount_warn_saturate(r, REFCOUNT_ADD_NOT_ZERO_OVF);
 
 	return old;
 }
@@ -182,11 +190,10 @@ static inline void refcount_add(int i, refcount_t *r)
 {
 	int old = atomic_fetch_add_relaxed(i, &r->refs);
 
-	WARN_ONCE(!old, "refcount_t: addition on 0; use-after-free.\n");
-	if (unlikely(old <= 0 || old + i <= 0)) {
-		refcount_set(r, REFCOUNT_SATURATED);
-		WARN_ONCE(old, "refcount_t: saturated; leaking memory.\n");
-	}
+	if (unlikely(!old))
+		refcount_warn_saturate(r, REFCOUNT_ADD_UAF);
+	else if (unlikely(old < 0 || old + i < 0))
+		refcount_warn_saturate(r, REFCOUNT_ADD_OVF);
 }
 
 /**
@@ -253,10 +260,8 @@ static inline __must_check bool refcount_sub_and_test(int i, refcount_t *r)
 		return true;
 	}
 
-	if (unlikely(old < 0 || old - i < 0)) {
-		refcount_set(r, REFCOUNT_SATURATED);
-		WARN_ONCE(1, "refcount_t: underflow; use-after-free.\n");
-	}
+	if (unlikely(old < 0 || old - i < 0))
+		refcount_warn_saturate(r, REFCOUNT_SUB_UAF);
 
 	return false;
 }
@@ -291,12 +296,8 @@ static inline __must_check bool refcount_dec_and_test(refcount_t *r)
  */
 static inline void refcount_dec(refcount_t *r)
 {
-	int old = atomic_fetch_sub_release(1, &r->refs);
-
-	if (unlikely(old <= 1)) {
-		refcount_set(r, REFCOUNT_SATURATED);
-		WARN_ONCE(1, "refcount_t: decrement hit 0; leaking memory.\n");
-	}
+	if (unlikely(atomic_fetch_sub_release(1, &r->refs) <= 1))
+		refcount_warn_saturate(r, REFCOUNT_DEC_LEAK);
 }
 #else /* CONFIG_REFCOUNT_FULL */
 
diff --git a/lib/refcount.c b/lib/refcount.c
index 3a534fbebdcc..8b7e249c0e10 100644
--- a/lib/refcount.c
+++ b/lib/refcount.c
@@ -8,6 +8,34 @@
 #include <linux/spinlock.h>
 #include <linux/bug.h>
 
+#define REFCOUNT_WARN(str)	WARN_ONCE(1, "refcount_t: " str ".\n")
+
+void refcount_warn_saturate(refcount_t *r, enum refcount_saturation_type t)
+{
+	refcount_set(r, REFCOUNT_SATURATED);
+
+	switch (t) {
+	case REFCOUNT_ADD_NOT_ZERO_OVF:
+		REFCOUNT_WARN("saturated; leaking memory");
+		break;
+	case REFCOUNT_ADD_OVF:
+		REFCOUNT_WARN("saturated; leaking memory");
+		break;
+	case REFCOUNT_ADD_UAF:
+		REFCOUNT_WARN("addition on 0; use-after-free");
+		break;
+	case REFCOUNT_SUB_UAF:
+		REFCOUNT_WARN("underflow; use-after-free");
+		break;
+	case REFCOUNT_DEC_LEAK:
+		REFCOUNT_WARN("decrement hit 0; leaking memory");
+		break;
+	default:
+		REFCOUNT_WARN("unknown saturation event!?");
+	}
+}
+EXPORT_SYMBOL(refcount_warn_saturate);
+
 /**
  * refcount_dec_if_one - decrement a refcount if it is 1
  * @r: the refcount
-- 
2.24.0.432.g9d3f5f5b63-goog
```

## [RESEND PATCH v4 07/10] lib/refcount: Consolidate REFCOUNT_{MAX,SATURATED} definitions

```patch
The definitions of REFCOUNT_MAX and REFCOUNT_SATURATED are the same,
regardless of CONFIG_REFCOUNT_FULL, so consolidate them into a single
pair of definitions.

Cc: Ingo Molnar <mingo@kernel.org>
Cc: Elena Reshetova <elena.reshetova@intel.com>
Cc: Peter Zijlstra <peterz@infradead.org>
Cc: Ard Biesheuvel <ard.biesheuvel@linaro.org>
Reviewed-by: Kees Cook <keescook@chromium.org>
Signed-off-by: Will Deacon <will@kernel.org>
---
 include/linux/refcount.h | 9 ++-------
 1 file changed, 2 insertions(+), 7 deletions(-)

diff --git a/include/linux/refcount.h b/include/linux/refcount.h
index 1cd0a876a789..757d4630115c 100644
--- a/include/linux/refcount.h
+++ b/include/linux/refcount.h
@@ -22,6 +22,8 @@ typedef struct refcount_struct {
 } refcount_t;
 
 #define REFCOUNT_INIT(n)	{ .refs = ATOMIC_INIT(n), }
+#define REFCOUNT_MAX		INT_MAX
+#define REFCOUNT_SATURATED	(INT_MIN / 2)
 
 enum refcount_saturation_type {
 	REFCOUNT_ADD_NOT_ZERO_OVF,
@@ -57,9 +59,6 @@ static inline unsigned int refcount_read(const refcount_t *r)
 #ifdef CONFIG_REFCOUNT_FULL
 #include <linux/bug.h>
 
-#define REFCOUNT_MAX		INT_MAX
-#define REFCOUNT_SATURATED	(INT_MIN / 2)
-
 /*
  * Variant of atomic_t specialized for reference counts.
  *
@@ -300,10 +299,6 @@ static inline void refcount_dec(refcount_t *r)
 		refcount_warn_saturate(r, REFCOUNT_DEC_LEAK);
 }
 #else /* CONFIG_REFCOUNT_FULL */
-
-#define REFCOUNT_MAX		INT_MAX
-#define REFCOUNT_SATURATED	(INT_MIN / 2)
-
 # ifdef CONFIG_ARCH_HAS_REFCOUNT
 #  include <asm/refcount.h>
 # else
-- 
2.24.0.432.g9d3f5f5b63-goog
```

## [RESEND PATCH v4 08/10] refcount: Consolidate implementations of refcount_t

```patch
The generic implementation of refcount_t should be good enough for
everybody, so remove ARCH_HAS_REFCOUNT and REFCOUNT_FULL entirely,
leaving the generic implementation enabled unconditionally.

Cc: Ingo Molnar <mingo@kernel.org>
Cc: Elena Reshetova <elena.reshetova@intel.com>
Cc: Peter Zijlstra <peterz@infradead.org>
Cc: Ard Biesheuvel <ard.biesheuvel@linaro.org>
Cc: Kees Cook <keescook@chromium.org>
Acked-by: Kees Cook <keescook@chromium.org>
Signed-off-by: Will Deacon <will@kernel.org>
---
 arch/Kconfig                       |  21 ----
 arch/arm/Kconfig                   |   1 -
 arch/arm64/Kconfig                 |   1 -
 arch/s390/configs/debug_defconfig  |   1 -
 arch/x86/Kconfig                   |   1 -
 arch/x86/include/asm/asm.h         |   6 --
 arch/x86/include/asm/refcount.h    | 126 -----------------------
 arch/x86/mm/extable.c              |  49 ---------
 drivers/gpu/drm/i915/Kconfig.debug |   1 -
 include/linux/refcount.h           | 158 +++++++++++------------------
 lib/refcount.c                     |   2 +-
 11 files changed, 59 insertions(+), 308 deletions(-)
 delete mode 100644 arch/x86/include/asm/refcount.h

diff --git a/arch/Kconfig b/arch/Kconfig
index 5f8a5d84dbbe..8bcc1c746142 100644
--- a/arch/Kconfig
+++ b/arch/Kconfig
@@ -892,27 +892,6 @@ config STRICT_MODULE_RWX
 config ARCH_HAS_PHYS_TO_DMA
 	bool
 
-config ARCH_HAS_REFCOUNT
-	bool
-	help
-	  An architecture selects this when it has implemented refcount_t
-	  using open coded assembly primitives that provide an optimized
-	  refcount_t implementation, possibly at the expense of some full
-	  refcount state checks of CONFIG_REFCOUNT_FULL=y.
-
-	  The refcount overflow check behavior, however, must be retained.
-	  Catching overflows is the primary security concern for protecting
-	  against bugs in reference counts.
-
-config REFCOUNT_FULL
-	bool "Perform full reference count validation at the expense of speed"
-	help
-	  Enabling this switches the refcounting infrastructure from a fast
-	  unchecked atomic_t implementation to a fully state checked
-	  implementation, which can be (slightly) slower but provides protections
-	  against various use-after-free conditions that can be used in
-	  security flaw exploits.
-
 config HAVE_ARCH_COMPILER_H
 	bool
 	help
diff --git a/arch/arm/Kconfig b/arch/arm/Kconfig
index 8a50efb559f3..0d3c5d7cceb7 100644
--- a/arch/arm/Kconfig
+++ b/arch/arm/Kconfig
@@ -117,7 +117,6 @@ config ARM
 	select OLD_SIGSUSPEND3
 	select PCI_SYSCALL if PCI
 	select PERF_USE_VMALLOC
-	select REFCOUNT_FULL
 	select RTC_LIB
 	select SYS_SUPPORTS_APM_EMULATION
 	# Above selects are sorted alphabetically; please add new ones
diff --git a/arch/arm64/Kconfig b/arch/arm64/Kconfig
index 41a9b4257b72..bc990d3abfe9 100644
--- a/arch/arm64/Kconfig
+++ b/arch/arm64/Kconfig
@@ -181,7 +181,6 @@ config ARM64
 	select PCI_SYSCALL if PCI
 	select POWER_RESET
 	select POWER_SUPPLY
-	select REFCOUNT_FULL
 	select SPARSE_IRQ
 	select SWIOTLB
 	select SYSCTL_EXCEPTION_TRACE
diff --git a/arch/s390/configs/debug_defconfig b/arch/s390/configs/debug_defconfig
index 38d64030aacf..2e60c80395ab 100644
--- a/arch/s390/configs/debug_defconfig
+++ b/arch/s390/configs/debug_defconfig
@@ -62,7 +62,6 @@ CONFIG_OPROFILE=m
 CONFIG_KPROBES=y
 CONFIG_JUMP_LABEL=y
 CONFIG_STATIC_KEYS_SELFTEST=y
-CONFIG_REFCOUNT_FULL=y
 CONFIG_LOCK_EVENT_COUNTS=y
 CONFIG_MODULES=y
 CONFIG_MODULE_FORCE_LOAD=y
diff --git a/arch/x86/Kconfig b/arch/x86/Kconfig
index d6e1faa28c58..fa6274f1e370 100644
--- a/arch/x86/Kconfig
+++ b/arch/x86/Kconfig
@@ -73,7 +73,6 @@ config X86
 	select ARCH_HAS_PMEM_API		if X86_64
 	select ARCH_HAS_PTE_DEVMAP		if X86_64
 	select ARCH_HAS_PTE_SPECIAL
-	select ARCH_HAS_REFCOUNT
 	select ARCH_HAS_UACCESS_FLUSHCACHE	if X86_64
 	select ARCH_HAS_UACCESS_MCSAFE		if X86_64 && X86_MCE
 	select ARCH_HAS_SET_MEMORY
diff --git a/arch/x86/include/asm/asm.h b/arch/x86/include/asm/asm.h
index 3ff577c0b102..5a0c14ebef70 100644
--- a/arch/x86/include/asm/asm.h
+++ b/arch/x86/include/asm/asm.h
@@ -139,9 +139,6 @@
 # define _ASM_EXTABLE_EX(from, to)				\
 	_ASM_EXTABLE_HANDLE(from, to, ex_handler_ext)
 
-# define _ASM_EXTABLE_REFCOUNT(from, to)			\
-	_ASM_EXTABLE_HANDLE(from, to, ex_handler_refcount)
-
 # define _ASM_NOKPROBE(entry)					\
 	.pushsection "_kprobe_blacklist","aw" ;			\
 	_ASM_ALIGN ;						\
@@ -170,9 +167,6 @@
 # define _ASM_EXTABLE_EX(from, to)				\
 	_ASM_EXTABLE_HANDLE(from, to, ex_handler_ext)
 
-# define _ASM_EXTABLE_REFCOUNT(from, to)			\
-	_ASM_EXTABLE_HANDLE(from, to, ex_handler_refcount)
-
 /* For C file, we already have NOKPROBE_SYMBOL macro */
 #endif
 
diff --git a/arch/x86/include/asm/refcount.h b/arch/x86/include/asm/refcount.h
deleted file mode 100644
index 232f856e0db0..000000000000
--- a/arch/x86/include/asm/refcount.h
+++ /dev/null
@@ -1,126 +0,0 @@
-#ifndef __ASM_X86_REFCOUNT_H
-#define __ASM_X86_REFCOUNT_H
-/*
- * x86-specific implementation of refcount_t. Based on PAX_REFCOUNT from
- * PaX/grsecurity.
- */
-#include <linux/refcount.h>
-#include <asm/bug.h>
-
-/*
- * This is the first portion of the refcount error handling, which lives in
- * .text.unlikely, and is jumped to from the CPU flag check (in the
- * following macros). This saves the refcount value location into CX for
- * the exception handler to use (in mm/extable.c), and then triggers the
- * central refcount exception. The fixup address for the exception points
- * back to the regular execution flow in .text.
- */
-#define _REFCOUNT_EXCEPTION				\
-	".pushsection .text..refcount\n"		\
-	"111:\tlea %[var], %%" _ASM_CX "\n"		\
-	"112:\t" ASM_UD2 "\n"				\
-	ASM_UNREACHABLE					\
-	".popsection\n"					\
-	"113:\n"					\
-	_ASM_EXTABLE_REFCOUNT(112b, 113b)
-
-/* Trigger refcount exception if refcount result is negative. */
-#define REFCOUNT_CHECK_LT_ZERO				\
-	"js 111f\n\t"					\
-	_REFCOUNT_EXCEPTION
-
-/* Trigger refcount exception if refcount result is zero or negative. */
-#define REFCOUNT_CHECK_LE_ZERO				\
-	"jz 111f\n\t"					\
-	REFCOUNT_CHECK_LT_ZERO
-
-/* Trigger refcount exception unconditionally. */
-#define REFCOUNT_ERROR					\
-	"jmp 111f\n\t"					\
-	_REFCOUNT_EXCEPTION
-
-static __always_inline void refcount_add(unsigned int i, refcount_t *r)
-{
-	asm volatile(LOCK_PREFIX "addl %1,%0\n\t"
-		REFCOUNT_CHECK_LT_ZERO
-		: [var] "+m" (r->refs.counter)
-		: "ir" (i)
-		: "cc", "cx");
-}
-
-static __always_inline void refcount_inc(refcount_t *r)
-{
-	asm volatile(LOCK_PREFIX "incl %0\n\t"
-		REFCOUNT_CHECK_LT_ZERO
-		: [var] "+m" (r->refs.counter)
-		: : "cc", "cx");
-}
-
-static __always_inline void refcount_dec(refcount_t *r)
-{
-	asm volatile(LOCK_PREFIX "decl %0\n\t"
-		REFCOUNT_CHECK_LE_ZERO
-		: [var] "+m" (r->refs.counter)
-		: : "cc", "cx");
-}
-
-static __always_inline __must_check
-bool refcount_sub_and_test(unsigned int i, refcount_t *r)
-{
-	bool ret = GEN_BINARY_SUFFIXED_RMWcc(LOCK_PREFIX "subl",
-					 REFCOUNT_CHECK_LT_ZERO,
-					 r->refs.counter, e, "er", i, "cx");
-
-	if (ret) {
-		smp_acquire__after_ctrl_dep();
-		return true;
-	}
-
-	return false;
-}
-
-static __always_inline __must_check bool refcount_dec_and_test(refcount_t *r)
-{
-	bool ret = GEN_UNARY_SUFFIXED_RMWcc(LOCK_PREFIX "decl",
-					 REFCOUNT_CHECK_LT_ZERO,
-					 r->refs.counter, e, "cx");
-
-	if (ret) {
-		smp_acquire__after_ctrl_dep();
-		return true;
-	}
-
-	return false;
-}
-
-static __always_inline __must_check
-bool refcount_add_not_zero(unsigned int i, refcount_t *r)
-{
-	int c, result;
-
-	c = atomic_read(&(r->refs));
-	do {
-		if (unlikely(c == 0))
-			return false;
-
-		result = c + i;
-
-		/* Did we try to increment from/to an undesirable state? */
-		if (unlikely(c < 0 || c == INT_MAX || result < c)) {
-			asm volatile(REFCOUNT_ERROR
-				     : : [var] "m" (r->refs.counter)
-				     : "cc", "cx");
-			break;
-		}
-
-	} while (!atomic_try_cmpxchg(&(r->refs), &c, result));
-
-	return c != 0;
-}
-
-static __always_inline __must_check bool refcount_inc_not_zero(refcount_t *r)
-{
-	return refcount_add_not_zero(1, r);
-}
-
-#endif
diff --git a/arch/x86/mm/extable.c b/arch/x86/mm/extable.c
index 4d75bc656f97..30bb0bd3b1b8 100644
--- a/arch/x86/mm/extable.c
+++ b/arch/x86/mm/extable.c
@@ -44,55 +44,6 @@ __visible bool ex_handler_fault(const struct exception_table_entry *fixup,
 }
 EXPORT_SYMBOL_GPL(ex_handler_fault);
 
-/*
- * Handler for UD0 exception following a failed test against the
- * result of a refcount inc/dec/add/sub.
- */
-__visible bool ex_handler_refcount(const struct exception_table_entry *fixup,
-				   struct pt_regs *regs, int trapnr,
-				   unsigned long error_code,
-				   unsigned long fault_addr)
-{
-	/* First unconditionally saturate the refcount. */
-	*(int *)regs->cx = INT_MIN / 2;
-
-	/*
-	 * Strictly speaking, this reports the fixup destination, not
-	 * the fault location, and not the actually overflowing
-	 * instruction, which is the instruction before the "js", but
-	 * since that instruction could be a variety of lengths, just
-	 * report the location after the overflow, which should be close
-	 * enough for finding the overflow, as it's at least back in
-	 * the function, having returned from .text.unlikely.
-	 */
-	regs->ip = ex_fixup_addr(fixup);
-
-	/*
-	 * This function has been called because either a negative refcount
-	 * value was seen by any of the refcount functions, or a zero
-	 * refcount value was seen by refcount_dec().
-	 *
-	 * If we crossed from INT_MAX to INT_MIN, OF (Overflow Flag: result
-	 * wrapped around) will be set. Additionally, seeing the refcount
-	 * reach 0 will set ZF (Zero Flag: result was zero). In each of
-	 * these cases we want a report, since it's a boundary condition.
-	 * The SF case is not reported since it indicates post-boundary
-	 * manipulations below zero or above INT_MAX. And if none of the
-	 * flags are set, something has gone very wrong, so report it.
-	 */
-	if (regs->flags & (X86_EFLAGS_OF | X86_EFLAGS_ZF)) {
-		bool zero = regs->flags & X86_EFLAGS_ZF;
-
-		refcount_error_report(regs, zero ? "hit zero" : "overflow");
-	} else if ((regs->flags & X86_EFLAGS_SF) == 0) {
-		/* Report if none of OF, ZF, nor SF are set. */
-		refcount_error_report(regs, "unexpected saturation");
-	}
-
-	return true;
-}
-EXPORT_SYMBOL(ex_handler_refcount);
-
 /*
  * Handler for when we fail to restore a task's FPU state.  We should never get
  * here because the FPU state of a task using the FPU (task->thread.fpu.state)
diff --git a/drivers/gpu/drm/i915/Kconfig.debug b/drivers/gpu/drm/i915/Kconfig.debug
index 00786a142ff0..1400fce39c58 100644
--- a/drivers/gpu/drm/i915/Kconfig.debug
+++ b/drivers/gpu/drm/i915/Kconfig.debug
@@ -22,7 +22,6 @@ config DRM_I915_DEBUG
         depends on DRM_I915
         select DEBUG_FS
         select PREEMPT_COUNT
-        select REFCOUNT_FULL
         select I2C_CHARDEV
         select STACKDEPOT
         select DRM_DP_AUX_CHARDEV
diff --git a/include/linux/refcount.h b/include/linux/refcount.h
index 757d4630115c..0ac50cf62d06 100644
--- a/include/linux/refcount.h
+++ b/include/linux/refcount.h
@@ -1,64 +1,4 @@
 /* SPDX-License-Identifier: GPL-2.0 */
-#ifndef _LINUX_REFCOUNT_H
-#define _LINUX_REFCOUNT_H
-
-#include <linux/atomic.h>
-#include <linux/compiler.h>
-#include <linux/limits.h>
-#include <linux/spinlock_types.h>
-
-struct mutex;
-
-/**
- * struct refcount_t - variant of atomic_t specialized for reference counts
- * @refs: atomic_t counter field
- *
- * The counter saturates at REFCOUNT_SATURATED and will not move once
- * there. This avoids wrapping the counter and causing 'spurious'
- * use-after-free bugs.
- */
-typedef struct refcount_struct {
-	atomic_t refs;
-} refcount_t;
-
-#define REFCOUNT_INIT(n)	{ .refs = ATOMIC_INIT(n), }
-#define REFCOUNT_MAX		INT_MAX
-#define REFCOUNT_SATURATED	(INT_MIN / 2)
-
-enum refcount_saturation_type {
-	REFCOUNT_ADD_NOT_ZERO_OVF,
-	REFCOUNT_ADD_OVF,
-	REFCOUNT_ADD_UAF,
-	REFCOUNT_SUB_UAF,
-	REFCOUNT_DEC_LEAK,
-};
-
-void refcount_warn_saturate(refcount_t *r, enum refcount_saturation_type t);
-
-/**
- * refcount_set - set a refcount's value
- * @r: the refcount
- * @n: value to which the refcount will be set
- */
-static inline void refcount_set(refcount_t *r, int n)
-{
-	atomic_set(&r->refs, n);
-}
-
-/**
- * refcount_read - get a refcount's value
- * @r: the refcount
- *
- * Return: the refcount's value
- */
-static inline unsigned int refcount_read(const refcount_t *r)
-{
-	return atomic_read(&r->refs);
-}
-
-#ifdef CONFIG_REFCOUNT_FULL
-#include <linux/bug.h>
-
 /*
  * Variant of atomic_t specialized for reference counts.
  *
@@ -136,6 +76,64 @@ static inline unsigned int refcount_read(const refcount_t *r)
  *
  */
 
+#ifndef _LINUX_REFCOUNT_H
+#define _LINUX_REFCOUNT_H
+
+#include <linux/atomic.h>
+#include <linux/bug.h>
+#include <linux/compiler.h>
+#include <linux/limits.h>
+#include <linux/spinlock_types.h>
+
+struct mutex;
+
+/**
+ * struct refcount_t - variant of atomic_t specialized for reference counts
+ * @refs: atomic_t counter field
+ *
+ * The counter saturates at REFCOUNT_SATURATED and will not move once
+ * there. This avoids wrapping the counter and causing 'spurious'
+ * use-after-free bugs.
+ */
+typedef struct refcount_struct {
+	atomic_t refs;
+} refcount_t;
+
+#define REFCOUNT_INIT(n)	{ .refs = ATOMIC_INIT(n), }
+#define REFCOUNT_MAX		INT_MAX
+#define REFCOUNT_SATURATED	(INT_MIN / 2)
+
+enum refcount_saturation_type {
+	REFCOUNT_ADD_NOT_ZERO_OVF,
+	REFCOUNT_ADD_OVF,
+	REFCOUNT_ADD_UAF,
+	REFCOUNT_SUB_UAF,
+	REFCOUNT_DEC_LEAK,
+};
+
+void refcount_warn_saturate(refcount_t *r, enum refcount_saturation_type t);
+
+/**
+ * refcount_set - set a refcount's value
+ * @r: the refcount
+ * @n: value to which the refcount will be set
+ */
+static inline void refcount_set(refcount_t *r, int n)
+{
+	atomic_set(&r->refs, n);
+}
+
+/**
+ * refcount_read - get a refcount's value
+ * @r: the refcount
+ *
+ * Return: the refcount's value
+ */
+static inline unsigned int refcount_read(const refcount_t *r)
+{
+	return atomic_read(&r->refs);
+}
+
 /**
  * refcount_add_not_zero - add a value to a refcount unless it is 0
  * @i: the value to add to the refcount
@@ -298,46 +296,6 @@ static inline void refcount_dec(refcount_t *r)
 	if (unlikely(atomic_fetch_sub_release(1, &r->refs) <= 1))
 		refcount_warn_saturate(r, REFCOUNT_DEC_LEAK);
 }
-#else /* CONFIG_REFCOUNT_FULL */
-# ifdef CONFIG_ARCH_HAS_REFCOUNT
-#  include <asm/refcount.h>
-# else
-static inline __must_check bool refcount_add_not_zero(int i, refcount_t *r)
-{
-	return atomic_add_unless(&r->refs, i, 0);
-}
-
-static inline void refcount_add(int i, refcount_t *r)
-{
-	atomic_add(i, &r->refs);
-}
-
-static inline __must_check bool refcount_inc_not_zero(refcount_t *r)
-{
-	return atomic_add_unless(&r->refs, 1, 0);
-}
-
-static inline void refcount_inc(refcount_t *r)
-{
-	atomic_inc(&r->refs);
-}
-
-static inline __must_check bool refcount_sub_and_test(int i, refcount_t *r)
-{
-	return atomic_sub_and_test(i, &r->refs);
-}
-
-static inline __must_check bool refcount_dec_and_test(refcount_t *r)
-{
-	return atomic_dec_and_test(&r->refs);
-}
-
-static inline void refcount_dec(refcount_t *r)
-{
-	atomic_dec(&r->refs);
-}
-# endif /* !CONFIG_ARCH_HAS_REFCOUNT */
-#endif /* !CONFIG_REFCOUNT_FULL */
 
 extern __must_check bool refcount_dec_if_one(refcount_t *r);
 extern __must_check bool refcount_dec_not_one(refcount_t *r);
diff --git a/lib/refcount.c b/lib/refcount.c
index 8b7e249c0e10..ebac8b7d15a7 100644
--- a/lib/refcount.c
+++ b/lib/refcount.c
@@ -1,6 +1,6 @@
 // SPDX-License-Identifier: GPL-2.0
 /*
- * Out-of-line refcount functions common to all refcount implementations.
+ * Out-of-line refcount functions.
  */
 
 #include <linux/mutex.h>
-- 
2.24.0.432.g9d3f5f5b63-goog
```

## [RESEND PATCH v4 09/10] lib/refcount: Remove unused 'refcount_error_report()' function

```patch
'refcount_error_report()' has no callers. Remove it.

Cc: Ingo Molnar <mingo@kernel.org>
Cc: Elena Reshetova <elena.reshetova@intel.com>
Cc: Peter Zijlstra <peterz@infradead.org>
Cc: Ard Biesheuvel <ard.biesheuvel@linaro.org>
Cc: Kees Cook <keescook@chromium.org>
Acked-by: Kees Cook <keescook@chromium.org>
Signed-off-by: Will Deacon <will@kernel.org>
---
 include/linux/kernel.h |  7 -------
 kernel/panic.c         | 11 -----------
 2 files changed, 18 deletions(-)

diff --git a/include/linux/kernel.h b/include/linux/kernel.h
index d83d403dac2e..09f759228e3f 100644
--- a/include/linux/kernel.h
+++ b/include/linux/kernel.h
@@ -328,13 +328,6 @@ extern int oops_may_print(void);
 void do_exit(long error_code) __noreturn;
 void complete_and_exit(struct completion *, long) __noreturn;
 
-#ifdef CONFIG_ARCH_HAS_REFCOUNT
-void refcount_error_report(struct pt_regs *regs, const char *err);
-#else
-static inline void refcount_error_report(struct pt_regs *regs, const char *err)
-{ }
-#endif
-
 /* Internal, do not use. */
 int __must_check _kstrtoul(const char *s, unsigned int base, unsigned long *res);
 int __must_check _kstrtol(const char *s, unsigned int base, long *res);
diff --git a/kernel/panic.c b/kernel/panic.c
index 47e8ebccc22b..10d05fd4f9c3 100644
--- a/kernel/panic.c
+++ b/kernel/panic.c
@@ -670,17 +670,6 @@ EXPORT_SYMBOL(__stack_chk_fail);
 
 #endif
 
-#ifdef CONFIG_ARCH_HAS_REFCOUNT
-void refcount_error_report(struct pt_regs *regs, const char *err)
-{
-	WARN_RATELIMIT(1, "refcount_t %s at %pB in %s[%d], uid/euid: %u/%u\n",
-		err, (void *)instruction_pointer(regs),
-		current->comm, task_pid_nr(current),
-		from_kuid_munged(&init_user_ns, current_uid()),
-		from_kuid_munged(&init_user_ns, current_euid()));
-}
-#endif
-
 core_param(panic, panic_timeout, int, 0644);
 core_param(panic_print, panic_print, ulong, 0644);
 core_param(pause_on_oops, pause_on_oops, int, 0644);
-- 
2.24.0.432.g9d3f5f5b63-goog
```

## [RESEND PATCH v4 10/10] drivers/lkdtm: Remove references to CONFIG_REFCOUNT_FULL

```patch
CONFIG_REFCOUNT_FULL no longer exists, so remove all references to it.

Cc: Ingo Molnar <mingo@kernel.org>
Cc: Elena Reshetova <elena.reshetova@intel.com>
Cc: Peter Zijlstra <peterz@infradead.org>
Cc: Ard Biesheuvel <ard.biesheuvel@linaro.org>
Cc: Kees Cook <keescook@chromium.org>
Acked-by: Kees Cook <keescook@chromium.org>
Signed-off-by: Will Deacon <will@kernel.org>
---
 drivers/misc/lkdtm/refcount.c | 3 +--
 1 file changed, 1 insertion(+), 2 deletions(-)

diff --git a/drivers/misc/lkdtm/refcount.c b/drivers/misc/lkdtm/refcount.c
index abf3b7c1f686..de7c5ab528d9 100644
--- a/drivers/misc/lkdtm/refcount.c
+++ b/drivers/misc/lkdtm/refcount.c
@@ -119,7 +119,7 @@ void lkdtm_REFCOUNT_DEC_ZERO(void)
 static void check_negative(refcount_t *ref, int start)
 {
 	/*
-	 * CONFIG_REFCOUNT_FULL refuses to move a refcount at all on an
+	 * refcount_t refuses to move a refcount at all on an
 	 * over-sub, so we have to track our starting position instead of
 	 * looking only at zero-pinning.
 	 */
@@ -202,7 +202,6 @@ static void check_from_zero(refcount_t *ref)
 
 /*
  * A refcount_inc() from zero should pin to zero or saturate and may WARN.
- * Only CONFIG_REFCOUNT_FULL provides this protection currently.
  */
 void lkdtm_REFCOUNT_INC_ZERO(void)
 {
-- 
2.24.0.432.g9d3f5f5b63-goog
```