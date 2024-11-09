#include "graphics.h"
#include <cstring>  // Add this for memcpy
#include <SDL.h>
#include "stb_image.h"

int Graphics_loadImage(const char *filename, u32 **pixels, int *width, int *height) {
    // Load the image using stb_image
    int channels;
    unsigned char *data = stbi_load(filename, width, height, &channels, 4); // 4 channels for RGBA

    if (data == NULL) {
        printf("Error: Failed to load image: %s\n", filename);
        printf("stbi_error: %s\n", stbi_failure_reason());  // This prints a more detailed error message

        return 0; // Failed to load the image
    }

    // Allocate memory for the pixel data in u32 format
    *pixels = (u32*)malloc((*width) * (*height) * sizeof(u32));

    if (*pixels == NULL) {
        printf("Error: Failed to allocate memory for pixel data.\n");
        stbi_image_free(data);
        return 0; // Memory allocation failed
    }

    // Convert from 8-bit RGBA to 32-bit format (u32 = 0xAARRGGBB)
    for (int y = 0; y < *height; y++) {
        for (int x = 0; x < *width; x++) {
            int idx = (y * *width + x) * 4;
            u32 pixel = (data[idx + 3] << 24) | (data[idx + 0] << 16) | (data[idx + 1] << 8) | data[idx + 2];
            (*pixels)[y * (*width) + x] = pixel;
        }
    }

    // Free the raw image data
    stbi_image_free(data);
    return 1; // Success
}

void Graphics_setPixel(FrameBuffer buffer, i32 x, i32 y, u32 color)
{
    if(x >= 0 && x < buffer.width && y >= 0 && y < buffer.height)
        buffer.buffer[buffer.width * y + x] = color;
}

FrameBuffer Graphics_createColorBuffer(u32 w, u32 h)
{
    FrameBuffer result = {};
    result.buffer = (u32*) malloc(w * h * sizeof(u32));

    if(!result.buffer) 
    {
        // Something wrong happend ...
    }

    result.width = w;
    result.height = h;
    
    SDL_memset(result.buffer, 0, w * h * sizeof(u32));

    return result;
}

void Graphics_clearFrameBuffer(FrameBuffer &buffer, u32 color)
{
   for(int i = 0; i < buffer.width * buffer.height; i++)
   {
    buffer.buffer[i] = color;
   }
}

// Draws a line between two points using Bresenham's line algorithm
void Graphics_drawLine(FrameBuffer buffer, i32 x0, i32 y0, i32 x1, i32 y1, u32 color) 
{
    i32 dx = abs(x1 - x0);
    i32 dy = abs(y1 - y0);
    i32 sx = x0 < x1 ? 1 : -1;
    i32 sy = y0 < y1 ? 1 : -1;
    i32 err = dx - dy;

    while (true) 
    {
        Graphics_setPixel(buffer, x0, y0, color); // Draw pixel

        if (x0 == x1 && y0 == y1) break;

        i32 e2 = 2 * err;
        if (e2 > -dy) 
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) 
        {
            err += dx;
            y0 += sy;
        }
    }
}

void Graphics_drawBackgroundGrid(FrameBuffer &buffer, i32 step, GRID_MODE mode)
{
    const u32 WHITE = 0xFFFFFFFF;
    const u32 LIGHT_GRAY = 0xFFCCCCCC;
    const u32 DARK_GRAY = 0xFF404040;
    
    for(int x = 0; x < buffer.width; x++)
    {
        for(int y = 0; y < buffer.height; y++)
        {
            if(mode == LINES)
            {
                if(x % step == 0 || y % step == 0)
                    Graphics_setPixel(buffer, x, y, DARK_GRAY);
            }
            else
            {
                if(x % step == 0 && y % step == 0)
                    Graphics_setPixel(buffer, x, y, WHITE);
            }
        }
    }
}

void Graphics_drawRectangle(FrameBuffer &buffer, i32 x0, i32 y0, i32 w, i32 h, u32 color, RECT_MODE mode)
{
    for(int x = x0; x <= x0 + w; x++)
    {
        for(int y = y0; y <= y0 + h; y++)
        {
            if(mode == FILL)
                Graphics_setPixel(buffer, x, y, color);
            else
            {   
                bool yCondition1 = y >= y0 && y <= y0 + h;
                bool xCondition1 = x == x0 || x == x0 + w;
                bool yCondition2 = y == y0 || y == y0 + h;
                bool xCondition2 = x >= x0 && x <= x0 + w;
                if(xCondition1 && yCondition1 || xCondition2 && yCondition2) 
                    Graphics_setPixel(buffer, x, y, color);
            }
        }   
    }
}

void Graphics_blitColorBufferToWindow(SDL_Window *window, SDL_Surface *windowSurface, FrameBuffer &buffer)
{
    memcpy(windowSurface->pixels, buffer.buffer, buffer.width * buffer.height * sizeof(u32));
    SDL_UpdateWindowSurface(window);
}

void Graphics_blitImageToBuffer(FrameBuffer &buffer, u32 *imgPixels, int imgW, int imgH, int x, int y, int w, int h)
{
    // Ensure the destination area doesn't go beyond the framebuffer boundaries
    int destX = x;
    int destY = y;
    int destW = w;
    int destH = h;

    if (destX < 0) {
        destW += destX; // Adjust width if x is out of bounds
        destX = 0;
    }
    if (destY < 0) {
        destH += destY; // Adjust height if y is out of bounds
        destY = 0;
    }
    if (destX + destW > buffer.width) {
        destW = buffer.width - destX; // Ensure we don't go beyond the right edge
    }
    if (destY + destH > buffer.height) {
        destH = buffer.height - destY; // Ensure we don't go beyond the bottom edge
    }

    // Blit pixels from the image to the framebuffer
    for (int j = 0; j < destH; ++j) {
        for (int i = 0; i < destW; ++i) {
            int imgX = i * imgW / w; // Map the destination pixel to the image pixel
            int imgY = j * imgH / h; // Map the destination pixel to the image pixel

            // Get the color from the image at (imgX, imgY)
            u32 pixelColor = imgPixels[imgY * imgW + imgX];

            // Copy the color to the framebuffer at the destination position
            buffer.buffer[(destY + j) * buffer.width + (destX + i)] = pixelColor;
        }
    }
}
