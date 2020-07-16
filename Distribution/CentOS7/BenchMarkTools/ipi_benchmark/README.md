
## ipi_benchmark —— Performance test for IPI on SMP machines



ipi_benchmark.c
```
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ktime.h>

#define NTIMES 100000

#define POKE_ANY        0
#define DRY_RUN         1
#define POKE_SELF       2
#define POKE_ALL        3

static void __init handle_ipi(void *t)
{
        ktime_t *time = (ktime_t *) t;

        if (time)
                *time = ktime_get() - *time;
}

static ktime_t __init send_ipi(int flags)
{
        ktime_t time;
        unsigned int cpu = get_cpu();

        switch (flags) {
        case POKE_ALL:
                /* If broadcasting, don't force all CPUs to update time. */
                smp_call_function_many(cpu_online_mask, handle_ipi, NULL, 1);
                /* Fall thru */
        case DRY_RUN:
                /* Do everything except actually sending IPI. */
                time = 0;
                break;
        case POKE_ANY:
                cpu = cpumask_any_but(cpu_online_mask, cpu);
                if (cpu >= nr_cpu_ids) {
                        time = -ENOENT;
                        break;
                }
                /* Fall thru */
        case POKE_SELF:
                time = ktime_get();
                smp_call_function_single(cpu, handle_ipi, &time, 1);
                break;
        default:
                time = -EINVAL;
        }

        put_cpu();
        return time;
}

static int __init __bench_ipi(unsigned long i, ktime_t *time, int flags)
{
        ktime_t t;

        *time = 0;
        while (i--) {
                t = send_ipi(flags);
                if ((int) t < 0)
                        return (int) t;

                *time += t;
        }

        return 0;
}

static int __init bench_ipi(unsigned long times, int flags,
                                ktime_t *ipi, ktime_t *total)
{
        int ret;

        *total = ktime_get();
        ret = __bench_ipi(times, ipi, flags);
        if (unlikely(ret))
                return ret;

        *total = ktime_get() - *total;

        return 0;
}

static int __init init_bench_ipi(void)
{
        ktime_t ipi, total;
        int ret;

        ret = bench_ipi(NTIMES, DRY_RUN, &ipi, &total);
        if (ret)
                pr_err("Dry-run FAILED: %d\n", ret);
        else
                pr_err("Dry-run:       %18llu, %18llu ns\n", ipi, total);

        ret = bench_ipi(NTIMES, POKE_SELF, &ipi, &total);
        if (ret)
                pr_err("Self-IPI FAILED: %d\n", ret);
        else
                pr_err("Self-IPI:      %18llu, %18llu ns\n", ipi, total);

        ret = bench_ipi(NTIMES, POKE_ANY, &ipi, &total);
        if (ret)
                pr_err("Normal IPI FAILED: %d\n", ret);
        else
                pr_err("Normal IPI:    %18llu, %18llu ns\n", ipi, total);

        ret = bench_ipi(NTIMES, POKE_ALL, &ipi, &total);
        if (ret)
                pr_err("Broadcast IPI FAILED: %d\n", ret);
        else
                pr_err("Broadcast IPI: %18llu, %18llu ns\n", ipi, total);

        /* Return error to avoid annoying rmmod. */
        return -EINVAL;
}
module_init(init_bench_ipi);

MODULE_LICENSE("GPL");
```

Makefile:
```
obj-m += ipi_benchmark.o

all:
        make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
        make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
```

dmesg:
```
[1199572.545289] Dry-run:                        0,             364020 ns
[1199572.569177] Self-IPI:                 9996005,           17371838 ns
[1199572.754743] Normal IPI:             124278996,          179066426 ns
[1199575.330621] Broadcast IPI:                  0,         2569376682 ns
```