/** boundary_main.c
 *
 * Main source file for CarBoundary.
 */

#include <SDL.h>
#include <SDL_gfxPrimitives.h>
#include "libfreenect_sync.h"
#include <math.h>
#include "boundary_sdl.h"

struct _kdevice_definition_ {
  int device;
  double baseX;
  double baseY;
  double baseZ;
  double hangle;
  double vangle;
};

#include "device_definitions.h"

/* CUBE_RATIO: Meters per side of a cube. */
#define CUBE_RATIO 0.1
/* CUBE_HEIGHT: # of cubes of height. Anything below get stuck to bottom,
 * anything above gets ignored. */
// 2 meters from bottom to top.
#define CUBE_HEIGHT 20
/* CUBE_WIDTH: # of cubes from left to right our environment is.
 * 15 meters == 150 cubes. */
#define CUBE_WIDTH 150
/* CUBE_LENGTH: # of cubes from front to back.
 * 20 meters == 150 cubes. */

typedef struct _cube_ {
  char known_by; // Which device knows this is here?
  char cleared_by; // Which device has cleared this?
  char guesscount; // How many devices have guessed this?
} Cube;

int depths[SCREEN_WIDTH * SCREEN_HEIGHT];
#define DEPTH_AT(y, x) depths[y*SCREEN_WIDTH + x]
double depthDistance[2048];
double horizDepthMultiplier[SENSOR_WIDTH];
double vertDepthMultiplier[SENSOR_HEIGHT];

/* Color map */
int colorMap[31];

typedef struct _kdevice_ {
  int device;
  double vangle[SENSOR_HEIGHT];
  double hangle[SENSOR_WIDTH];
  double dvv[SENSOR_HEIGHT];
  double dvh[SENSOR_HEIGHT];
  double dx[SENSOR_WIDTH];
  double dy[SENSOR_WIDTH];
  double baseX, baseY, baseZ;
} KDevice;

KDevice *devices;
int numDevices;

void
draw_depths() {
  int x,y;
  SDL_LockSurface(render);
  for (y = 0; y < SCREEN_HEIGHT; y++) {
    for (x = 0; x < SCREEN_WIDTH; x++) {
      setPixel(x, y, Z_COLOR(DEPTH_AT(y,x)));
    }
  }
  SDL_UnlockSurface(render);
  SDL_BlitSurface(render,0,screen,0);
  SDL_Flip(screen);
}

#define PI 3.141592654
#define DEG2RAD(x) (x * (PI / 180.0))
void
init_one_device(KDevice *dev, int device,
                double baseX, double baseY, double baseZ,
                double hangle, double vangle) {
  int i;
  dev->device = device;
  dev->baseX = baseX;
  dev->baseY = baseY;
  dev->baseZ = baseZ;

  for (i = 0; i < SENSOR_WIDTH; i++) {
    dev->hangle[i] = hangle + (HANGLE * (i / SENSOR_WIDTH_D)) - (HANGLE/2);
    dev->dx[i] = sin(DEG2RAD(dev->hangle[i]));
    dev->dy[i] = cos(DEG2RAD(dev->hangle[i]));
  }
  for (i = 0; i < SENSOR_HEIGHT; i++) {
    dev->vangle[i] = vangle - (VANGLE * (i / SENSOR_HEIGHT_D)) + (VANGLE/2);
    dev->dvv[i] = sin(DEG2RAD(dev->vangle[i]));
    dev->dvh[i] = cos(DEG2RAD(dev->vangle[i]));
  }
}
/**
 * The depth map is 10 meters by 10 meters.
 *
 * depthDistance[depthval] is the distance in meters.
 */

/* baseX, Y, Z are in meters.
 * hangle and vangle are in degerees.
 */
