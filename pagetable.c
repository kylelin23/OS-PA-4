#include <stdio.h>
#include <stdlib.h>
#include "memsim.h"

static int page_table[NUM_PAGES];

static void check_page(int page, const char *who)
{
    if (page < 0 || page >= NUM_PAGES) {
        fprintf(stderr, "%s: invalid page number %d\n", who, page);
        exit(EXIT_FAILURE);
    }
}

void pt_init(void)
{
    for (int p = 0; p < NUM_PAGES; p++) {
        page_table[p] = NOT_FOUND;
    }
}

int pt_lookup(int page)
{
    check_page(page, "pt_lookup");
    return page_table[page];
}

void pt_set(int page, int frame)
{
    check_page(page, "pt_set");
    if (frame < 0 || frame >= MAX_FRAMES) {
        fprintf(stderr, "pt_set: invalid frame number %d\n", frame);
        exit(EXIT_FAILURE);
    }
    page_table[page] = frame;
}

void pt_invalidate(int page)
{
    check_page(page, "pt_invalidate");
    page_table[page] = NOT_FOUND;
}