#ifndef MEMSIM_H
#define MEMSIM_H

#define PAGE_SIZE    256
#define NUM_PAGES    256
#define TLB_SIZE     16
#define MAX_FRAMES   256

#define ADDR_MASK    0xFFFF
#define OFFSET_MASK  0xFF
#define PAGE_SHIFT   8

#define NOT_FOUND    (-1)

#define DEFAULT_FRAMES      256
#define BACKING_STORE_FILE  "BACKING_STORE.bin"

typedef enum {
    PRA_FIFO,
    PRA_LRU,
    PRA_OPT
} pra_t;

int *read_references(const char *path, int *count);

void        memory_init(int num_frames);
void        load_page(int page, int frame);
signed char memory_read(int phys_addr);
void        memory_cleanup(void);

void pt_init(void);
int  pt_lookup(int page);
void pt_set(int page, int frame);
void pt_invalidate(int page);

void tlb_init(void);
int  tlb_lookup(int page);
void tlb_insert(int page, int frame);
void tlb_invalidate(int page);

void replace_init(int num_frames, pra_t pra, const int *refs, int n);
int  get_frame(int page, int i);
void record_access(int page, int frame, int i);
void replace_cleanup(void);

#endif