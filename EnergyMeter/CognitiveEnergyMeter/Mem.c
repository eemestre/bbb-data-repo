/*
 * Mem.c
 *
 *  Created on: Feb 13, 2017
 *      Author: fernando
 */

#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define RAM_SIZE(RAM_BYTES) (RAM_BYTES / 4)
#define MAP_SIZE(RAM_BYTES) (RAM_BYTES + 4096UL)
#define PAGE_MASK (4096UL - 1)        /* BeagleBone Black page size: 4096 */
#define MMAP1_LOC   "/sys/class/uio/uio0/maps/map1/"

// Function to load the shared RAM memory information from sysfs
int getMemInfo(unsigned int *addr, unsigned int *size)
{
    FILE* pfile;

    // Read shared RAM address
    pfile = fopen(MMAP1_LOC "addr", "rt");
    fscanf(pfile, "%x", addr);
    fclose(pfile);

    // Read shared RAM size
    pfile = fopen(MMAP1_LOC "size", "rt");
    fscanf(pfile, "%x", size);
    fclose(pfile);

    return(0);
}



int Allocate_RAM(void **mem_map, void **ram_addr, unsigned int * addr, const int buffer_size)
{
    int fd;
    unsigned int size;

    /***** SHARED RAM SETUP *****/
    printf("Allocating RAM buffer... ");
    fflush(stdout);

    /* Get shared RAM information */
    getMemInfo(addr, &size);

    if (size < buffer_size) {
        printf("error.\n");
        fprintf(stderr, "External RAM pool must be at least %d bytes.\n", buffer_size);
        goto error;
    }

    /* Get access to device memory */
    if ((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
        printf("error.\n");
        perror("Failed to open memory!");
        goto error;
    }

    /* Map shared RAM */
    *mem_map = mmap(0, buffer_size, PROT_READ, MAP_SHARED, fd, (*addr) & ~PAGE_MASK);

    /* Close file descriptor (not needed after memory mapping) */
    close(fd);

    if (*mem_map == (void *) -1) {
        printf("error.\n");
        perror("Failed to map base address");
        goto error;
    }

    /* Memory mapping must be page aligned */
    *ram_addr = *mem_map + ((*addr) & PAGE_MASK);

    printf("RAM OK!\n");
    return 0;

    error:
    return -1;
}

int Release_RAM(void *mem_map, const int buffer_size)
{
    /* SHARED RAM CLEAN UP */
    if (munmap(mem_map, RAM_SIZE(buffer_size)) == -1) {
        perror("Failed to unmap memory");
        return -1;
    }

    return 0;
}
