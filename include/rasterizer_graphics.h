#pragma once

// Fixed Size Types (Avoid Implementation Specifics)
#include <stdint.h>

#define u8  uint8_t
#define u32 uint32_t
#define i32 int32_t
#define globalVariable static


struct FrameBuffer
{
    u32 *buffer;
    u32 width;
    u32 height;
};

enum GRID_MODE
{
    LINES,
    DOTS
};

enum RECT_MODE
{
    OUTLINE,
    FILL
};

enum PROJECTION_MODE
{
    ORTHOGRAPHIC,
    PERSPECTIVE
};

struct SDL_Window;
struct SDL_Surface;
struct Vector2;
struct Vector3;

extern int windowWidth; // Declare global variables with extern
extern int windowHeight;
extern SDL_Window* window;
extern bool quit;
extern FrameBuffer buffer;


extern int          Graphics_loadImage                (const char *filename, u32 **pixels, int *width, int *height);
extern void         Graphics_setPixel                 (FrameBuffer buffer, i32 x, i32 y, u32 color);
extern FrameBuffer  Graphics_createColorBuffer        (u32 w, u32 h);
extern void         Graphics_clearFrameBuffer         (FrameBuffer &buffer, u32 color);
extern void         Graphics_drawLine                 (FrameBuffer buffer, i32 x0, i32 y0, i32 x1, i32 y1, u32 color);
extern void         Graphics_drawBackgroundGrid       (FrameBuffer &buffer, i32 step, GRID_MODE mode);
extern void         Graphics_drawRectangle            (FrameBuffer &buffer, i32 x0, i32 y0, i32 w, i32 h, u32 color, RECT_MODE mode);
extern void         Graphics_blitColorBufferToWindow  (SDL_Window *window, SDL_Surface *windowSurface, FrameBuffer &buffer);
extern void         Graphics_blitImageToBuffer        (FrameBuffer &buffer, u32 *imgPixels, int imgW, int imgH, int x, int y, int w, int h);
extern void         Graphics_initializeWindow();
extern void         Graphics_processInput();
extern void         Graphics_update();
extern void         Graphics_render();

// Naive 3D Virtual World to Screen Space Projection
extern Vector2         Graphics_project     (Vector3 point, PROJECTION_MODE mode);
extern u32             Graphics_darkenColor (u32 color, float z);