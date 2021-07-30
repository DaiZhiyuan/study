## 1. Code

vtop.c
```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>

#define BUFFER "Hello, World!"
#ifdef __aarch64__
#define PAGE_SHIFT 16
#endif

#ifdef __x86_64__
#define PAGE_SHIFT 12
#endif

#define PAGEMAP_LENGTH 8

unsigned long long vtop(void *addr);

int main(void) {
    size_t buf_size = strlen(BUFFER) + 1;
    void *buffer = malloc(buf_size);
    if (buffer == NULL) {
       fprintf(stderr, "Failed to allocate memory for buffer\n");
       exit(1);
    }

    if (mlock(buffer, buf_size) == -1) {
       fprintf(stderr, "Failed to lock page in memory: %s\n", strerror(errno));
       exit(1);
    }

    strncpy(buffer, BUFFER, strlen(BUFFER));

    printf("PID: %u\n", getpid());
    printf("Buffer: %s\n", buffer);
    printf("VA: %p\n", buffer);
    printf("PA: %p\n", vtop(buffer));
    puts("intput enter to exit...");
    getchar();

    free(buffer);
    return 0;
}

unsigned long long vtop(void *addr)
{
    unsigned long long page_frame_number = 0;
    unsigned long long distance_from_page_boundary = 0;

    FILE *pagemap = fopen("/proc/self/pagemap", "rb");

    unsigned long long offset = (unsigned long)addr / getpagesize() * PAGEMAP_LENGTH;
    if(fseek(pagemap, (unsigned long long )offset, SEEK_SET) != 0) {
        fprintf(stderr, "Failed to seek pagemap to proper location\n");
        exit(1);
    }

    fread(&page_frame_number, 1, PAGEMAP_LENGTH-1, pagemap);
    page_frame_number &= 0x7FFFFFFFFFFFFF;
    distance_from_page_boundary = (unsigned long long)addr % getpagesize();
    fclose(pagemap);

    unsigned long long pa = ((page_frame_number << PAGE_SHIFT) + distance_from_page_boundary);
    return pa;
}
```

## 2. Test

Terminal1：
```sh
#> ./vtop
PID: 1682964
Buffer: Hello, World!
VA: 0x11600010
PA: 0x402d5a30010
intput enter to exit...
```

Terminal2:
```sh
crash> vtop -u -c 1682964 0x11600010
VIRTUAL     PHYSICAL
11600010    402d5a30010

PAGE DIRECTORY: ffff8281e7cc0000
   PGD: ffff8281e7cc0000 => 482dd030003
   PMD: ffff8482dd030000 => 482c8b10003
   PTE: ffff8482c8b18b00 => e80402d5a30f53
  PAGE: 402d5a30000

     PTE         PHYSICAL    FLAGS
e80402d5a30f53  402d5a30000  (VALID|USER|SHARED|AF|NG|PXN|UXN|DIRTY)

      VMA           START       END     FLAGS FILE
ffff80019e6cd4c0   11600000   11610000 102073

      PAGE         PHYSICAL      MAPPING       INDEX CNT FLAGS
ffff7fe100b568c0 402d5a30000                0        0  0 0
```

### 3. refs
- https://www.kernel.org/doc/Documentation/vm/pagemap.txt

