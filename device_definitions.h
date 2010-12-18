/* device_definitions.h
 * 
 * This contains constants and the device definitions for all attached
 * Kinect devices.
 */

/* Pixels of height for SDL screen. */
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT SCREEN_WIDTH

/* Do we radially fill in everything after a pixel lands?
 * It's currently extremely CPU-intensive.
 * If so, DOFILL is 1. */
#define DOFILL 1

/* Scale of pixels per meter.
 * This ensures that the display, whatever it is, is 10 meters tall. */
#define SCREEN_SCALE (SCREEN_HEIGHT / 10.0)

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
#define HASPRIORITY(oldz, newz) (newz > oldz)
// #define HASPRIORITY(oldz, newz) ((newz > 20) ? (oldz == 0) : (newz > oldz))

/* Minimum and maximum Z to be plotted, in meters.
 * Indoors, a Z_MAX of 1.5 is recommended. Outdoors, a Z_MAX taller than
 * the car is recommended.
 *
 * Z_MIN will only be useful when we figure out how to display potholes :D.
 */
#define Z_MIN 0.0
#define Z_MAX 2.0

/* do we draw something? Takes both a z index and a map height. */
#define Z_DRAW(z, height) ((Z_MIN < height) && (Z_MAX >= height))

/* This list _must_ end in a device id < 0. */
/* <device id>, <baseX>, <baseY>, <baseZ>, <horiz angle>, <vert angle> */
struct _kdevice_definition_ deviceDefinitions[] = {
  {0,  5.0, 5.0, 0.7874, 00.0, 0.0},
  // {0,  2.0, 5.0, 0.7874, 30.0, 0.0},
  // {0,  8.0, 5.0, 0.7874, -30.0, 0.0},
  {-1, 0.0, 0.0, 0.0,    0.0, 0.0},
};

/* Constants for the Kinect devices. */
#define SENSOR_WIDTH 640
#define SENSOR_WIDTH_D 640.0
#define SENSOR_HEIGHT 480
#define SENSOR_HEIGHT_D 480.0

#define VANGLE 43.0
#define HANGLE 57.0
