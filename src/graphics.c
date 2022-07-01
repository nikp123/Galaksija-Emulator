#include <string.h>

#include "galaxy.h"

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

static inline void galaxy_draw_char_RGBX8888(uint32_t *corrected_framebuffer,
                                galaxy_character *galaxy_char_bitmap) {
    #if GALAXY_FONT_WIDTH != 8
        #error "Incompatible draw!"
    #endif
    #if GALAXY_FONT_HEIGHT != 13
        #error "Incompatible draw!"
    #endif

    for(int i = 0; i < 13; i++) {
        (*corrected_framebuffer++) = (*galaxy_char_bitmap)[i][0];
        (*corrected_framebuffer++) = (*galaxy_char_bitmap)[i][1];
        (*corrected_framebuffer++) = (*galaxy_char_bitmap)[i][2];
        (*corrected_framebuffer++) = (*galaxy_char_bitmap)[i][3];
        (*corrected_framebuffer++) = (*galaxy_char_bitmap)[i][4];
        (*corrected_framebuffer++) = (*galaxy_char_bitmap)[i][5];
        (*corrected_framebuffer++) = (*galaxy_char_bitmap)[i][6];
        (*corrected_framebuffer++) = (*galaxy_char_bitmap)[i][7];
        corrected_framebuffer += GALAXY_HORIZONTAL_RESOLUTION - GALAXY_FONT_WIDTH;
    }
}

bool galaxy_draw(galaxy_state *state, void *framebuffer) {
    int offset;
    switch(state->config.graphics_mode) {
        case GALAXY_GRAPHICS_MODE_RGBX8888: {
            for(uint16_t c = 0; c < GALAXY_CHAR_COUNT; c++) {
                uint8_t raw_char = state->memory[GALAXY_SCREEN_ADDR_START+c];

                // skip same characters
                if(state->char_framebuffer[c] == raw_char)
                    continue;

                uint32_t *fb = (uint32_t*)framebuffer;
                uint16_t i = c&0x1f;
                uint16_t j = c>>5;

                fb += GALAXY_HORIZONTAL_RESOLUTION * GALAXY_FONT_HEIGHT * j;
                fb += GALAXY_FONT_WIDTH * i;

                uint8_t conv_char = state->charmap[raw_char];

                galaxy_draw_char_RGBX8888(fb, &state->font_bitmap[(conv_char&0x7F)]);
            }
            break;
        }
        default:
            state->error = GALAXY_DRAW_PIXEL_FORMAT_INVALID;
            return false;
    }

    // assume that the buffer has been copied over
    memcpy(state->char_framebuffer, &state->memory[GALAXY_SCREEN_ADDR_START],
            GALAXY_CHAR_COUNT*sizeof(uint8_t));

    return true;
}

