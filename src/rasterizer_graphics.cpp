#include "rasterizer_graphics.h"
#include <cstring>  // Add this for memcpy
#include <SDL.h>
#include "stb_image.h"

#include "rasterizer_math.h"

// Define global variables here
int windowWidth      = 800;
int windowHeight     = 600;
SDL_Window* window   = nullptr;
bool quit            = false;
FrameBuffer buffer;
SDL_Surface *windowSurface = nullptr;

// Cube Points
const int M_POINTS = 9 * 9 * 9;
Vector3 cloudOfPoints[M_POINTS];
Vector2 projectedPoints[M_POINTS];
float fovFactor = 128 * 6;
Vector3 cameraPosition = {0, 0, -5};

// Bitmap Testing
u32 *pixels;
int w, h;
    

int Graphics_loadImage(const char *filename, u32 **pixels, int *width, int *height) 
{
    // Load the image using stb_image
    int channels;
    unsigned char *data = stbi_load(filename, width, height, &channels, 4); // 4 channels for RGBA

    if (data == NULL) 
    {
        printf("Error: Failed to load image: %s\n", filename);
        printf("stbi_error: %s\n", stbi_failure_reason());  // This prints a more detailed error message

        return 0; // Failed to load the image
    }

    // Allocate memory for the pixel data in u32 format
    *pixels = (u32*)malloc((*width) * (*height) * sizeof(u32));

    if (*pixels == NULL) 
    {
        printf("Error: Failed to allocate memory for pixel data.\n");
        stbi_image_free(data);
        return 0; // Memory allocation failed
    }

    // Convert from 8-bit RGBA to 32-bit format (u32 = 0xAARRGGBB)
    for (int y = 0; y < *height; y++) 
    {
        for (int x = 0; x < *width; x++) 
        {
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

void Graphics_drawRectangle(FrameBuffer &buffer, i32 x0, i32 y0, i32 w, i32 h,
     u32 color, RECT_MODE mode)
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

void Graphics_blitColorBufferToWindow(SDL_Window *window, SDL_Surface *windowSurface,
     FrameBuffer &buffer)
{
    memcpy(windowSurface->pixels, buffer.buffer, buffer.width * buffer.height * sizeof(u32));
    SDL_UpdateWindowSurface(window);
}

void Graphics_blitImageToBuffer(FrameBuffer &buffer, u32 *imgPixels, int imgW,
     int imgH, int x, int y, int w, int h)
{
    // Ensure the destination area doesn't go beyond the framebuffer boundaries
    int destX = x;
    int destY = y;
    int destW = w;
    int destH = h;

    if (destX < 0) 
    {
        destW += destX; // Adjust width if x is out of bounds
        destX = 0;
    }
    if (destY < 0) 
    {
        destH += destY; // Adjust height if y is out of bounds
        destY = 0;
    }
    if (destX + destW > buffer.width) 
    {
        destW = buffer.width - destX; // Ensure we don't go beyond the right edge
    }
    if (destY + destH > buffer.height) 
    {
        destH = buffer.height - destY; // Ensure we don't go beyond the bottom edge
    }

    // Blit pixels from the image to the framebuffer
    for (int j = 0; j < destH; ++j) 
    {
        for (int i = 0; i < destW; ++i) 
        {
            int imgX = i * imgW / w; // Map the destination pixel to the image pixel
            int imgY = j * imgH / h; // Map the destination pixel to the image pixel

            // Get the color from the image at (imgX, imgY)
            u32 pixelColor = imgPixels[imgY * imgW + imgX];

            // Copy the color to the framebuffer at the destination position
            buffer.buffer[(destY + j) * buffer.width + (destX + i)] = pixelColor;
        }
    }
}

void Graphics_initializeWindow()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return;
    }
  
    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);
    
    windowWidth = displayMode.w;
    windowHeight = displayMode.h;
    
    // Create a window
    window = SDL_CreateWindow("3D Rasterizer", 
                                          SDL_WINDOWPOS_CENTERED, 
                                          SDL_WINDOWPOS_CENTERED, 
                                          windowWidth, windowHeight, 
                                          SDL_WINDOW_BORDERLESS);
    if (window == NULL) 
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }

    buffer = Graphics_createColorBuffer(windowWidth, windowHeight);

     // Directly access the window surface and copy the color buffer
    windowSurface = SDL_GetWindowSurface(window);

    // Initialize the Cloud of Points (Position Vectors)
    int pointCount = 0;
 
    for(float x = -1; x <= 1.0; x += 0.25f)
    {
        for(float y = -1; y <= 1.0f; y += 0.25f)
        {
            for(float z = -1; z <= 1.0f; z += 0.25f)
            {
                Vector3 newPoint = {x, y, z};

                cloudOfPoints[pointCount++] = newPoint;
            }
        }
    }

    Graphics_loadImage("./res/t.jpeg", &pixels, &w, &h);
}

