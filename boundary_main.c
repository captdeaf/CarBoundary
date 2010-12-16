/** boundary_main.c
 *
 * Main source file for CarBoundary.
 */

#include "SDL.h"

#define WIDTH 1024
#define HEIGHT 768

SDL_Surface *screen;
SDL_Surface *render;

int
sdl_init() {
  /* startup SDL */
  if (SDL_Init(SDL_INIT_VIDEO)==-1) {
    printf("SDL_Init: %s\n", SDL_GetError());
    return 0;
  }

  screen=SDL_SetVideoMode(WIDTH, HEIGHT, 32, SDL_DOUBLEBUF | SDL_ANYFORMAT);
  if (!screen) {
    printf("SDL_SetVideoMode: %s\n", SDL_GetError());
    SDL_Quit();
    return 0;
  }

  render = SDL_CreateRGBSurface(SDL_SWSURFACE, WIDTH, HEIGHT, 32,
                                0xFF << 16, 0xFF << 8, 0xFF, 0xFF << 24);
  if (!render) {
    printf("SDL_CreateRGBSurface: %s\n", SDL_GetError());
    SDL_Quit();
    return 0;
  }

  /* set the window title to the filename */
  SDL_WM_SetCaption("CarBoundary", "CarBoundary");

  /* print some info about the obtained screen */
  printf("screen is %dx%d %dbpp\n", screen->w, screen->h, screen->format->BitsPerPixel);
}

int
sdl_pollevent() {
  SDL_Event event;
  int done = 0;
  /* the event loop, redraws if needed, quits on keypress or quit event */
  while(!done && SDL_PollEvent(&event)) {
    switch(event.type) {
    case SDL_KEYDOWN:
    case SDL_QUIT:
      /* quit events, exit the event loop */
      done=1;
      break;
    case SDL_VIDEOEXPOSE:
      /* need a redraw, we just redraw the whole screen for simplicity */
      SDL_BlitSurface(render,0,screen,0);
      SDL_Flip(screen);
      break;
    default:
      break;
    }
  }
  return done;
}

void
sdl_shutdown() {
  /* free the loaded surfaces */
  SDL_FreeSurface(render);

  /* shutdown SDL */
  SDL_Quit();
}

void
setPixel(SDL_Surface *pSurface, int x, int y, int color) {
  int col = SDL_MapRGB(pSurface->format,
                       (color & 0xFF0000) >> 16,
                       (color & 0xFF00) >> 8,
                       (color & 0xFF));
  char *pPosition = (char *) pSurface->pixels;
  pPosition += (pSurface->pitch * y);
  pPosition += (pSurface->format->BytesPerPixel * x);
  memcpy(pPosition, &col, pSurface->format->BytesPerPixel);
}

#define SETPIXEL(x,y,col) setPixel(render, x, y, col)
// #define SETPIXEL(x,y,col) *((int *) render->pixels + y * WIDTH + x) = col

void
draw_surroundings() {
  int x,y;
  SDL_LockSurface(render);
  for (y = 0; y < HEIGHT; y++) {
    for (x = 0; x < WIDTH; x++) {
      SETPIXEL(x, y, ((x%0xFF) << 16) | ((y%0xFF) << 8));
    }
  }
  SDL_UnlockSurface(render);
  SDL_BlitSurface(render,0,screen,0);
  SDL_Flip(screen);
}

int
main(int argc, char *argv[]) {
  if (!sdl_init()) {
    return;
  }

  while (!sdl_pollevent()) {
    /* Sleep 1/10th of a second. */
    usleep(100000);
    draw_surroundings();
  }

  /* Shut down */
  sdl_shutdown();
}
