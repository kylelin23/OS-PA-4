//translation lookaside buffer
// hardware cache that stores recent page-toframe translations 
// so that the CPU does not have to walk the full page table in main memory on every memory access

//16 entries in the TLB

//TLB Hit: page number is found in the TLB, corresponding frame number is retrieved instantly
//TLB Miss: page nunmber is not found in the TLB, CPU accesses the page table in main memory

#include <stdio.h>
#include "memsim.h"

typedef struct {
    int page;
    int frame;
} tlb_entry;

static tlb_entry tlb[TLB_SIZE]; //memsim.h
static int fifo_index; //fifo replacement policy

void tlb_init(void){
    for (int c = 0; c < TLB_SIZE; c++){
        tlb[c].page = NOT_FOUND; //marking as empty/invalid
        tlb[c].frame = NOT_FOUND; //marking as empty/invalid
    }
    fifo_index = 0;
}

int tlb_lookup(int page){
    for (int c = 0; c < TLB_SIZE; c ++){
        if (tlb[c].frame != NOT_FOUND && tlb[c].page == page){
            return tlb[c].frame;
        }
    }
    return NOT_FOUND;
}

void tlb_insert(int page, int frame){
    for (int c = 0; c < TLB_SIZE; c++){
        if(tlb[c].page == page && tlb[c].frame != NOT_FOUND){
            tlb[c].frame = frame;
            return;
        }
    }
    tlb[fifo_index].page = page;
    tlb[fifo_index].frame = frame;
    fifo_index = (fifo_index + 1) % TLB_SIZE;
}

void tlb_invalidate(int page){
    for(int c = 0; c < TLB_SIZE; c++){
        if(tlb[c].page == page && tlb[c].frame != NOT_FOUND){
            tlb[c].frame = NOT_FOUND;
            tlb[c].page = NOT_FOUND;
            return;
        }
    }
}