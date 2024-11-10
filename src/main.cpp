#include <SDL.h>
#include <stdio.h>

// Fixed Size Types (Avoid Implementation Specifics)
#include <stdint.h>
#include <cstring>  // Add this for memcpy

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "rasterizer_graphics.h"

#include "rasterizer_math.h"

int main(int argc, char* argv[]) 
{
    Graphics_initializeWindow();

    // Real Full Screen
    // SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    // Event loop
    while (!quit) 
    {    
        Graphics_processInput();

        Graphics_update();

        Graphics_render();
    }

    return 0;
}