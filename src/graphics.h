#ifndef GALAXY_GRAPHICS_H_
#define GALAXY_GRAPHICS_H_

#include <stdint.h>
#include <stdbool.h>

#include "galaxy.h"

// TODO: docs
extern bool galaxy_graphics_generate_bitmap_font(galaxy_state *state);

// TODO: docs
extern bool galaxy_graphics_colors_generate(galaxy_state *state);

// TODO: docs
extern bool galaxy_draw(galaxy_state *state, void *framebuffer);

// TODO: docs
extern bool galaxy_graphics_screen_clear(galaxy_state *state, void *framebuffer);

#endif