void
poll_one_device(KDevice *dev) {
  int i,j;
  uint16_t *fdepths;
  uint32_t timestamp;
  double d;
  double x, y, z;
  double ha, va;
  if (!freenect_sync_get_depth((void **) &fdepths,
                               &timestamp,
                               dev->device,
                               FREENECT_DEPTH_11BIT)) {
    // printf("\n\n\n");
    for (j = SENSOR_HEIGHT - 1; j >= 0; j--) {
      va = dev->vangle[j];
      for (i = 0; i < SENSOR_WIDTH; i++) {
        d = depthDistance[fdepths[j*SENSOR_WIDTH+i] & 0x7FF];
        d *= horizDepthMultiplier[i];
        // d *= vertDepthMultiplier[j];
        /* It's not worth plotting if it's too far away or too close. */
        if (d > 0.1 && d < 30.0) {
          ha = dev->hangle[i];

          /* We have spherical coordinates, and now we need to convert that to
           * cartesian coordiantes. */
          /* Trig approach. Needs tweaking? */
          double dh = d * dev->dvh[j];
          double dx = dev->dx[i];
          double dy = dev->dy[i];
          y = dh * dy;
          x = dh * dx;
          z = d * dev->dvv[j];

          /* Approximation approach: Broken. */
          /*
          y = (i - (SENSOR_WIDTH/2)) * (d - 10.0) * (SENSOR_WIDTH/SENSOR_HEIGHT) * 0.0021;
          z = (j - 240) * (d - 10.0) * 0.0021;
          x = d;
          */

          /* Measurements are relative to the camera's position. Now adjust
           * for the base position. */
          x += dev->baseX;
          y += dev->baseY;
          z += dev->baseZ;

          /* Now PLOT onto the depth chart! */
          double plotX = x * SCREEN_SCALE;
          double plotY = y * SCREEN_SCALE;

#define ix ((int) plotX)
#define iy ((int) plotY)
          int zindex = Z_MAP(z);

          /*
          if (!(i&0x5f) && !(j & 0x5f)) {
            printf("%d,%d:%f meters away. %f meters horizontally, %f vert",
                i, j, d, dh, z);
            printf("%d,%d:%d/%f %f meters ahead, %f meters to the side, %f meters tall.\n",
                i, j, fdepths[j*SENSOR_WIDTH+i] & 0x7FF, d,
                x, y, z);
          }
          */
          /*
          if (((i == 9) || (i == 160) || (i == 320) || (i == 480) || (i == 631)) &&
              ((j == 9) || (j == 120) || (j == 240) || (j == 360) || (j == 470))) {
            //  printf("%d,%d point is %f meters in front of camera, %f meters "
            //         "to the side, and is %f meters above the camera. "
            //         "ha: %f, va: %f\n",
            //         j, i, y, x, z, ha, va);
            printf("%d,%d: Z is %f, zindex is %d\n", j, i, z, zindex);
          }
          */

          if (Z_DRAW(iz, z)) {
            while (ix >= 0 && ix < SCREEN_WIDTH &&
                iy >= 0 && iy < SCREEN_HEIGHT) {
              if (HAS_PRIORITY(DEPTH_AT(iy, ix),zindex)) {
                DEPTH_AT(iy, ix) = zindex;
              }
              plotX += dx;
              plotY += dy;
#if DOFILL
#else
              break;
#endif
            }
          }
        }
      }
    }
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
  /* It's in the center (10m, 10m). A half meter off the ground (0.5m),
   * pointing straight forward (0.0) and resting flat and horizontal (0.0)
   */
  int i;
  for (i = 0; i < numDevices; i++) {
    poll_one_device(&devices[i]);
  }
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
    /*
    if (!(i&0x7F)) {
      printf("At distance %d (%f), meters is %f\n", i, i/k2, depthDistance[i]);
    }
    */
    /* Psychedelic:
    depthColors[i] =
        ((i & 0xF) << 20) // Red
      | ((i & 0xF0) << 8) // Green
      | ((i & 0xF00) >> 3) // Blue
      ;
    */
    /* Blue shades: */
    /*
    depthColors[i] =
          0xFF // blue
        | (0x010100 * ((2048-i)>>2));
    */
    /*
    if (!(i&0x7F)) {
      printf("At height %d, color is %.6X\n", i, depthColors[i]);
    }
    */
  }

  for (i = 0; i < 31; i++) {
    colorMap[i] =
          0xFF // blue
        | (0x101000 * ((30-i)/2));
    // printf("Z_COLOR(%d) = %.6X\n", i, colorMap[i]);
  }

  depthDistance[2047] = 0.0;

  /* Since depth is weirdly curved. */
  double angle;
  for (i = 0; i < SENSOR_WIDTH; i++) {
    angle = (HANGLE * (i / SENSOR_WIDTH_D)) - (HANGLE/2);
    horizDepthMultiplier[i] = 1.0 / cos(DEG2RAD(angle));
    /*
    if (!(i&63)) {
      printf("HorizDepthMultiplier[%d] = %f\n", i, horizDepthMultiplier[i]);
    }
    */
  }
  for (i = 0; i < SENSOR_HEIGHT; i++) {
    angle = (VANGLE * (i / SENSOR_HEIGHT_D)) - (VANGLE/2);
    vertDepthMultiplier[i] = 1.0 / cos(DEG2RAD(angle));
    /*
    if (!(i&63)) {
      printf("VertDepthMultiplier[%d] = %f\n", i, vertDepthMultiplier[i]);
    }
    */
  }

  /* Init the devices. */
  for (numDevices = 0; deviceDefinitions[numDevices].device >= 0; numDevices++);
  /* Create the devices. */
  devices = malloc(numDevices * sizeof(KDevice));
  for (i = 0; i < numDevices; i++) {
    init_one_device(&devices[i],
            deviceDefinitions[i].device,
            deviceDefinitions[i].baseX,
            deviceDefinitions[i].baseY,
            deviceDefinitions[i].baseZ,
            deviceDefinitions[i].hangle,
            deviceDefinitions[i].vangle);
  }
  return 1;
}

int
kinect_shutdown() {
  freenect_sync_stop();
  return 1;
}

int
main(int argc, char *argv[]) {
  if (!sdl_init(SCREEN_HEIGHT, SCREEN_WIDTH)) {
    return 1;
  }

  if (!kinect_init()) {
    return 1;
  }

  while (!sdl_pollevent()) {
    /* Sleep 1/10th of a second. */
    // usleep(30000);
    kinect_poll();
    draw_depths();
  }

  /* Shut down */
  kinect_shutdown();
  sdl_shutdown();
  return 0;
}
