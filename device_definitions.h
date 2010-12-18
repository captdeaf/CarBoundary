/* device_definitions.h
 * 
 * This contains constants and the device definitions for all attached
 * Kinect devices.
 */

/* Pixels of height for SDL screen. */
#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 512

/* Do we radially fill in everything after a pixel lands?
 * It's currently extremely CPU-intensive.
 * If so, DOFILL is 1. */
#define DOFILL 1

/* Scale of pixels per meter.
 * This ensures that the display, whatever it is, is 10 meters tall. */
#define SCREEN_SCALE (SCREEN_HEIGHT / 10.0)

/* Minimum and maximum Z to be plotted, in meters.
 * Indoors, a Z_MAX of 1.5 is recommended. Outdoors, a Z_MAX taller than
 * the car is recommended.
 */
#define Z_MIN 0.0
#define Z_MAX 1.5

/* This list _must_ end in a device id < 0. */
/* <device id>, <baseX>, <baseY>, <baseZ>, <horiz angle>, <vert angle> */
struct _kdevice_definition_ deviceDefinitions[] = {
  // {0,  5.0, 5.0, 0.7874, 00.0, 0.0},
  {0,  2.0, 5.0, 0.7874, 30.0, 0.0},
  {0,  8.0, 5.0, 0.7874, -30.0, 0.0},
  {-1, 0.0, 0.0, 0.0,    0.0, 0.0},
};

/* Constants for the Kinect devices. */
#define SENSOR_WIDTH 640
#define SENSOR_WIDTH_D 640.0
#define SENSOR_HEIGHT 480
#define SENSOR_HEIGHT_D 480.0

#define VANGLE 43.0
#define HANGLE 57.0
