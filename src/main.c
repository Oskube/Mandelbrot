#include "SDL.h"
#include "chunks.h"
#include <stdbool.h>
#include <stdio.h>  /* fprintf, printf */
#include <stdlib.h> /* malloc, free, atoi*/
#include <string.h> /* parsing cmdline args */

#define DEF_WINDOW_WIDTH  1200
#define DEF_WINDOW_HEIGHT 720

typedef struct bounds {
    double rl_high, rl_low, im_high, im_low;
    struct bounds* last;
} bounds;
bounds* CalcNewView(bounds* current, unsigned winWidth, unsigned winHeight,unsigned x1, unsigned y1, unsigned x2, unsigned y2);

SDL_Texture* TextureMandelbrot(SDL_Renderer* ren, map* ptr, unsigned max_iter);

void PrintChunks(map* ptr);
void RenderChunksStart(SDL_Renderer* ren, chunklist* last);

int main(int argc, char** argv) {
    unsigned winWidth = DEF_WINDOW_WIDTH;
    unsigned winHeight = DEF_WINDOW_HEIGHT;
    unsigned max_iter = 100;
    unsigned base = 128;
    unsigned max_diff = 3;

    //  Default view, shows whole fractal
    bounds viewRoot = {
        .rl_high = 1.0f,
        .rl_low = -2.0f,
        .im_high = -1.0f,
        .im_low = 1.0f,
        .last = NULL
    };
    bounds *viewCurrent = &viewRoot;

    //  Process cmd line arguments
    for (unsigned i=1; i<argc; i++) {
        if (!strcmp(argv[i], "-width")) {
            if (++i < argc) winWidth = atoi(argv[i]);
        } else if (!strcmp(argv[i], "-height")) {
            if (++i < argc) winHeight = atoi(argv[i]);
        } else if (!strcmp(argv[i], "-i")) {
            if (++i < argc) max_iter = atoi(argv[i]);
        } else if (!strcmp(argv[i], "-b")) {
            if (++i < argc) base = atoi(argv[i]);
        } else if (!strcmp(argv[i], "-d")) {
            if (++i < argc) max_diff = atoi(argv[i]);
        } else if (!strcmp(argv[i], "-p")) {
            if (i+4 < argc) {
                bounds* a = (bounds*)malloc(sizeof(bounds));
                a->last = viewCurrent;
                a->rl_low = atof(argv[++i]);
                a->rl_high = atof(argv[++i]);
                a->im_low = atof(argv[++i]);
                a->im_high = atof(argv[++i]);
                viewCurrent = a;
            }
        } else {
            printf ("Usage: ./mandelbrot [options] \
            \nOptions: \
            \n -width <width> \t window width\
            \n -height <height>\t window height\
            \n -i <iterations>\t maximum iterations\
            \n -b <base>\t\t starting size of chunk\
            \n -d <difference>\t maximum difference allowed between chunks\
            \n -p <real-low> <real-up> <im-low> <im-up>\tbounds in complex plane\n");
            return -1;
        }
    }
    if (argc > 1) {
        printf("Starting program with: \
        \n\twindow: %u x %u chunk_base: %u \
        \n\tview: rl(%g to %g), im(%g to %g) \tmax_iter: %u\n",
        winWidth, winHeight, base, viewCurrent->rl_low, viewCurrent->rl_high, viewCurrent->im_low, viewCurrent->im_high, max_iter);
    }

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL_Init failed!");
        return 1;
    }
    SDL_Window* win = SDL_CreateWindow(
        "SDL2 - Mandelbrot",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        winWidth,
        winHeight,
        SDL_WINDOW_RESIZABLE);
    if (!win) {
        printf("Window creation failed!");
        return 2;
    }

    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (!ren) {
        printf("Renderer creation failed!");
        return 3;
    }


    SDL_Texture* textMandel = NULL;
    map* mandelbrot = NULL;

    printf("Initialization completed\nEntering main loop\n");

    SDL_Rect* selection = NULL;
    int recalc = 1;
    bool drawChunks = false;
    bool reset = true;
    bool noQuit = true;
    while (noQuit) {
        //  ------ EVENT HANDLER --------
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
                case SDL_QUIT: { noQuit = false; } break;
                case SDL_KEYUP: {
                    SDL_Keymod mod = SDL_GetModState();
                    int multi = 1;
                    //  Check modifiers
                    if (mod & KMOD_CTRL) {
                        if (mod & KMOD_SHIFT) multi *= 10;
                        else if (mod & KMOD_ALT) multi *= 100;
                        multi *= 10;
                    }
                    if (ev.key.keysym.sym == SDLK_KP_PLUS) {
                        max_iter += 1*multi;
                        reset = true;
                    }
                    else if (ev.key.keysym.sym == SDLK_KP_MINUS) {
                        if (max_iter < 1*multi)
                            max_iter = 0;
                        else
                            max_iter -= 1*multi;
                        reset = true;
                    }
                    else if (ev.key.keysym.sym == SDLK_SPACE) {
                        drawChunks = !drawChunks;
                        printf("Drawing chunks: %s\n", drawChunks ? "true" : "false");
                    }
                    else if (ev.key.keysym.sym == SDLK_r) {
                        reset = true;
                    }
                    else if (ev.key.keysym.sym == SDLK_p) {
                        PrintChunks(mandelbrot);
                    }
                } break;
                case SDL_MOUSEBUTTONUP: {
                    if (ev.button.button == SDL_BUTTON_LEFT) {
                        int x1 = selection->x, y1 = selection->y;
                        int x2 = ev.button.x, y2 = ev.button.y;
                        //  Free selection
                        free(selection);
                        selection = NULL;

                        if (!(SDL_GetModState() & KMOD_CTRL)) {
                            unsigned w = abs(x1-x2);
                            unsigned h = w * (float)winHeight/winWidth;
                            printf ("%d, %d", w, h);
                            if (y2 < y1) {
                                y2 = y1-h;
                            } else {
                                y2 = y1+h;
                            }
                        }

                        viewCurrent = CalcNewView(viewCurrent, winWidth, winHeight, x1, y1, x2, y2);
                        printf("New boundaries: t:%f, r:%f, b:%f, l:%f\n", viewCurrent->rl_low, viewCurrent->rl_high, viewCurrent->im_low, viewCurrent->im_high);

                        //  Request recalc
                        reset = true;
                    }
                } break;
                case SDL_MOUSEBUTTONDOWN: {
                    if (ev.button.button == SDL_BUTTON_LEFT) {
                        selection = malloc(sizeof(SDL_Rect));
                        if (selection) {
                            selection->x = ev.button.x;
                            selection->y = ev.button.y;
                        }
                    } else if (ev.button.button == SDL_BUTTON_RIGHT) {
                        if (viewCurrent->last != NULL) {
                            //  Free current view and set previous as current
                            bounds* temp = viewCurrent->last;
                            free(viewCurrent);
                            viewCurrent = temp;

                            reset = true;
                        }
                    }
                } break;
                case SDL_WINDOWEVENT: {
                    if (ev.window.event == SDL_WINDOWEVENT_RESIZED) {
                        winWidth  = ev.window.data1;
                        winHeight = ev.window.data2;
                        printf("New window size: %u, %u.\n", winWidth, winHeight);
                        reset = true;
                    }
                } break;
                default: break;
            }
        }
        //  ----------- END EVENT HANDLER -------
        if (reset) {
            if (mandelbrot) FreeMap(mandelbrot);

            mandelbrot = InitMap(winWidth, winHeight, base, viewCurrent->rl_low, viewCurrent->rl_high, viewCurrent->im_low, viewCurrent->im_high);
            recalc = 1;
            reset = false;
        }
        //  If recalc is requested
        if (recalc) {
            fprintf(stderr, "Calculating mandelbrot, %d iterations... ", max_iter);
            unsigned startTicks = SDL_GetTicks();

            IterateChunks(mandelbrot, max_iter);
            FlagDifferent(mandelbrot, max_diff);
            recalc = SplitChunks(mandelbrot);

            fprintf(stderr, "time: %d\nRendering mandelbrot... ", SDL_GetTicks()-startTicks);
            startTicks = SDL_GetTicks();

            if (textMandel) SDL_DestroyTexture(textMandel);
            textMandel = TextureMandelbrot(ren, mandelbrot, max_iter);
            fprintf(stderr, "time: %d\n", SDL_GetTicks()-startTicks);
        }

        const char *error = SDL_GetError();
        if (strlen(error) > 2) {
            fprintf(stderr, "SDL_Error: %s\n", SDL_GetError());
            SDL_ClearError();
        }

        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderClear(ren);

        //  If mandelbrot texture exists
        if (textMandel) SDL_RenderCopy(ren, textMandel, NULL, NULL);

        if (drawChunks) RenderChunksStart(ren, mandelbrot->lastChunk);

        //  If first corner of selection is set, draw box to current mouse location.
        if (selection) {
            SDL_SetRenderDrawColor(ren, 255, 255, 255, 64);
            int x2, y2;
            SDL_GetMouseState(&x2, &y2);
            selection->w = x2-selection->x;
            if (SDL_GetModState() & KMOD_CTRL) {
                selection->h = y2-selection->y;
            } else {
                selection->h = abs(selection->w) * (float)winHeight/winWidth;
                if (y2 < selection->y) selection->h = -selection->h;
            }

            SDL_RenderDrawRect(ren, selection);
        }

        SDL_RenderPresent(ren);
    }
    // Free view history
    while (viewCurrent != &viewRoot) {
        bounds* tmp = viewCurrent;
        viewCurrent = viewCurrent->last;
        free(tmp);
    }

    FreeMap(mandelbrot);
    free(selection);

    SDL_DestroyTexture(textMandel);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    printf("Exiting successful\n");
    return 0;
}

