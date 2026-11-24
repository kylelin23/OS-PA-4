#include <stdio.h>
#include <stdlib.h>
#include "memsim.h"

typedef struct {
    pra_t pra;
    int num_frames; // physical frames
    int *currently_in_frame; 
    int *last_used; 
    int *pages;
    int n_refs;
    int fifo_cursors;
} replace_state; 

static replace_state rep = {PRA_FIFO, 0, NULL, NULL, NULL, 0, 0};


void replace_init(int num_frames, pra_t pra, const int *refs, int n){
    rep.pra = pra;
    rep.num_frames = num_frames;
    rep.n_refs = n;
    rep.fifo_cursors = 0;
    rep.currently_in_frame = NULL;
    rep.last_used = NULL;
    rep.pages = NULL;

    rep.currently_in_frame = malloc((size_t)num_frames * sizeof(int));
    rep.last_used = malloc((size_t)num_frames * sizeof(int));
    if(rep.currently_in_frame == NULL || rep.last_used == NULL){
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    for (int c = 0; c < num_frames; c++){
        rep.currently_in_frame[c] = NOT_FOUND;
        rep.last_used[c] = -1;
    }
    if(pra == PRA_OPT && n > 0 && refs != NULL){
        rep.pages = malloc((size_t)n * sizeof(int));
        if (rep.pages == NULL){
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        for(int c = 0; c < n; c++){
            rep.pages[c] = ((refs[c] & ADDR_MASK) >> PAGE_SHIFT) & OFFSET_MASK;
        }
    }
}
int  get_frame(int page, int i){
    for(int c = 0; c < rep.num_frames; c++){
        if(rep.currently_in_frame[c] == NOT_FOUND){
            rep.currently_in_frame[c] = page;
            return c;
        } 
    }
    
    int w = 0;
    if(rep.pra == PRA_FIFO){
        w = rep.fifo_cursors % rep.num_frames;
        rep.fifo_cursors = (rep.fifo_cursors + 1) % rep.num_frames;
    }
    else if (rep.pra == PRA_LRU){
        for (int c = 1; c < rep.num_frames; c++){
            if(rep.last_used[c] < rep.last_used[w]){
                w = c;
            }
        }
    }
    else if (rep.pra == PRA_OPT){
        int f = -1;
        for (int c = 0; c < rep.num_frames; c++){
            int next_used = -1;
            for(int o = i + 1; o < rep.n_refs; o++){
                if(rep.pages[o] == rep.currently_in_frame[c]){
                    next_used = o;
                    break;
                }
            }
            if(next_used == -1){
                w = c;
                break;
            }
            if(next_used > f){
                f = next_used;
                w = c;
            }
        }
    }
    int last_page = rep.currently_in_frame[w];
    if(last_page != NOT_FOUND){
        pt_invalidate(last_page);
        tlb_invalidate(last_page);
    }
    rep.currently_in_frame[w] = page;
    return w;
}
void record_access(int page, int frame, int i){
    rep.currently_in_frame[frame] = page;
    rep.last_used[frame] = i;
}
void replace_cleanup(void){
    free(rep.currently_in_frame);
    free(rep.last_used);
    free(rep.pages);
    rep.currently_in_frame = rep.last_used = rep.pages = NULL;
}