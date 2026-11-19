#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "memsim.h"

typedef struct {
    int tlb_hits;
    int page_faults;
} stats_t;

static void usage(const char *prog)
{
    fprintf(stderr, "Usage: %s <reference-sequence-file.txt> [FRAMES] [PRA]\n"
                    "  FRAMES: integer, 0 < FRAMES <= 256 (default 256)\n"
                    "  PRA:    fifo | lru | opt (default fifo)\n", prog);
    exit(EXIT_FAILURE);
}

static int parse_frames(const char *s, const char *prog)
{
    char *end;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0' || v <= 0 || v > MAX_FRAMES) {
        fprintf(stderr, "Invalid FRAMES '%s' (must be 1..%d)\n", s, MAX_FRAMES);
        usage(prog);
    }
    return (int)v;
}

static pra_t parse_pra(const char *s, const char *prog)
{
    if (strcmp(s, "fifo") == 0) return PRA_FIFO;
    if (strcmp(s, "lru") == 0)  return PRA_LRU;
    if (strcmp(s, "opt") == 0)  return PRA_OPT;
    fprintf(stderr, "Invalid PRA '%s' (must be fifo, lru, or opt)\n", s);
    usage(prog);
    return PRA_FIFO;
}

static int translate(int logical, int i, stats_t *stats)
{
    int masked = logical & ADDR_MASK;
    int page   = (masked >> PAGE_SHIFT) & OFFSET_MASK;
    int offset = masked & OFFSET_MASK;

    int frame = tlb_lookup(page);
    if (frame != NOT_FOUND) {
        stats->tlb_hits++;
    } else {
        frame = pt_lookup(page);
        if (frame == NOT_FOUND) {
            stats->page_faults++;
            frame = get_frame(page, i);
            load_page(page, frame);
            pt_set(page, frame);
        }
        tlb_insert(page, frame);
    }

    record_access(page, frame, i);
    return frame * PAGE_SIZE + offset;
}

static void print_stats(const stats_t *stats, int n_refs)
{
    double fault_rate = n_refs ? (double)stats->page_faults / n_refs : 0.0;
    double tlb_rate   = n_refs ? (double)stats->tlb_hits / n_refs : 0.0;

    printf("Number of Translated Addresses = %d\n", n_refs);
    printf("Page Faults = %d\n", stats->page_faults);
    printf("Page Fault Rate = %.3f\n", fault_rate);
    printf("TLB Hits = %d\n", stats->tlb_hits);
    printf("TLB Hit Rate = %.3f\n", tlb_rate);
}

int main(int argc, char *argv[])
{
    if (argc < 2 || argc > 4) usage(argv[0]);

    const char *ref_file = argv[1];
    int frames = (argc >= 3) ? parse_frames(argv[2], argv[0]) : DEFAULT_FRAMES;
    pra_t pra  = (argc >= 4) ? parse_pra(argv[3], argv[0])    : PRA_FIFO;

    int n_refs = 0;
    int *refs = read_references(ref_file, &n_refs);

    memory_init(frames);
    pt_init();
    tlb_init();
    replace_init(frames, pra, refs, n_refs);

    stats_t stats = {0, 0};
    for (int i = 0; i < n_refs; i++) {
        int physical = translate(refs[i], i, &stats);
        signed char value = memory_read(physical);
        printf("Virtual address: %d Physical address: %d Value: %d\n",
               refs[i], physical, value);
    }

    print_stats(&stats, n_refs);

    replace_cleanup();
    memory_cleanup();
    free(refs);
    return EXIT_SUCCESS;
}