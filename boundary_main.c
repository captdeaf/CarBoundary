/** boundary_main.c
 *
 * Main source file for CarBoundary.
 */

#include "SDL.h"
#include "libfreenect_sync.h"
#include <math.h>

#define WIDTH 1024
#define HEIGHT 1024

SDL_Surface *screen;
SDL_Surface *render;

int depths[WIDTH*HEIGHT];
float depthDistance[2048];
int depthColors[2048];

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
  return 1;
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
draw_depths() {
  int x,y;
  SDL_LockSurface(render);
  for (y = 0; y < HEIGHT; y++) {
    for (x = 0; x < WIDTH; x++) {
      SETPIXEL(x, y, depthColors[depths[y*WIDTH + x]]);
    }
  }
  SDL_UnlockSurface(render);
  SDL_BlitSurface(render,0,screen,0);
  SDL_Flip(screen);
}

void
poll_one_device(int device,
                float baseX, float baseY, float baseZ,
                float hangle, float vangle) {
  int x,y;
  uint16_t *fdepths;
  uint32_t timestamp;
  if (!freenect_sync_get_depth((void **) &fdepths,
                               &timestamp,
                               device,
                               FREENECT_DEPTH_11BIT)) {
    for (y = 0; y < 480; y++) {
      for (x = 0; x < 640; x++) {
        depths[y*WIDTH+x] = (fdepths[y*640+x] * 0xFF) % 0x7FF;
      }
    }
  }
}

void
kinect_poll() {
  poll_one_device(0, 50.0f, 50.0f, 50.0f, 50.0f, 50.0f);
}

/* Pixel distance lookup ganked from ofxKinect and
 * http://openkinect.org/wiki/Imaging_Information
 */
int
kinect_init() {
  int i;
  const float k1 = 0.1236;
  const float k2 = 2842.5;
  const float k3 = 1.1863;
  const float k4 = 0.0370;

  /* Populate depthDistance[i], in centimeters. */
  for (i = 0; i < 2047; i++) {
    depthDistance[i] = k1 * tanf(i / k2 + k3) - k4;
    depthDistance[i] *= 100;
    /* Colors; Red for bits 0-4, green for bits 5-8, blue for 9-11 */
    depthColors[i] =
        ((i & 0xF) << 20) /* Red */
      | ((i & 0xF0) << 8) /* Green */
      | ((i & 0xF00) >> 3) /* Blue */
      ;
  }
  depthDistance[i] = 0.0f;
  depthColors[i] = 0;
  return 1;
}

int
kinect_shutdown() {
  freenect_sync_stop();
}

int
main(int argc, char *argv[]) {
  if (!sdl_init()) {
    return 1;
  }

  if (!kinect_init()) {
    return 1;
  }

  while (!sdl_pollevent()) {
    /* Sleep 1/10th of a second. */
    usleep(30000);
    kinect_poll();
    draw_depths();
  }

  /* Shut down */
  kinect_shutdown();
  sdl_shutdown();
  return 0;
}