SDL_Texture* TextureMandelbrot(SDL_Renderer* ren, map* ptr, unsigned max_iter) {
    unsigned length = ptr->width*ptr->height;
    unsigned* pixels = (unsigned*)malloc(sizeof(unsigned)*length);
    if (!pixels) return NULL;
    for (unsigned i = 0; i < length; i++) pixels[i] = 0;

    double colStep = (double)255/max_iter;

    unsigned char* red = (unsigned char*)pixels;
    for (int i=0; i < length; i++, red += 4) {
        *(red+3) = 0xff;
        unsigned iter = ptr->chunks[i]->iterations;
        if (iter>= max_iter) {
            continue;
        } else if (iter < max_iter/2) {
            *red = (double)iter * colStep * 2;
        } else {
            int c = (double)iter * colStep;
            *red = 0xff;
            *(red+1) = c;
            *(red+2) = c;
        }
    }

    SDL_Surface* surf = SDL_CreateRGBSurfaceFrom((void*)pixels, ptr->width, ptr->height, sizeof(unsigned)*8, ptr->width*sizeof(unsigned), 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
    if (!surf) {
        free(pixels);
        fprintf(stderr, "ERROR: Surface creation failed.\n");
        return NULL;
    }
    SDL_Texture* ret = SDL_CreateTextureFromSurface(ren, surf);
    SDL_FreeSurface(surf);
    free(pixels);   //  Surface was made by with SDL_CreateRGBSurfaceFrom, so the pixel data must be freed separetely

    return ret;
}

void RenderChunksStart(SDL_Renderer* ren, chunklist* last) {
    chunklist* cur = last;
    unsigned char* r;
    while (cur != NULL) {
        unsigned col = rand();
        r = (unsigned char*)&col;
        SDL_SetRenderDrawColor(ren, *r, *(r+1), *(r+2), 255);
        SDL_RenderDrawPoint(ren, cur->chn->x, cur->chn->y);
        cur = cur->prev;
    }
}

void PrintChunks(map* ptr) {
   chunklist* cur = ptr->lastChunk;
   unsigned i = 0;
   unsigned totalArea = 0;
   while (cur != NULL) {
       chunk* chn = cur->chn;
       printf("%u: %p:\tit: %u\tpos: (%u, %u)\t(w,h): (%u, %u)\t(%g, %g)\n",
           i++, (void*)chn, chn->iterations, chn->x, chn->y, chn->w, chn->h, chn->rl, chn->im);
       totalArea += chn->w*chn->h;
       cur = cur->prev;
   }
   printf("%u chunks cover area of %u.\n", i, totalArea);
}

bounds* CalcNewView(bounds* current, unsigned winWidth, unsigned winHeight,unsigned x1, unsigned y1, unsigned x2, unsigned y2) {
    bounds* newView = (bounds*)malloc(sizeof(bounds));
    if (!newView || x1 == x2 || y1 == y2) return current;

    if (x2 < x1) {
        int swp = x2;
        x2 = x1;
        x1 = swp;
    }
    if (y2 < y1) {
        int swp = y2;
        y2 = y1;
        y1 = swp;
    }

    //  Calculate new boundaries
    double length = current->rl_high - current->rl_low;
    double ratio = length/winWidth;
    newView->rl_high = current->rl_low + ratio*x2;
    newView->rl_low = current->rl_low + ratio*x1;

    length = current->im_high - current->im_low;
    ratio = length/winHeight;
    newView->im_high = current->im_low + ratio*(winHeight-y1);
    newView->im_low = current->im_low + ratio*(winHeight-y2);

    newView->last = current;
    return newView;
}
