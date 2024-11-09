#include <SDL.h>
#include <stdio.h>

// Fixed Size Types (Avoid Implementation Specifics)
#include <stdint.h>
#include <cstring>  // Add this for memcpy

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "graphics.h"


int main(int argc, char* argv[]) {
    printf("Hello, World\n");
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);
    windowWidth = displayMode.w;
    windowHeight = displayMode.h;
    // Create a window
    SDL_Window* window = SDL_CreateWindow("3D Rasterizer", 
                                          SDL_WINDOWPOS_CENTERED, 
                                          SDL_WINDOWPOS_CENTERED, 
                                          windowWidth, windowHeight, 
                                          SDL_WINDOW_BORDERLESS);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Real Full Screen
    // SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

    FrameBuffer buffer = Graphics_createColorBuffer(windowWidth, windowHeight);
    Graphics_setPixel(buffer, 100, 100, 0xFFFFFFFF);
    Graphics_setPixel(buffer, 10, 20, 0xFFFF0000);
    // Draw a line
    Graphics_drawLine(buffer, 100, 100, 700, 500, 0xFFFFFFFF); // White line

    // Directly access the window surface and copy the color buffer
    SDL_Surface *windowSurface = SDL_GetWindowSurface(window);

    // Main loop flag
    bool quit = false;
    SDL_Event e;

    u32 *imgPixels = NULL;
    int imgW, imgH;
    Graphics_loadImage("./res/t.jpeg", &imgPixels, &imgW, &imgH);
    
    int x = 400, y = 300; // Initial position of the object
    int speed = 5;         // Speed of movement
    // Event loop
    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                quit = true;  // Quit the program if Escape key is pressed
            }
        }
        
         // Handle WASD input
        const Uint8* keystate = SDL_GetKeyboardState(NULL); // Get the state of all keys
        if (keystate[SDL_SCANCODE_W]) {
            y -= speed; // Move up
        }
        if (keystate[SDL_SCANCODE_S]) {
            y += speed; // Move down
        }
        if (keystate[SDL_SCANCODE_A]) {
            x -= speed; // Move left
        }
        if (keystate[SDL_SCANCODE_D]) {
            x += speed; // Move right
        }

        Graphics_clearFrameBuffer(buffer, 0xFF0000FF);
        Graphics_drawBackgroundGrid(buffer, 10, DOTS);
        Graphics_drawRectangle(buffer, 100, 100, 20, 10, 0xFFFF0000, OUTLINE);
        Graphics_blitImageToBuffer(buffer, imgPixels, imgW, imgH, x, y, imgW, imgH);
        Graphics_blitColorBufferToWindow(window, windowSurface, buffer);
    }
    return 0;
}

