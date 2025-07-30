#include <linux/futex.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>

#define THREAD_NUM 32 // Number of threads

// Shared counter
unsigned long counter = 0;

//  Shared mutex lock
unsigned long mutex = 0;

// Futex syscall wrapper
static int futex(void *addr1, int op, int val, const struct timespec *timeout,
                 void *addr2, int val3) 
{
    return syscall(SYS_futex, addr1, op, val, timeout, addr2, val3);
}

// LL/SC-based CAS using ARM64 assembly (64-bit values)
bool cas(unsigned long *addr, unsigned long expected, unsigned long desired) 
{
    unsigned long tmp;       // To hold loaded value or return value
    unsigned int status;     // Must be 32-bit for STLXR!

    __asm__ volatile (
        "1:\n"
        "ldaxr  %0, [%2]        \n"  // tmp = *addr (load-acquire)
        "cmp    %0, %3          \n"  // Compare with expected
        "b.ne   2f              \n"  // If not equal, fail, goto lable 2
        "stlxr  %w1, %4, [%2]   \n"  // Try to store desired -> *addr (store-release)
        "cbnz   %w1, 1b         \n"  // Retry if store failed, goto lable 1
        "mov    %0, #1          \n"  // Success
        "b      3f              \n"
        "2:\n"
        "mov    %0, #0          \n"  // Fail
        "3:\n"
        : "=&r"(tmp), "=&r"(status)
        : "r"(addr), "r"(expected), "r"(desired)
        : "cc", "memory"
    );

    return tmp;
}

// Mutex lock using LL/SC CAS and futex
static int mutex_lock(unsigned long *mutex) 
{
    while (!cas(mutex, 0, 1)) {
        futex(mutex, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, 1, NULL, NULL, 0);
    }
    return 0;
}

// Mutex unlock
static int mutex_unlock(unsigned long *mutex) 
{
    *mutex = 0;
    futex(mutex, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, 1, NULL, NULL, 0);
    return 0;
}

// Thread function
void *thread_func(void *arg) {
    for (int i = 0; i < 1000; i++) {
        mutex_lock(&mutex);
        counter++; // critical section
        mutex_unlock(&mutex);
    }
    return NULL;
}

int main(int argc, char **argv) 
{
    pthread_t threads[THREAD_NUM];

    for (int i = 0; i < THREAD_NUM; i++) {
        if (pthread_create(&threads[i], NULL, thread_func, NULL) != 0) {
            perror("pthread_create failed");
            return 1;
        }
    }

    for (int i = 0; i < THREAD_NUM; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Final counter value: %lu (expected: %d)\n", counter, THREAD_NUM * 1000);
    return 0;
}
