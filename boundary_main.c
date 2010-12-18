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

int depths[SCREEN_WIDTH * SCREEN_HEIGHT];
double depthDistance[2048];
int depthColors[2048];
double horizDepthMultiplier[SENSOR_WIDTH];
double vertDepthMultiplier[SENSOR_HEIGHT];

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
      setPixel(x, y, depthColors[depths[y*SCREEN_WIDTH + x]]);
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
  int x;
  ci = SCREEN_WIDTH / 2;
  cj = SCREEN_HEIGHT / 2;
  // double di, dj;
  // double ni, nj;
  int nd;
  for (j = cj+5; j < SCREEN_HEIGHT; j++) {
    for (i = 0; i < SCREEN_WIDTH; i++) {
      x = j*SCREEN_WIDTH+i;
      nd = depths[x];
      if (nd < depths[x-SCREEN_WIDTH]) {
        nd = depths[x-SCREEN_WIDTH];
      }
      depths[x] = nd;
    }
  }
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
    for (j = 0; j < SENSOR_HEIGHT; j++) {
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

          /*
          if (!(i&0x5f) && !(j & 0x5f)) {
            printf("%d,%d:%f meters away. %f meters horizontally, %f vert",
                i, j, d, dh, z);
            printf("%d,%d:%d/%f %f meters ahead, %f meters to the side, %f meters tall.\n",
                i, j, fdepths[j*SENSOR_WIDTH+i] & 0x7FF, d,
                x, y, z);
          }
          if (((i == 9) || (i == 160) || (i == 320) || (i == 480) || (i == 631)) &&
              ((j == 9) || (j == 120) || (j == 240) || (j == 360) || (j == 470))) {
            printf("%d,%d point is %f meters in front of camera, %f meters "
                   "to the side, and is %f meters above the camera. "
                   "ha: %f, va: %f\n",
                   j, i, y, x, z, ha, va);
          }
          */

          /* Measurements are relative to the camera's position. Now adjust
           * for the base position. */
          x += dev->baseX;
          y += dev->baseY;
          z += dev->baseZ;

          /* Now PLOT onto the depth chart! */
          double plotX = x * SCREEN_SCALE;
          double plotY = y * SCREEN_SCALE;
          double plotZ = z * (2048 / 2.0); /* Depth units per meter */

#define ix ((int) plotX)
#define iy ((int) plotY)
          int iz = (int) plotZ;

          if (z >= Z_MIN && z < Z_MAX) {
            while (ix >= 0 && ix < SCREEN_WIDTH &&
                iy >= 0 && iy < SCREEN_HEIGHT) {
              if (depths[iy*SCREEN_WIDTH+ix] < iz) {
                depths[iy*SCREEN_WIDTH+ix] = iz;
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
  // fillDepths(depths);
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
    /**/
    /* Blue shades: */
    depthColors[i] =
          0xFF // blue
        | (0x010100 * ((2048-i)>>2));
        /**/
    /*
    if (!(i&0x7F)) {
      printf("At height %d, color is %.6X\n", i, depthColors[i]);
    }
    */
  }
  depthDistance[2047] = 0.0;
  depthColors[2047] = 0;
  depthColors[0] = 0xFFFFFF;

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
