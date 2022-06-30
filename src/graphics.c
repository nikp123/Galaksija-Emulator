#include <string.h>

#include "shared.h"

bool galaxy_graphics_colors_generate(galaxy_state *state) {
    if(state->config.default_colors == true) {
        switch(state->config.graphics_mode) {
            case GALAXY_GRAPHICS_MODE_RGBX8888:
                state->config.foreground = GALAXY_GRAPHICS_MODE_RGBX8888_DEFAULT_FOREGROUND;
                state->config.background = GALAXY_GRAPHICS_MODE_RGBX8888_DEFAULT_BACKGROUND;
                break;
            default:
                state->error = GALAXY_COLORGEN_PIXEL_FORMAT_INVALID;
                return false;
                break;
        }
    }
    return true;
}

bool galaxy_graphics_generate_bitmap_font(galaxy_state *state) {
    uint32_t *pixel = (uint32_t*)state->font_bitmap; // if bugs look here
            // reason being: mapping 2d array to a 1d pointer

    switch(state->config.graphics_mode) {
    case GALAXY_GRAPHICS_MODE_RGBX8888:
        for(uint16_t i=0; i < GALAXY_FONT_COUNT; i++) {
            for(uint8_t j=0; j < GALAXY_FONT_HEIGHT; j++) {
                uint16_t offset = (j<<7) | i;

                uint8_t l = state->chargen[offset];

                for (uint8_t x=0; x<GALAXY_FONT_WIDTH; x++) {
                    *pixel = !(l&(1<<x)) ? state->config.foreground :
                                           state->config.background;
                    pixel++;
                }

                // pitch shifting (if needed) goes here
            }
        }
        break;
    default:
        state->error = GALAXY_CHARGEN_PIXEL_FORMAT_INVALID;
        return false;
        break;
    }

    return true;
}

bool galaxy_graphics_screen_clear(galaxy_state *state, void *framebuffer) {
    switch(state->config.graphics_mode) {
        case GALAXY_GRAPHICS_MODE_RGBX8888:
            for(uint32_t i = 0; i < GALAXY_PIXEL_COUNT; i++) {
                ((uint32_t*)framebuffer)[i] = state->config.background;
            }
            break;
        default:
            state->error = GALAXY_CLEAR_SCREEN_PIXEL_FORMAT_INVALID;
            return false;
    }

    return true;
}

bool galaxy_draw(galaxy_state *state, void *framebuffer) {
    int offset;
    switch(state->config.graphics_mode) {
        case GALAXY_GRAPHICS_MODE_RGBX8888: {
            uint32_t *fb = (uint32_t*)framebuffer;
            for(uint32_t i = 0; i < GALAXY_VERTICAL_CHARS; i++) {
                for(uint32_t j = 0; j < GALAXY_HORIZONTAL_CHARS; j++) {
                    char character = state->charmap[
                        state->memory[GALAXY_SCREEN_ADDR_START+i*GALAXY_HORIZONTAL_CHARS + j]];
                    for(uint32_t k = 0; k < GALAXY_FONT_HEIGHT; k++) {
                        memcpy(fb, &state->font_bitmap[(character&0x7F)][k], sizeof(uint32_t)*GALAXY_FONT_WIDTH);
                        fb += GALAXY_HORIZONTAL_RESOLUTION;
                    }
                    fb -= (GALAXY_HORIZONTAL_RESOLUTION * GALAXY_FONT_HEIGHT) - GALAXY_FONT_WIDTH;
                }
                fb += GALAXY_HORIZONTAL_RESOLUTION * (GALAXY_FONT_HEIGHT-1);
            }
            break;
        }
        default:
            state->error = GALAXY_DRAW_PIXEL_FORMAT_INVALID;
            return false;
    }

    return true;
}

