#include <stdlib.h>
#include "chunks.h"

//  Updates maps chunks array with given chunk
static void MapChunk(map* ptrMap, chunk* ptrChn) {
    if (ptrMap == NULL || ptrChn == NULL) return;

    //  Set section of map to point the chunk
    unsigned pos = ptrChn->y*ptrMap->width +ptrChn->x;
    for (unsigned rows = 0, col=0; rows < ptrChn->h;) {
        ptrMap->chunks[pos+col] = ptrChn;
        if (col+1 >= ptrChn->w) {
            col=0;
            pos += ptrMap->width;
            rows++;
        } else {
            col++;
        }
    }
}

static chunklist* AddToList(chunklist* last, chunk* addition) {
    chunklist* newnode = (chunklist*)malloc(sizeof(chunklist));
    newnode->chn = addition;
    if (last == NULL) {
        newnode->prev = NULL;
    } else {
        newnode->prev = last;
    }
    return newnode;
}

static void InterpolateCenter(map* ptr, chunk* chn) {
    double rlstep = (ptr->rl_high - ptr->rl_low) /ptr->width;
    double imstep = (ptr->im_high - ptr->im_low) /ptr->height;

    //  Chunk center
    unsigned chunkx = (chn->x + (double)chn->w/2);
    unsigned chunky = (ptr->height - (chn->y + (double)chn->h/2)); // chunkcenter and rotate y axle 180 deg

    chn->rl = ptr->rl_low + rlstep*chunkx;
    chn->im = ptr->im_low + imstep*chunky;
}

static chunk* CreateChunk(map* ptr, unsigned x, unsigned y, unsigned w, unsigned h) {
    chunk* add = (chunk*)malloc(sizeof(chunk));
    if (add) {
        add->iterations = 0;
        add->x = x;
        add->y = y;
        add->w = w;
        add->h = h;
        add->rl = 0;
        add->im = 0;
        add->flags |= CHUNK_CALC;

        InterpolateCenter(ptr, add);
        MapChunk(ptr, add);
        ptr->lastChunk = AddToList(ptr->lastChunk, add);
    }
    return add;
}

// -------------------------------------------------------------
//  Functions declared in chunks.h
map* InitMap(unsigned mapw, unsigned maph, unsigned base, double rl_low, double rl_high, double im_low, double im_high) {
    if (rl_high < rl_low) {
        double tmp = rl_high;
        rl_high = rl_low;
        rl_high = tmp;
    }
    if (im_high < im_low) {
        double tmp = im_high;
        im_high = im_low;
        im_high = tmp;
    }

    map* ret = (map*)malloc(sizeof(map));
    ret->width = mapw;
    ret->height = maph;
    ret->chunks = NULL;
    ret->lastChunk = NULL;
    ret->rl_low = rl_low;
    ret->rl_high = rl_high;
    ret->im_low = im_low;
    ret->im_high = im_high;

    //  Every cell/pixel belongs to chunk
    unsigned len = mapw*maph;
    ret->chunks = (chunk**)malloc(sizeof(chunk*)*len);

    //  Chunks per row/column
    int chunksX = mapw/base;
    int chunksY = maph/base;
    if (mapw%base > 0) chunksX++;
    if (maph%base > 0) chunksY++;

    for (unsigned row = 0; row < chunksY; row++) {
        unsigned chnH = base;
        unsigned chnW = base;

        //  If chunk is too tall make it shorter
        if (base*row+chnH > maph) {
            chnH -= base*row+chnH-maph;
            if (chnH <= 0) break;   // heigh must be atleast 1
        }

        for (unsigned col = 0; col < chunksX; col++) {
            //  If chunk is too wide make it narrow
            if (base*col+chnW > mapw) {
                chnW -= base*col+chnW-mapw;
                if (chnW <= 0) break;
            }
            //  Create chunk
            CreateChunk(ret, col*base, row*base, chnW, chnH);
        }
    }
    return ret;
}

map* FreeMap(map* ptr) {
    if (ptr == NULL) return NULL;
    //  Iterate through chunklist and free the chunk and the list element
    chunklist* last = ptr->lastChunk;
    while (last != NULL) {
        free(last->chn);
        chunklist* tmp = last;
        last = last->prev;
        free(tmp);
    }
    ptr->lastChunk = NULL;

    //  Free chunks array
    free(ptr->chunks);
    ptr->chunks = NULL;
    free(ptr);
    return NULL;
}

