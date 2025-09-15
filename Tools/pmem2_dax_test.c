#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <libpmem2.h>

#define PMEM_PATH "/dev/dax0.0"

/*
Program Output:
    Mapped /dev/dax0.0 to address 0x7ff099e00000 with size 1054867456 bytes.
    Successfully wrote data and persisted to PMEM.
    Data read from PMEM: "Hello from libpmem2!"
    Cleaned up resources.
*/

int main(int argc, char *argv[])
{
    int fd;
    struct pmem2_config *cfg = NULL;
    struct pmem2_map *map = NULL;
    struct pmem2_source *src = NULL;
    char *pmem_addr = NULL;
    size_t pmem_size;
    pmem2_persist_fn persist;

    // Use a fixed path for simplicity, as in the previous examples
    // Or uncomment the argv check if you want to use it as a command-line tool
    /*
    if (argc != 2) {
        fprintf(stderr, "usage: %s file\n", argv[0]);
        exit(1);
    }
    */

    // 1. Open the DAX device file
    if ((fd = open(PMEM_PATH, O_RDWR)) < 0) {
        perror("open");
        exit(1);
    }

    // 2. Create a pmem2_source object from the file descriptor
    if (pmem2_source_from_fd(&src, fd)) {
        pmem2_perror("pmem2_source_from_fd");
        close(fd);
        exit(1);
    }

    // 3. Create a pmem2_config object
    if (pmem2_config_new(&cfg)) {
        pmem2_perror("pmem2_config_new");
        pmem2_source_delete(&src);
        close(fd);
        exit(1);
    }

    // 4. Set the required store granularity
    // This is the correct function call to set granularity
    if (pmem2_config_set_required_store_granularity(cfg, PMEM2_GRANULARITY_PAGE)) {
        pmem2_perror("pmem2_config_set_required_store_granularity");
        pmem2_config_delete(&cfg);
        pmem2_source_delete(&src);
        close(fd);
        exit(1);
    }

    // 5. Perform the memory mapping
    if (pmem2_map_new(&map, cfg, src)) {
        pmem2_perror("pmem2_map_new");
        pmem2_config_delete(&cfg);
        pmem2_source_delete(&src);
        close(fd);
        exit(1);
    }

    // 6. Get the mapped address and size
    pmem_addr = pmem2_map_get_address(map);
    pmem_size = pmem2_map_get_size(map);

    printf("Mapped %s to address %p with size %zu bytes.\n", PMEM_PATH, pmem_addr, pmem_size);

    // 7. Write data and get the persistence function
    const char *data_to_write = "Hello from libpmem2!";
    strcpy(pmem_addr, data_to_write);

    // Get the hardware-specific persist function from the map
    persist = pmem2_get_persist_fn(map);

    // Persist the data to the non-volatile memory
    persist(pmem_addr, pmem_size);

    printf("Successfully wrote data and persisted to PMEM.\n");
    printf("Data read from PMEM: \"%s\"\n", pmem_addr);

    // 8. Clean up resources
    pmem2_map_delete(&map);
    pmem2_source_delete(&src);
    pmem2_config_delete(&cfg);
    close(fd);

    printf("Cleaned up resources.\n");

    return 0;
}
