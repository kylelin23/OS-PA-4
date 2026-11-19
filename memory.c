#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "memsim.h"

typedef struct {
    int backing_fd;
    signed char *frames;
    int num_frames;
} memory_t;

static memory_t mem = { -1, NULL, 0 };

static void backing_read_page(int page, signed char *buf)
{
    if (lseek(mem.backing_fd, (off_t)page * PAGE_SIZE, SEEK_SET) < 0) {
        perror("lseek");
        exit(EXIT_FAILURE);
    }

    int got = 0;
    while (got < PAGE_SIZE) {
        ssize_t r = read(mem.backing_fd, buf + got, PAGE_SIZE - got);
        if (r < 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        if (r == 0) {
            fprintf(stderr, "%s: unexpected end of file reading page %d\n",
                    BACKING_STORE_FILE, page);
            exit(EXIT_FAILURE);
        }
        got += (int)r;
    }
}

void memory_init(int num_frames)
{
    if (num_frames <= 0 || num_frames > MAX_FRAMES) {
        fprintf(stderr, "memory_init: invalid frame count %d\n", num_frames);
        exit(EXIT_FAILURE);
    }

    mem.num_frames = num_frames;
    mem.frames = calloc((size_t)num_frames * PAGE_SIZE, 1);
    if (mem.frames == NULL) {
        perror("calloc");
        exit(EXIT_FAILURE);
    }

    mem.backing_fd = open(BACKING_STORE_FILE, O_RDONLY);
    if (mem.backing_fd < 0) {
        perror(BACKING_STORE_FILE);
        exit(EXIT_FAILURE);
    }
}

void load_page(int page, int frame)
{
    if (page < 0 || page >= NUM_PAGES) {
        fprintf(stderr, "load_page: page %d out of range\n", page);
        exit(EXIT_FAILURE);
    }
    if (frame < 0 || frame >= mem.num_frames) {
        fprintf(stderr, "load_page: frame %d out of range (FRAMES=%d)\n",
                frame, mem.num_frames);
        exit(EXIT_FAILURE);
    }
    backing_read_page(page, mem.frames + frame * PAGE_SIZE);
}

signed char memory_read(int phys_addr)
{
    if (phys_addr < 0 || phys_addr >= mem.num_frames * PAGE_SIZE) {
        fprintf(stderr, "memory_read: physical address %d out of range\n", phys_addr);
        exit(EXIT_FAILURE);
    }
    return mem.frames[phys_addr];
}

void memory_cleanup(void)
{
    if (mem.backing_fd >= 0) close(mem.backing_fd);
    mem.backing_fd = -1;
    free(mem.frames);
    mem.frames = NULL;
}