#define CHUNK_DIFF 1
#define CHUNK_CALC 2

typedef struct chunk {
    unsigned int iterations;
    unsigned int x,y, w,h;
    double rl, im; //   position in complex plane
    unsigned int flags;
} chunk;

typedef struct chunklist {
    chunk* chn;
    struct chunklist* prev;
} chunklist;

typedef struct map {
    chunk** chunks; //  Array of pointers to chunks. Cell for every pixel
    chunklist* lastChunk;
    double rl_low, rl_high, im_low, im_high;
    unsigned int width, height;
} map;

//  Initializes map to chunks with given size
extern map* InitMap(unsigned int mapw, unsigned int maph, unsigned int base, double rll, double rlr, double imb, double imt);
//  Frees memory allocated for map, map will be freed as well, return NULL
extern map* FreeMap(map* src);
//  Sets CHUNK_DIFF if difference of iterations to its neighbors is > maxdiff
extern void FlagDifferent(map* ptr, unsigned int maxdiff);
//  Splits flagged chunks if possible, and sets CHUNK_CALC
extern int SplitChunks(map* ptr);
//  Calculates mandelbrot for given array of chunks
extern void IterateChunks(map* src, unsigned int max_iter);
