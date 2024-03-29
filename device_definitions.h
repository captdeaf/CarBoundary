/* device_definitions.h
 * 
 * This contains constants and the device definitions for all attached
 * Kinect devices.
 */

/* Pixels of height for SDL screen. */
#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 400

/* Size of environment scanner in meters. */
/* 15 meters from front to back, 10 from left to right. 2 meters top-bottom */
#define ENV_WIDTH  12
#define ENV_LENGTH 15
#define ENV_HEIGHT 2

/* Car information: For rendering a car on the screen. Units are in meters.
 */
// The car is 64 inches wide.
#define CAR_LEFT (6.0 - 0.8128)
#define CAR_RIGHT (6.0 + 0.8128)

// And 175 inches long.
#define CAR_FRONT (7.5 + 2.2225)
#define CAR_REAR (7.5 - 2.2225)

// Car top is 59 inches (1.4986 meters) above the ground.
#define CAR_TOP 1.4986

// Testing stuff.
#define TABLE_TOP 0.7874
#define BOX_TOP 0.4318

#define FP_TYPE double
#define FP_COS cos
#define FP_SIN sin
#define FP_TAN tan

struct _kdevice_definition_ {
  int device;
  FP_TYPE baseX;
  FP_TYPE baseY;
  FP_TYPE baseZ;
  FP_TYPE hangle;
  FP_TYPE vangle;
};
/* This list _must_ end in a device id < 0. */
/* <device id>, <baseX>, <baseY>, <baseZ>, <horiz angle>, <vert angle> */
struct _kdevice_definition_ deviceDefinitions[] = {
	// Placed on top of my car.
  // The cameras are 40 inches (1.016m) apart
	// They are on my car's roof, 81 inches behind the front of the car.
  {1,  6.0 - 0.508, CAR_FRONT - 2.05, CAR_TOP, 0.0, 0.0},
  {0,  6.0 + 0.508, CAR_FRONT - 2.05, CAR_TOP, 0.0, 0.0},
  // One, on a box, pointing straight ahead.
  // {0,  6.0, CAR_FRONT, BOX_TOP, 0.0, 0.0},
  // Two, on boxes, pointing straight ahead.
  // {1,  5.25, CAR_FRONT, BOX_TOP, 0.0, 0.0},
  // {0,  6.75, CAR_FRONT, BOX_TOP, 0.0, 0.0},
  // Default: One, on table top.
  // {0,  6.0, CAR_FRONT, TABLE_TOP, 0.0, 0.0},
  // Two, pointing straight ahead.
  // {0,  CAR_LEFT, CAR_FRONT, TABLE_TOP, 0.0, 0.0},
  // {0,  CAR_RIGHT, CAR_FRONT, TABLE_TOP, 0.0, 0.0},
  // Two, slightly angled at each other.
  // {0,  CAR_LEFT, CAR_FRONT, TABLE_TOP, 10.0, 0.0},
  // {0,  CAR_RIGHT, CAR_FRONT, TABLE_TOP, 350.0, 0.0},
  // Two, pointing in reverse.
  // {0,  CAR_LEFT, CAR_REAR, TABLE_TOP, 180.0, 0.0},
  // {0,  CAR_RIGHT, CAR_REAR, TABLE_TOP, 180.0, 0.0},
  {-1, 0.0, 0.0, 0.0,    0.0, 0.0},
};

/* What to color areas not detected by the sensors? */
#define NULL_COLOR 0xFFFFCC

/* How many pixels should recognize a cube before it's recognized? */
#define MIN_COUNT 5

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
#define CUBE_ZSCALE 10

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

// Car cube counts.
#define CAR_CUBE_LEFT (int) (CAR_LEFT * CUBE_XSCALE)
#define CAR_CUBE_RIGHT (int) (CAR_RIGHT * CUBE_XSCALE)
#define CAR_CUBE_FRONT (int) (CAR_FRONT * CUBE_YSCALE)
#define CAR_CUBE_REAR (int) (CAR_REAR * CUBE_YSCALE)
