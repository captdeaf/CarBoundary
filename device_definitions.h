/* device_definitions.h
 * 
 * This contains constants and the device definitions for all attached
 * Kinect devices.
 */

/* Pixels of height for SDL screen. */
#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT SCREEN_WIDTH

/* Size of environment scanner in meters. */
/* 15 meters from front to back, 10 from left to right. 2 meters top-bottom */
#define ENV_WIDTH  12
#define ENV_LENGTH 15
#define ENV_HEIGHT 2

/* Car information: For rendering a car on the screen. Units are in meters.
 */
#define CAR_LEFT 5.0
#define CAR_RIGHT 7.0
#define CAR_TOP 10.0
#define CAR_BOTTOM 5.0

/* Do we radially fill in everything after a pixel lands?
 * It's currently extremely CPU-intensive.
 * If so, DOFILL is 1. */
#define DOFILL 1

/* Scale of pixels per meter.
 * This ensures that the display, whatever it is, is 15 meters tall. */
#define SCREEN_SCALE (SCREEN_HEIGHT / 15.0)

/* Converts a height-in-meters to an integer representation, which is
 * used by the color generator.
 *
 * Currently: 10 centimeters per int.
 */
#define Z_MAP(height) (int) round(height * 10.0)

/* Z_COLOR takes a depth number as returned by Z_MAP and returns
 * a 0xRRGGBB */
/* This Z_COLOR returns light red for below-ground-level, light green for
 * above-ceiling-level and a shade of blue for hazard level. */
#define Z_COLOR(z) ((z < 0) ? 0xFFDDDD : (z > 20) ? 0xDDFFDD : colorMap[z])

/* Priority of height. This is currently only used to determine if a ceiling
 * is being drawn. */
#define HAS_PRIORITY(oldz, newz) (1)
// #define HASPRIORITY(oldz, newz) ((newz > 20) ? (oldz == 0) : (newz > oldz))

/* Minimum and maximum Z to be plotted, in meters.
 * Indoors, a Z_MAX of 1.5 is recommended. Outdoors, a Z_MAX taller than
 * the car is recommended.
 *
 * Z_MIN will only be useful when we figure out how to display potholes :D.
 */
#define Z_MIN 0.0
#define Z_MAX 1.6

/* do we draw something? Takes both a z index and a map height. */
#define Z_DRAW(z, height) ((Z_MIN < height) && (Z_MAX >= height))

/* This list _must_ end in a device id < 0. */
/* <device id>, <baseX>, <baseY>, <baseZ>, <horiz angle>, <vert angle> */
struct _kdevice_definition_ deviceDefinitions[] = {
  // Default: One.
  // {0,  6.0, CAR_TOP, 0.7874, 0.0, 0.0},
  // Two, pointing straight ahead.
  {0,  CAR_LEFT, CAR_TOP, 0.7874, 0.0, 0.0},
  {0,  CAR_RIGHT, CAR_TOP, 0.7874, 0.0, 0.0},
  // Two, slightly angled at each other.
  // {0,  CAR_LEFT, CAR_TOP, 0.7874, 10.0, 0.0},
  // {0,  CAR_RIGHT, CAR_TOP, 0.7874, 350.0, 0.0},
  // Two, pointing in reverse.
  // {0,  CAR_LEFT, CAR_BOTTOM, 0.7874, 180.0, 0.0},
  // {0,  CAR_RIGHT, CAR_BOTTOM, 0.7874, 180.0, 0.0},
  {-1, 0.0, 0.0, 0.0,    0.0, 0.0},
};

/* Constants for the Kinect devices. */
#define SENSOR_WIDTH 640
#define SENSOR_WIDTH_D 640.0
#define SENSOR_HEIGHT 480
#define SENSOR_HEIGHT_D 480.0

#define VANGLE 43.0
#define HANGLE 57.0

/* CUBE_SCALE: Cubes per meter. */
#define CUBE_XSCALE 30
#define CUBE_YSCALE 30
#define CUBE_ZSCALE 30

/* CUBE_HEIGHT: # of cubes of height. Anything below get stuck to bottom,
 * anything above gets ignored. */
// 2 meters from bottom to top.
#define CUBE_HEIGHT (ENV_HEIGHT * CUBE_ZSCALE)
/* CUBE_WIDTH: # of cubes from left to right our environment is.
 * 15 meters == 150 cubes. */
#define CUBE_WIDTH (ENV_WIDTH * CUBE_XSCALE)
/* CUBE_LENGTH: # of cubes from front to back.
 * 20 meters == 200 cubes. */
#define CUBE_LENGTH (ENV_LENGTH * CUBE_YSCALE)

#define MIN_COUNT 3

// Car cube counts.
#define CAR_CUBE_LEFT (int) (CAR_LEFT * CUBE_XSCALE)
#define CAR_CUBE_RIGHT (int) (CAR_RIGHT * CUBE_XSCALE)
#define CAR_CUBE_TOP (int) (CAR_TOP * CUBE_YSCALE)
#define CAR_CUBE_BOTTOM (int) (CAR_BOTTOM * CUBE_YSCALE)
