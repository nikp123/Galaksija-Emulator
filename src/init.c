#include <string.h>

#include "galaxy.h"
#include "system.h"
#include "graphics.h"

#include "util/file.h"

void galaxy_init_generate_charmap(galaxy_state *state) {
    for(uint16_t n=0; n<GALAXY_CHARMAP_SIZE; n++) {
        if ((n>63 && n<96) || (n>127 && n < 192)) {
            state->charmap[n]=n-64;
        } else if (n > 191) {
            state->charmap[n]=n-128;
        } else {
            state->charmap[n]=n;
        }
    }
}

void galaxy_reset(galaxy_state *state) {
    Z80RESET(&state->context);

    // reset ram
    for (uint32_t n=GALAXY_RAM_ADDR_START; n<GALAXY_RAM_ADDR_END; n++)
        state->memory[n]=0;

    // Screen
#ifdef GALAXY_SCREEN_TEST_ENABLE
    for(uint16_t n=GALAXY_SCREEN_ADDR_START; n<GALAXY_SCREEN_ADDR_END; n++)
        state->memory[n]=' '+n;
#else
    memset(&state->memory[GALAXY_SCREEN_ADDR_START], 0xFF, GALAXY_SCREEN_ADDR_END-GALAXY_SCREEN_ADDR_START);
#endif

    // reset 2nd-ary char framebuffer
    memset(state->char_framebuffer, 0x00, GALAXY_CHAR_COUNT*sizeof(uint8_t));

    // Keyboard
    memset(&state->memory[GALAXY_KEYBOARD_ADDR_START], 0xFF, GALAXY_KEYBOARD_ADDR_END-GALAXY_KEYBOARD_ADDR_START);
}

bool galaxy_init(galaxy_state *state) {
    char temp[4096];

    if(state->config.firmware_path == NULL) {
        state->error = GALAXY_FIRMWARE_PATH_INVALID;
        return false;
    }

    snprintf(temp, 4096, "%s/CHRGEN.BIN", state->config.firmware_path);
    if(!util_file_exists(temp)) {
        state->error = GALAXY_FIRMWARE_CHARGEN_MISSING;
        return false;
    }
    if(util_file_size(temp) != GALAXY_CHARGEN_SIZE) {
        state->error = GALAXY_FIRMWARE_CHARGEN_INVALID;
        return false;
    }
    if(util_file_to_existing_buffer_unsafe(temp, state->chargen) == 0) {
        state->error = GALAXY_FIRMWARE_CHARGEN_READ_FAILURE;
        return false;
    }

    snprintf(temp, 4096, "%s/ROM1.BIN", state->config.firmware_path);
    if(!util_file_exists(temp)) {
        state->error = GALAXY_FIRMWARE_ROM1_MISSING;
        return false;
    }
    if(util_file_size(temp) != GALAXY_ROM1_SIZE) {
        state->error = GALAXY_FIRMWARE_ROM1_INVALID;
        return false;
    }
    if(util_file_to_existing_buffer_unsafe(temp, state->memory) == 0) {
        state->error = GALAXY_FIRMWARE_ROM1_READ_FAILURE;
        return false;
    }

    snprintf(temp, 4096, "%s/ROM2.BIN", state->config.firmware_path);
    if(!util_file_exists(temp)) {
        state->error = GALAXY_FIRMWARE_ROM2_MISSING; // acts as a warning beyond anything else
    } else if(util_file_size(temp) != GALAXY_ROM2_SIZE) {
        state->error = GALAXY_FIRMWARE_ROM2_INVALID;
        return false;
    } else if(util_file_to_existing_buffer_unsafe(temp, &state->memory[GALAXY_ROM2_START]) == 0) {
        state->error = GALAXY_FIRMWARE_ROM2_READ_FAILURE;
        return false;
    }

    /**
     * TODO: this
     * // GAL_PLUS
     * if((rom=fopen("GAL_PLUS.BIN", "rb"))!=NULL) {
     *  fread(&MEMORY[0xE000], 1, 4096, rom);
     *  fclose(rom);
     * } else {
     *  printf("GAL_PLUS.BIN nije prisutan, idemo dalje bez njega!\n");
     *  memset(&MEMORY[0xE000], 0xff, 4096);
     * }
    **/

    // TODO: check whether clearing further memory is necessary

    galaxy_init_generate_charmap(state);

    if(galaxy_graphics_colors_generate(state) == false)
        return false;

    if(galaxy_graphics_generate_bitmap_font(state) == false)
        return false;

    if(state->config.cpu_speed < 1)
        state->config.cpu_speed = GALAXY_CPU_FREQUENCY;

    if(state->config.framerate < 1)
        state->config.framerate = GALAXY_FRAMERATE;

    state->context.memParam = (size_t)state;
    state->context.memRead = (Z80DataIn)galaxy_mem_read;
    state->context.memWrite = (Z80DataOut)galaxy_mem_write;

    state->context.ioParam = (size_t)state;
    state->context.ioRead  = (Z80DataIn)galaxy_io_read;
    state->context.ioWrite = (Z80DataOut)galaxy_io_write;

    galaxy_reset(state);

    return true;
}
