#include <SDL2/SDL.h>
#include "galaxy.h"

#include "tinyfiledialogs/tinyfiledialogs.h"

static const SDL_Keycode Kmap[54] = { 0, SDLK_a, SDLK_b, SDLK_c,
	SDLK_d, SDLK_e, SDLK_f, SDLK_g, SDLK_h, SDLK_i, SDLK_j,
	SDLK_k, SDLK_l, SDLK_m, SDLK_n, SDLK_o, SDLK_p, SDLK_q,
	SDLK_r, SDLK_s, SDLK_t, SDLK_u, SDLK_v, SDLK_w, SDLK_x,
	SDLK_y, SDLK_z, SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT,
	SDLK_SPACE, SDLK_0, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5,
	SDLK_6, SDLK_7, SDLK_8, SDLK_9, SDLK_SEMICOLON, SDLK_QUOTE,
	SDLK_COMMA, SDLK_EQUALS, SDLK_PERIOD, SDLK_SLASH, SDLK_RETURN,
	SDLK_TAB, SDLK_LALT, SDLK_DELETE, SDLK_SCROLLLOCK, SDLK_LSHIFT };

static const char *galaxy_state_file_types[] = { "*.gal", "*.Gal", "*.GAL"};

void event(SDL_Window *window, galaxy_state *state) {
    SDL_Event globEvent;
    while(SDL_PollEvent(&globEvent)) {
        switch(globEvent.type) {
            case SDL_WINDOWEVENT:
                switch(globEvent.window.event) {
                    case SDL_WINDOWEVENT_CLOSE:
                        exit(EXIT_SUCCESS);
                    case SDL_WINDOWEVENT_RESIZED: {
                        SDL_Surface *surface = SDL_GetWindowSurface(window);
                        SDL_FillRect(surface, NULL, 0x00000000);
                        break;
                    }
                }
                break;
        }
    }

    // Iako ovo moze biti malo stetan dizajn, ja cu ga otpratiti radi jednostavnosti
    const Uint8 *kS = SDL_GetKeyboardState(NULL); // kS == keyboardState

    if(kS[SDL_SCANCODE_LCTRL] || kS[SDL_SCANCODE_RCTRL]) {
        if(kS[SDL_SCANCODE_Q]) { // Izlaz
            exit(EXIT_SUCCESS);
        } else if(kS[SDL_SCANCODE_R]) {
            galaxy_reset(state);
        } else if(kS[SDL_SCANCODE_N]) {
            galaxy_trigger_nmi(state);
        } else if(kS[SDL_SCANCODE_S]) {
            // save
            const char *file = tinyfd_saveFileDialog("Save state",
                "stanje.gal", 1, galaxy_state_file_types, "Galaksija emulator state");
            if(file != NULL)
                galaxy_save_state(state, file);
        } else if(kS[SDL_SCANCODE_L]) {
            // load
	    const char *file = tinyfd_openFileDialog("Load emulator state",
                                NULL, 1, galaxy_state_file_types, "Galaksija emulator states", 0);
            if(file != NULL)
                galaxy_load_state(state, file);
        }
        return;
    }

    for (uint8_t A=1; A<54; A++) {
        // no idea what this does honestly
        switch (Kmap[A]) {
            case SDLK_LEFT:
                state->memory[0x2000+A]=kS[SDL_GetScancodeFromKey(Kmap[A])] ||
                                kS[SDL_SCANCODE_BACKSPACE] ? 0xFE : 0xFF;
                break;
            case SDLK_LSHIFT:
                state->memory[0x2000+A]=kS[SDL_GetScancodeFromKey(Kmap[A])] ||
                                kS[SDL_SCANCODE_RSHIFT] ? 0xFE : 0xFF;
                break;
            default:
                state->memory[0x2000+A]=kS[SDL_GetScancodeFromKey(Kmap[A])] ? 0xFE : 0xFF;
        }
    }
}

int main(int argc, char *argv[]) {
    uint32_t last_frame;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("Galaksija",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            GALAXY_HORIZONTAL_RESOLUTION,
            GALAXY_VERTICAL_RESOLUTION,
            SDL_WINDOW_RESIZABLE);

    galaxy_state state = {
        .config = (galaxy_config){
            .default_colors = true,
            .cpu_speed = 0,
            .framerate = 0,
            .firmware_path = "./",
            .graphics_mode = GALAXY_GRAPHICS_MODE_RGBX8888,
            .system_state_file = NULL,
        }
    };

    galaxy_init(&state);

    SDL_Surface *framebuffer = SDL_CreateRGBSurface(
            0, GALAXY_HORIZONTAL_RESOLUTION,
            GALAXY_VERTICAL_RESOLUTION, 32,
            0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);

    while(1) {
        event(window, &state);

        SDL_LockSurface(framebuffer);
        galaxy_run_frame(&state, framebuffer->pixels);
        SDL_UnlockSurface(framebuffer);

        SDL_Surface *win_frame = SDL_GetWindowSurface(window);
        SDL_BlitScaled(framebuffer, NULL, win_frame, NULL);

        SDL_UpdateWindowSurface(window);

        int delay = 1000/GALAXY_FRAMERATE;
        uint32_t new_frame = SDL_GetTicks();
        if(new_frame-last_frame < delay)
            SDL_Delay(delay - new_frame + last_frame);
        last_frame = SDL_GetTicks();
    }
}
