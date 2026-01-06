#ifndef GFX_C
#define GFX_C

#include "global_definitions.h"

static void drawPixel(PixelBuffer *buffer, int xpos, int ypos, u8 red, u8 green, u8 blue, bool xlooping, bool ylooping)
{
    xpos -= buffer->camera_x_position;
    ypos -= buffer->camera_y_position;

    if (xlooping)
    {
        while (xpos >= buffer->width)
        { xpos -= buffer->width; }
        while (xpos < 0)
        { xpos += buffer->width; }
    }
    if (ylooping)
    {
        while (ypos >= buffer->height)
        { ypos -= buffer->height; }
        while (ypos < 0)
        { ypos += buffer->height; }
    }

    if (xpos >= buffer->width || xpos < 0 || ypos >= buffer->height || ypos < 0)
    { return; }

    int pitch = buffer->width;
    u32 *pixel = (u32 *)buffer->data + (ypos * pitch) + xpos;

    *pixel = 255 << 24 | red << 16 | green << 8 | blue;
}
static void drawRect(PixelBuffer *buffer, int xpos, int ypos, int width, int height, u32 color)
{
    int x, y;
    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            u8 red = (color >> 16) & 255;
            u8 green = (color >> 8) & 255;
            u8 blue = (color >> 0) & 255;
            drawPixel(buffer, xpos + x, ypos + y, red, green, blue, false, false);
        }
    }
}

static void imageDraw(PixelBuffer *buffer, Image *img, int xpos, int ypos)
{
    u32 x, y;
    for (y = 0; y < IMAGE_SECTOR_SIDE_PIXEL_COUNT * img->sector_y_count; ++y)
    {
        for (x = 0; x < IMAGE_SECTOR_SIDE_PIXEL_COUNT * img->sector_x_count; ++x)
        {
            u32 color = img->pixel_data[y * (IMAGE_SECTOR_SIDE_PIXEL_COUNT * img->sector_x_count) + x];
            u8 red = (color >> 16) & 255;
            u8 green = (color >> 8) & 255;
            u8 blue = (color >> 0) & 255;
            /*u8 alpha = (color >> 24) & 255;*/
            drawPixel(buffer, xpos + x, ypos + y,
                    red, green, blue, false, false);
        }
    }
}

static void clearScreen(PixelBuffer *screen)
{
    u8 *row = (u8 *)(screen->data);
    int pitch = screen->width * screen->bytes_per_pixel;
    int x, y;
    for (y = 0; y < screen->height; ++y)
    {
        u32 *pixel = (u32 *)row;
        for (x = 0; x < screen->width; ++x)
        {
            *pixel++ = 0;
        }
        row += pitch;
    }
}

void pixelDataScale(void *src, int srcw, int srch, void *dest, int destw, int desth)
{
    float scaleX = (float)destw / (float)srcw;
    float scaleY = (float)desth / (float)srch;
    int x, y;
    u32 *pixel = (u32 *)dest;
    u32 *srcpixel = (u32 *)src;

    int minw = MIN(srcw, destw);
    int minh = MIN(srch, desth);
    int maxw = MAX(srcw, destw);
    int maxh = MAX(srch, desth);

    int currentX = 0;
    int currentY = 0;

    for (y = 0; y < desth; ++y)
    {
        for (x = 0; x < destw; ++x)
        {
            int pX = (int)((float)x / scaleX);
            int pY = (int)((float)y / scaleY);
            int diffX = pX - currentX;
            int diffY = pY - currentY;

            srcpixel += diffX;
            currentX = pX;
            srcpixel += diffY * srcw;
            currentY = pY;

            *pixel++ = *srcpixel;
        }
    }
}


#endif