void FlagDifferent(map* ptr, unsigned maxdiff) {
    chunklist* cur = ptr->lastChunk;

    while (cur != NULL) {
        chunk* chn = cur->chn;

        unsigned chunkbegin = ptr->width*chn->y + chn->x;
        //  Vertical
        unsigned pos = chunkbegin+chn->w+1;
        if (chn->x+chn->w+1 < ptr->width) {
            for (unsigned y=0; y < chn->h; y++, pos+=ptr->width) {
                int diff = ptr->chunks[pos]->iterations - chn->iterations;

                if (diff < 0) diff = -diff; // absolute value
                // if difference is too big set recalc
                if (diff > maxdiff) {
                    chn->flags |= CHUNK_DIFF;
                    ptr->chunks[pos]->flags |= CHUNK_DIFF;
                }
            }
        }

        //  Horizontal
        pos = chunkbegin + (chn->h+1)*ptr->width;
        if (chn->y+chn->h+1 < ptr->height) {
            for(int x=0; x < chn->w; x++, pos++) {
                int diff = ptr->chunks[pos]->iterations - chn->iterations;

                if (diff < 0) diff = -diff; // absolute value
                if (diff > maxdiff) {
                    chn->flags |= CHUNK_DIFF;
                    ptr->chunks[pos]->flags |= CHUNK_DIFF;
                }
            }
        }

        //  Diagonial
        pos = chunkbegin + (chn->h+1)*ptr->width + chn->w+1;
        if (pos < ptr->width*ptr->height) {
            int diff = ptr->chunks[pos]->iterations - chn->iterations;

            if (diff < 0) diff = -diff; // absolute value
            if (diff > maxdiff) {
                chn->flags |= CHUNK_DIFF;
                ptr->chunks[pos]->flags |= CHUNK_DIFF;
            }
        }
        pos = chunkbegin + (chn->h+1)*ptr->width -1;
        if (pos < ptr->width*ptr->height) {
            int diff = ptr->chunks[pos]->iterations - chn->iterations;

            if (diff < 0) diff = -diff; // absolute value
            if (diff > maxdiff) {
                chn->flags |= CHUNK_DIFF;
                ptr->chunks[pos]->flags |= CHUNK_DIFF;
            }
        }
        cur = cur->prev;
    }
}

int SplitChunks(map* ptr) {
    //  Iterate all chunks
    chunklist* cur = ptr->lastChunk;
    unsigned count = 0;
    while (cur != NULL) {
        chunk* chn = cur->chn;
        if (chn->flags & CHUNK_DIFF) {
            chn->flags &= ~(CHUNK_DIFF); // unset diff flag
            unsigned w = chn->w;
            unsigned h = chn->h;

            unsigned split = 0;
            if (chn->w > 1) split |=1;
            if (chn->h > 1) split |=2;

            switch (split) {
                case 0: {  // 1x1 chunk, cannot split
                    cur = cur->prev;
                    continue;
                } break;
                case 1: {   //  Vertical split
                    chn->w /= 2;
                    w -= chn->w;
                    CreateChunk(ptr, chn->x+chn->w, chn->y, w, chn->h); // right
                    count += 2;
                } break;
                case 2: {   //  Horizontal
                    chn->h /= 2;
                    h -= chn->h;
                    CreateChunk(ptr, chn->x, chn->y+chn->h, chn->w, h); // bottom
                    count += 2;
                } break;
                case 3: {
                    chn->w /= 2;
                    chn->h /= 2;
                    w -= chn->w;
                    h -= chn->h;

                    CreateChunk(ptr, chn->x+chn->w, chn->y, w, chn->h); // right-top
                    CreateChunk(ptr, chn->x, chn->y+chn->h, chn->w, h); // bottom-left
                    CreateChunk(ptr, chn->x+chn->w, chn->y+chn->h, w, h); // bottom-right
                    count += 4;
                }
            }
            chn->flags |= CHUNK_CALC;
            InterpolateCenter(ptr, chn);
        }
        cur = cur->prev;
    }
    return count;
}

void IterateChunks(map* src, unsigned max_iter) {
    chunklist* cur = src->lastChunk;

    //  find chunks with CHUNK_CALC flag set
    while (cur != NULL) {
        chunk* chn = cur->chn;
        if (chn->flags & CHUNK_CALC) {
            chn->flags &= ~CHUNK_CALC;  // unset flag
            chn->iterations = 0;
            double im = 0, rl = 0, sum = 0, lastSum = 0;
            do {
                double tmp = rl*rl -im*im +chn->rl;
                im = 2*rl*im +chn->im;
                rl = tmp;

                lastSum = sum;
                sum = rl*rl + im*im;
                if (lastSum == sum) {
                    chn->iterations = max_iter;
                    break;
                }
            } while (sum < 4 && chn->iterations++ < max_iter);
        }
        cur = cur->prev;
    }
}
