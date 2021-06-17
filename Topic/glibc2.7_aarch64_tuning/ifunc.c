#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

extern char *MemoryCopy(char *dst, const char *src, size_t n)  __attribute__((ifunc ("resolve")));

char *__memorycopy_c(char *dst, const char *src, size_t n)
{
    printf("memcpy(): C implementation\n");
}

/*
  Copies are split into 3 main cases:
    1. Small copies of up to 16 bytes.
    2. Medium copies of 17..96 bytes which are fully unrolled.
    3. Large copies of more than 96 bytes.

    Large copies align the sourceto a quad word and use an unrolled loop
    processing 64 bytes per iteration.
*/
char *__memorycopy_aarch64(char *dst, const char *src, size_t n)
{
    printf("memcpy(): armv8-a implementation\n");
}

/*
  Copies are split into 3 main cases:
    1. Small copies of up to 32 bytes.
    2. Medium copies of 32..128 bytes which are fully unrolled.
    3. Large copies of more than 128 bytes.

    Large copies align the sourceto a quad word and use an unrolled loop
    processing 64 bytes per iteration.


FALKOR-SPECIFIC DESIGN:

  The smallest copies (32 bytes or less) focus on optimal pipeline usage,
  which is why the redundant copies of 0-3 bytes have been replaced with
  conditionals, since the former would unnecessarily break across multiple
  issue groups.  The medium copy group has been enlarged to 128 bytes since
  bumping up the small copies up to 32 bytes allows us to do that without
  cost and also allows us to reduce the size of the prep code before loop64.

  All copies are done only via two registers r6 and r7.  This is to ensure
  that all loads hit a single hardware prefetcher which can get correctly
  trained to prefetch a single stream.

  The non-temporal stores help optimize cache utilization.
*/
char *__memorycopy_falkor(char *dst, const char *src, size_t n)
{
    printf("memcpy(): falkor implementation\n");
}

static void *resolve(void)
{
    srand((unsigned)(time(NULL)));
    int opt = 1 + (int)(3.0 * rand() / (RAND_MAX + 1.0));

    switch(opt) {
        case 1: return __memorycopy_c;
        case 2: return __memorycopy_aarch64;
        case 3: return __memorycopy_falkor;
    }
}


int main(int argc, char **argv)
{
    char *buffer[512];

    MemoryCopy(buffer, "glibc", 5);

    return 0;
}