void Graphics_processInput()
{
    SDL_Event e;

    // Handle events
    while (SDL_PollEvent(&e) != 0) 
    {
        if (e.type == SDL_QUIT) 
        {
            quit = true;
        }
        
        else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) 
        {
            quit = true;  // Quit the program if Escape key is pressed
        }
    }
}

void Graphics_update()
{
    for(int i = 0; i < M_POINTS; i++)
    {
        Vector3 point = cloudOfPoints[i];

        point.z -= cameraPosition.z;

        // Screen Space Coordinate
        Vector2 projectedPoint = Graphics_project(point, PERSPECTIVE);

        projectedPoint.x += windowWidth/2.0f;
        projectedPoint.y += windowHeight/2.0f;

        projectedPoints[i] = projectedPoint;
    }
}

// Darken color based on z-value
u32 Graphics_darkenColor(u32 color, float z) 
{
    // Extract ARGB components
    u8 alpha = (color >> 24) & 0xFF;
    u8 red = (color >> 16) & 0xFF;
    u8 green = (color >> 8) & 0xFF;
    u8 blue = color & 0xFF;

    // Calculate darkening factor based on z (inverted, so larger z means darker)
    float factor = 1.0f - z;  // Darker for larger z values

    // Apply the factor to each color component
    red = (u8)(red * factor);
    green = (u8)(green * factor);
    blue = (u8)(blue * factor);

    // Reconstruct the color with darkened RGB values
    return (alpha << 24) | (red << 16) | (green << 8) | blue;
}

void Graphics_render()
{
    Graphics_clearFrameBuffer(buffer, 0xFF000000);
       
    Graphics_drawBackgroundGrid(buffer, 10, DOTS);
    Graphics_drawRectangle(buffer, 100, 100, 20, 10, 0xFFFF0000, OUTLINE);

    Graphics_drawRectangle(buffer, 300, 200, 300, 150, 0xFFFF00FF, FILL);
       
    // Draw Projected Points On Screen Plane
    for(int i = 0; i < M_POINTS; i++)
    {
        Vector2 point = projectedPoints[i];

        // Darken color based on z value
        u32 color = Graphics_darkenColor(0xFFF00FFFF, cloudOfPoints[i].z);

        Graphics_drawRectangle(buffer, (u32) point.x, (u32) point.y, 5,5, color, FILL);
    }

    //Graphics_blitImageToBuffer(buffer, pixels, w, h, 100, 100, w, h);

    Graphics_blitColorBufferToWindow(window, windowSurface, buffer);
}

// We are using Left-Handed Coordinates Handedness
Vector2 Graphics_project(Vector3 point, PROJECTION_MODE mode)
{
    Vector2 screenPosition;
    switch(mode)
    {
        case ORTHOGRAPHIC:
        {
            // Naive Orthographic Projection
            screenPosition = {point.x * fovFactor, point.y * fovFactor};
        
        } break;

        case PERSPECTIVE:
        {
            screenPosition = {
                (point.x * fovFactor) / point.z, 
                (point.y * fovFactor) / point.z
            };

        } break;
    }
    return screenPosition;
}
