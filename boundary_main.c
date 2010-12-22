/** boundary_main.c
 *
 * Main source file for CarBoundary.
 */

#include <SDL.h>
#include <SDL_gfxPrimitives.h>
#include "libfreenect_sync.h"
#include <math.h>
#include "boundary_sdl.h"

#include "device_definitions.h"

FP_TYPE depthDistance[2048];
FP_TYPE horizDepthMultiplier[SENSOR_WIDTH];
FP_TYPE vertDepthMultiplier[SENSOR_HEIGHT];

typedef struct _cube_ {
  char known_by; // Which device knows this is here?
  char cleared_by; // Which device has cleared this?
  char guess_count; // How many devices have guessed this?
  char known_count; // How many devices have guessed this?
} Cube;

Cube environmentCubes[CUBE_HEIGHT*CUBE_LENGTH*CUBE_WIDTH];
#define CUBELOC(x,y,z) ((z*CUBE_WIDTH*CUBE_LENGTH)+(y*CUBE_WIDTH)+x)
#define CUBEAT(loc) environmentCubes[loc]
#define CUBE(x,y,z) environmentCubes[CUBELOC(x,y,z)]

#define COL_CLEAR '\0'
#define COL_KNOWN 'k'
#define COL_GUESSED 'g'
char columnStatus[CUBE_LENGTH*CUBE_WIDTH];
#define COLSTAT(x,y) columnStatus[y*CUBE_WIDTH + x]

/* Color map. This is the color of the cube given. */
int cubeColor[20];

typedef struct _kdevice_ {
  int device;
  char id;
  FP_TYPE vangle[SENSOR_HEIGHT];
  FP_TYPE hangle[SENSOR_WIDTH];
  FP_TYPE dvv[SENSOR_HEIGHT];
  FP_TYPE dvh[SENSOR_HEIGHT];
  FP_TYPE dx[SENSOR_WIDTH];
  FP_TYPE dy[SENSOR_WIDTH];
  FP_TYPE baseX, baseY, baseZ;
} KDevice;

KDevice *devices;
int numDevices;

