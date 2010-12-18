/** boundary_sdl.h
 *
 * SDL-related declarations.
 */

#ifndef __BOUNDARY_SDL_H_
#define __BOUNDARY_SDL_H_

#include <SDL.h>
#include <SDL_gfxPrimitives.h>

extern SDL_Surface *screen;
extern SDL_Surface *render;

int sdl_init(int height, int width);
int sdl_pollevent();
void sdl_shutdown();
void setPixel(int x, int y, int color);

#endif /* __BOUNDARY_SDL_H_ */
