#include <linux/futex.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <stdio.h>

#define THREAD_NUM 32
typedef unsigned long mutex_t;

static int futex(void *addr1, int op, int val, const struct timespec *timeout,
                 void *addr2, int val3)
{
    return syscall(SYS_futex, addr1, op, val, timeout, addr2, val3);
}

static void mutex_lock(mutex_t *lock)
{
    unsigned long expected_value = 0; // unlocked

    while (!__atomic_compare_exchange_n(&lock, &expected_value, 1,
                                /*weak=*/ 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST))
        {
        futex(lock, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 1, NULL, NULL, 0);
        expected_value = 0; // Reset expected value for next attempt
    }
}

static void mutex_unlock(mutex_t *lock)
{
    *lock = 0; // Unlock by setting to 0
    futex(lock, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1, NULL, NULL, 0); // Wake one waiter
}

// Shared counter for the critical section
unsigned long counter = 0;

// Mutex state: 0 = unlocked, 1 = locked
mutex_t lock = 0;

// Thread function
void *thread_func(void *arg) {
    for (int i = 0; i < 1000; i++) {
        mutex_lock(&lock);
        counter++; // Critical section
        mutex_unlock(&lock);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    pthread_t threads[THREAD_NUM];

    // Create threads
    for (int i = 0; i < THREAD_NUM; i++) {
        if (pthread_create(&threads[i], NULL, thread_func, NULL) != 0) {
            perror("pthread_create failed");
            return 1;
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < THREAD_NUM; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Final counter value: %lu (expected: %d)\n", counter, THREAD_NUM * 1000);
    return 0;
}
