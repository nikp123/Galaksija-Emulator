#ifndef GALAXY_SHARED_H_
#define GALAXY_SHARED_H_

#include <stdint.h>
#include <stdbool.h>

#include "../inc/libz80/z80.h"

#include "util/endianess.h"

#include "system.h"
#include "error.h"

#ifdef UTIL_ENDIANESS_LITTLE
    #define GALAXY_GRAPHICS_MODE_RGBX8888_DEFAULT_FOREGROUND 0xffffffff
    #define GALAXY_GRAPHICS_MODE_RGBX8888_DEFAULT_BACKGROUND 0xff003535
#else
    #error "Not implemented!"
#endif

typedef enum {
    GALAXY_GRAPHICS_MODE_RGBX8888,
    GALAXY_GRAPHICS_MODE_RGB888,
} galaxy_graphics_pixel_mode;

typedef uint32_t galaxy_character[GALAXY_FONT_HEIGHT][GALAXY_FONT_WIDTH];

typedef struct {
    uint64_t                   framerate;
    uint64_t                   cpu_speed;
    const bool                 default_colors;
    uint64_t                   background;  // single pixel (no 10bit unfortunately)
    uint64_t                   foreground;  // single pixel
    const char*                firmware_path;
    const char*                system_state_file;
    const galaxy_graphics_pixel_mode graphics_mode;
} galaxy_config;

typedef struct {
    Z80Context       context;
    uint8_t          memory[GALAXY_ADDRESS_SPACE];
    uint8_t          chargen[GALAXY_CHARGEN_SIZE];
    uint8_t          charmap[GALAXY_CHARMAP_SIZE];
    galaxy_character font_bitmap[GALAXY_FONT_COUNT];

    galaxy_config config;
    galaxy_error  error;
} galaxy_state;

bool galaxy_run_frame(galaxy_state *state, void *framebuffer);
bool galaxy_init(galaxy_state *state);
void galaxy_reset(galaxy_state *state);
void galaxy_trigger_nmi(galaxy_state *state);

byte galaxy_mem_read(galaxy_state *state, ushort address);
void galaxy_mem_write(galaxy_state *state, ushort address, byte data);
byte galaxy_io_read(galaxy_state *state, ushort address);
void galaxy_io_write(galaxy_state *state, ushort address, byte data);

bool galaxy_load_state(galaxy_state *state, const char *filename);
bool galaxy_save_state(galaxy_state *state, const char *filename);

#endif

