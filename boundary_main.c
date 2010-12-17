/** boundary_main.c
 *
 * Main source file for CarBoundary.
 */

#include <SDL.h>
#include <SDL_gfxPrimitives.h>
#include "libfreenect_sync.h"
#include <math.h>

#define WIDTH 1024
#define HEIGHT 1024

SDL_Surface *screen;
SDL_Surface *render;

int depths[WIDTH*HEIGHT];
double depthDistance[2048];
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
fillDepths(int depths[]) {
  int i,j;
  int ci, cj;
  ci = WIDTH / 2;
  cj = HEIGHT / 2;
  double di, dj;
  double ni, nj;
  for (j = cj-5; j >= 0; j--) {
    for (i = 0; i < WIDTH; i++) {
      if (depths[j*WIDTH+i] == 0) {
        /* This pixel is empty. Inherit its value from the next one closer
         * to cj,ci */
        depths[j*WIDTH+i] = depths[(j+1)*WIDTH+i];
        if (!depths[j*WIDTH+i])
          depths[j*WIDTH+i] = depths[(j+1)*WIDTH+i+1];
        if (!depths[j*WIDTH+i])
          depths[j*WIDTH+i] = depths[(j+1)*WIDTH+i-1];
      }
    }
  }
}

/**
 * The depth map is 20 meters by 20 meters.
 *
 * depthDistance[depthval] is the distance in meters.
 */

/* baseX, Y, Z are in meters.
 * hangle and vangle are in degerees.
 */
#define PI 3.141592654
#define DEG2RAD(x) (x * (PI / 180.0))
void
poll_one_device(int device,
                double baseX, double baseY, double baseZ,
                double hangle, double vangle) {
  int i,j;
  uint16_t *fdepths;
  uint32_t timestamp;
  double d;
  double x, y, z;
  double ha, va;
  int ix, iy, iz;
  if (!freenect_sync_get_depth((void **) &fdepths,
                               &timestamp,
                               device,
                               FREENECT_DEPTH_11BIT)) {
    for (j = 0; j < 480; j++) {
      va = vangle + (43.0 * (((double) j) / 480.0)) - 28.5;
      for (i = 0; i < 640; i++) {
        d = depthDistance[fdepths[j*640+i] & 0x7FF];
        /* It's not worth plotting if it's too far away or too close. */
        if (d > 0.1 && d < 30.0) {
          ha = hangle + (57.0 * (((double) i) / 640.0)) - 28.5;

          /* We have spherical coordinates, and now we need to convert that to
           * cartesian coordiantes. */
          /* Trig approach. Needs tweaking? */
          double dh = d * cos(DEG2RAD(va));
          y = dh * cos(DEG2RAD(ha));
          x = dh * sin(DEG2RAD(ha));
          z = d * cos(DEG2RAD(va+90.0));
          y = -y;
          /* Approximation approach: Broken. */
          /*
          y = (i - 320) * (d - 10.0) * (640/480) * 0.0021;
          z = (j - 240) * (d - 10.0) * 0.0021;
          x = d;
          */

          /*
          if (!(i&0x5f) && !(j & 0x5f)) {
            printf("%d,%d:%f meters away. %f meters horizontally, %f vert",
                i, j, d, dh, z);
            printf("%d,%d:%d/%f %f meters ahead, %f meters to the side, %f meters tall.\n",
                i, j, fdepths[j*640+i] & 0x7FF, d,
                x, y, z);
          }
          */

          /* Measurements are relative to the camera's position. Now adjust
           * for the base position. */
          x += baseX;
          y += baseY;
          z += baseZ;

          /* Now PLOT onto the depth chart! */
          ix = (int) (x * (1024 / 20.0)); /* Pixels per meter */
          iy = (int) (y * (1024 / 20.0)); /* Pixels per meter */
          iz = (int) (z * (2048 / 2.0)); /* Depth units per meter */

          if (ix >= 0 && ix < WIDTH &&
              iy >= 0 && iy < HEIGHT) {
            if (iz > 2048) iz = 2046;
            if (iz > 0 && iz < 2048) {
              if (depths[iy*WIDTH+ix] < iz) {
                depths[iy*WIDTH + ix] = iz;
              }
            }
          }
        }
      }
    }
    // fillDepths(depths);
  }
}

void
kinect_poll() {
  /* Clear depth map out */
  memset(depths, 0, sizeof(depths));
  /*
    poll_one_device(int device,
                    double baseX, double baseY, double baseZ,
                    double hangle, double vangle);
  */
  /* For each device we have, plot its detected collisions. */

  /* Just one device for now: */
  /* It's in the center (10m, 10m). Half a meter off the ground (0.5m),
   * pointing straight forward (0.0) and resting flat and horizontal (0.0)
   */
  poll_one_device(0, 10.0, 10.0, 0.5, 0.0, 0.0);
}

/* Pixel distance lookup ganked from ofxKinect and
 * http://openkinect.org/wiki/Imaging_Information
 */
int
kinect_init() {
  int i;
  const double k1 = 0.1236;
  const double k2 = 2842.5;
  const double k3 = 1.1863;
  const double k4 = 0.0370;

  /* Populate depthDistance[i], in meters. */
  for (i = 1; i < 2047; i++) {
    depthDistance[i] = k1 * tan((i/k2) + k3) - k4;
    /* Colors; Red for bits 0-4, green for bits 5-8, blue for 9-11 */
    if (!(i&0x7F)) {
      printf("At distance %d (%f), meters is %f\n", i, i/k2, depthDistance[i]);
    }
    /* Psychedelic:
    depthColors[i] =
        ((i & 0xF) << 20) // Red
      | ((i & 0xF0) << 8) // Green
      | ((i & 0xF00) >> 3) // Blue
      ;
    /**/
    /* Blue shades: */
    depthColors[i] =
          0xFF // blue
        | 0x0101 * (i/4);
        /**/
  }
  depthDistance[2047] = 0.0;
  depthColors[2047] = 0;
  depthColors[0] = 0xFFFFFF;
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