void
draw_depths() {
  int x,y,z;
  int sx, sy;
  int zmax;
  int loc;
  SDL_LockSurface(render);
  for (sx = 0; sx < SCREEN_WIDTH; sx++) {
    x = (int) (((FP_TYPE) sx / (FP_TYPE) SCREEN_WIDTH) * (FP_TYPE) CUBE_WIDTH);
    for (sy = 0; sy < SCREEN_HEIGHT; sy++) {
      y = (int) (((FP_TYPE) sy / (FP_TYPE) SCREEN_HEIGHT) * (FP_TYPE) CUBE_LENGTH);
      if (x < CAR_CUBE_LEFT || x > CAR_CUBE_RIGHT ||
          y < CAR_CUBE_REAR || y > CAR_CUBE_FRONT) {
        if (COLSTAT(x,y) == COL_KNOWN) {
          zmax = -1;
          for (z = 0; z < CUBE_HEIGHT; z++) {
            loc = CUBELOC(x, y, z);
            if ((CUBEAT(loc).known_count >= MIN_COUNT) ||
                ((CUBEAT(loc).guess_count >= MIN_COUNT) &&
                 !CUBEAT(loc).cleared_by)) {
              zmax = z;
            }
          }
          if (zmax >= 0 && zmax < CUBE_HEIGHT) {
            setPixel(sx, sy, cubeColor[zmax]);
          } else {
            setPixel(sx, sy, NULL_COLOR);
          }
        } else if (COLSTAT(x,y) == COL_GUESSED) {
          zmax = -1;
          for (z = 0; z < CUBE_HEIGHT; z++) {
            loc = CUBELOC(x, y, z);
            if ((CUBEAT(loc).known_count >= MIN_COUNT) ||
                ((CUBEAT(loc).guess_count >= MIN_COUNT) &&
                 !CUBEAT(loc).cleared_by)) {
              zmax = z;
            }
          }
          if (zmax >= 0 && zmax < CUBE_HEIGHT) {
            setPixel(sx, sy, cubeColor[zmax]);
          } else {
            setPixel(sx, sy, NULL_COLOR);
          }
        } else {
          /* If it's non-existant, set the pixel cleanly. */
          setPixel(sx, sy, NULL_COLOR);
        }
      } else {
        setPixel(sx, sy, 0x006600);
      }
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
                FP_TYPE baseX, FP_TYPE baseY, FP_TYPE baseZ,
                FP_TYPE hangle, FP_TYPE vangle) {
  int i;
  dev->device = device;
  dev->id = '0' + device; /* quick and dirty 'id' for known_by. */
  dev->baseX = baseX;
  dev->baseY = baseY;
  dev->baseZ = baseZ;

  for (i = 0; i < SENSOR_WIDTH; i++) {
    dev->hangle[i] = hangle + (HANGLE * (i / SENSOR_WIDTH_D)) - (HANGLE/2);
    dev->dx[i] = FP_SIN(DEG2RAD(dev->hangle[i]));
    dev->dy[i] = FP_COS(DEG2RAD(dev->hangle[i]));
  }
  for (i = 0; i < SENSOR_HEIGHT; i++) {
    dev->vangle[i] = vangle - (VANGLE * (i / SENSOR_HEIGHT_D)) + (VANGLE/2);
    dev->dvv[i] = FP_SIN(DEG2RAD(dev->vangle[i]));
    dev->dvh[i] = FP_COS(DEG2RAD(dev->vangle[i]));
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
  FP_TYPE d;
  FP_TYPE landX, landY, landZ;
  FP_TYPE ha, va;
  if (!freenect_sync_get_depth((void **) &fdepths,
                               &timestamp,
                               dev->device,
                               FREENECT_DEPTH_11BIT)) {
    // printf("\n\n\n");
    for (j = SENSOR_HEIGHT - 1; j >= 0; j--) {
    // for (j = SENSOR_HEIGHT/2 ; j ; j = 0) {
      va = dev->vangle[j];
      for (i = 0; i < SENSOR_WIDTH; i++) {
      // for (i = SENSOR_WIDTH / 2 ; i ; i = 0) {
        d = depthDistance[fdepths[j*SENSOR_WIDTH+i] & 0x7FF];
        d *= horizDepthMultiplier[i];
        d *= vertDepthMultiplier[j];
        /* It's not worth plotting if it's too far away or too close. */
        if (d > 0.1 && d < 30.0) {
          ha = dev->hangle[i];

          /* We have spherical coordinates, and now we need to convert that to
           * cartesian coordiantes. */
          /* Trig approach. Needs tweaking? */
          FP_TYPE dh = d * dev->dvh[j];
          FP_TYPE dx = dev->dx[i];
          FP_TYPE dy = dev->dy[i];
          FP_TYPE dz = dev->dvv[j];
          landY = dh * dy;
          landX = dh * dx;
          landZ = d * dz;

          /* Approximation approach: Broken. */
          /*
          y = (i - (SENSOR_WIDTH/2)) * (d - 10.0) * (SENSOR_WIDTH/SENSOR_HEIGHT) * 0.0021;
          z = (j - 240) * (d - 10.0) * 0.0021;
          x = d;
          */

          /* Measurements are relative to the camera's position. Now adjust
           * for the base position. */
          landX += dev->baseX;
          landY += dev->baseY;
          landZ += dev->baseZ;

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

          /* Current x/y/z. */
          FP_TYPE cx = dev->baseX;
          FP_TYPE cy = dev->baseY;
          FP_TYPE cz = dev->baseZ;

          // Plot everything onto the cube scale.
          landX *= CUBE_XSCALE;
          landY *= CUBE_YSCALE;
          landZ *= CUBE_ZSCALE;

          cx *= CUBE_XSCALE;
          cy *= CUBE_YSCALE;
          cz *= CUBE_ZSCALE;

          int xt = cx <= landX;
          int yt = cy <= landY;

          // ix, iy and iz are cube coordinates of cx,cy,cz
#define ix ((int) cx)
#define iy ((int) cy)
#define iz ((cz < 0) ? 0 : (cz > CUBE_HEIGHT) ? (CUBE_HEIGHT-1) : (int) cz)

#define INBOUNDS(x,y,z) ( \
                 (x < CUBE_WIDTH) && \
                 (x >= 0.0) && \
                 (y < CUBE_LENGTH) && \
                 (y >= 0.0) && \
                 (z < CUBE_HEIGHT) && \
                 (z >= 0.0))
          // printf("LandX,y,z: %d,%d,%d\n", (int) landX, (int) landY, (int) landZ);
          /* Step 1: Mark all items between basex/y/z and landing x/y/z. */
          while ((((cx <= landX) == xt) &&
                 ((cy <= landY) == yt)) &&
                 INBOUNDS(cx,cy,cz)) {
            // printf("cx <= landX: %d (xt: %d)\n", cx <= landX, xt);
            // printf("cy <= landY: %d (yt: %d)\n", cy <= landY, yt);
            // printf("Clearing x,y,z: %d,%d,%d\n", ix, iy, iz);
            CUBE(ix, iy, iz).cleared_by = dev->id;
            /* Advance to next cube. */
            cx += dx; cy += dy; cz += dz;
          }

          if (INBOUNDS(cx, cy, cz)) {
            /* Step 2: Mark the cube this lands in as known, as well as its
             * column status. */
            // printf("Knowing x,y,z: %d,%d,%d\n", ix, iy, iz);
            cx = landX;
            cy = landY;
            cz = landZ;
            CUBE(ix, iy, iz).known_by = dev->id;
            COLSTAT(ix, iy) = COL_KNOWN;
            CUBE(ix, iy, iz).known_count++;
            CUBE(ix, iy, iz).known_by = dev->id;
          // } else {
            // printf("Landed out of bounds?\n");
          }

          /* Advance to next cube. */
          cx += dx; cy += dy; cz += dz;

          /* Step 3: Mark every cube in a line _after_ known cube as 'guessed'
           */
          while (INBOUNDS(cx, cy, cz)) {
            // printf("Guessing x,y,z: %d,%d,%d\n", ix, iy, iz);
            CUBE(ix, iy, iz).guess_count++;
            /* Advance to next cube. */
            if (COLSTAT(ix, iy) == COL_CLEAR) {
              COLSTAT(ix, iy) = COL_GUESSED;
            }
            cx += dx; cy += dy; cz += dz;
          }
        }
      }
    }
  }
}

void
kinect_poll() {
  /* Clear depth map out */
  memset(environmentCubes, 0, sizeof(environmentCubes));
  memset(columnStatus, 0, sizeof(columnStatus));
  /*
    poll_one_device(int device,
                    FP_TYPE baseX, FP_TYPE baseY, FP_TYPE baseZ,
                    FP_TYPE hangle, FP_TYPE vangle);
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
  const FP_TYPE k1 = 0.1236;
  const FP_TYPE k2 = 2842.5;
  const FP_TYPE k3 = 1.1863;
  const FP_TYPE k4 = 0.0370;

  /* Populate depthDistance[i], in meters. */
  for (i = 1; i < 2047; i++) {
    depthDistance[i] = k1 * FP_TAN((i/k2) + k3) - k4;
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

  for (i = 0; i < CUBE_HEIGHT; i++) {
    cubeColor[i] =
          0xFF // blue
        | ((0x0101 * ((256 * (CUBE_HEIGHT - i)) / CUBE_HEIGHT)) << 8);
    // printf("Z_COLOR(%d) = %.6X\n", i, cubeColor[i]);
  }

  depthDistance[2047] = 0.0;

  /* Since depth is weirdly curved. */
  FP_TYPE angle;
  for (i = 0; i < SENSOR_WIDTH; i++) {
    angle = (HANGLE * (i / SENSOR_WIDTH_D)) - (HANGLE/2);
    horizDepthMultiplier[i] = 1.0 / FP_COS(DEG2RAD(angle));
    /*
    if (!(i&63)) {
      printf("HorizDepthMultiplier[%d] = %f\n", i, horizDepthMultiplier[i]);
    }
    */
  }
  for (i = 0; i < SENSOR_HEIGHT; i++) {
    angle = (VANGLE * (i / SENSOR_HEIGHT_D)) - (VANGLE/2);
    vertDepthMultiplier[i] = 1.0 / FP_COS(DEG2RAD(angle));
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
