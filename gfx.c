#ifndef GFX_C
#define GFX_C

#include "global_definitions.h"

static void drawPixel(int xpos, int ypos, u8 red, u8 green, u8 blue, bool looping)
{
    if (looping)
    {
        while (xpos >= screen_buffer.width)
        { xpos -= screen_buffer.width; }
        while (xpos < 0)
        { xpos += screen_buffer.width; }
        while (ypos >= screen_buffer.height)
        { ypos -= screen_buffer.height; }
        while (ypos < 0)
        { ypos += screen_buffer.height; }
    }

    if (xpos >= screen_buffer.width || xpos < 0 || ypos >= screen_buffer.height || ypos < 0)
    { return; }

    int pitch = screen_buffer.width;
    u32 *pixel = (u32 *)screen_buffer.data + (ypos * pitch) + xpos;

    *pixel = 255 << 24 | red << 16 | green << 8 | blue;
}
static void drawRect(int xpos, int ypos, int width, int height, u8 red, u8 green, u8 blue)
{
    int x, y;
    int pitch = screen_buffer.width * screen_buffer.bytes_per_pixel;
    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            drawPixel(xpos + x, ypos + y, red, green, blue, true);
        }
    }
}

static void spriteFillRandom(Sprite *spr)
{
    int i;
    for (i = 0; i < IMAGE_PIXEL_COUNT; ++i)
    {
        spr->red[i] = random(0, 255);
        spr->green[i] = random(0, 255);
        spr->blue[i] = random(0, 255);
    }
}
static void spriteDraw(Sprite *spr, int xpos, int ypos)
{
    int x, y;
    for (y = 0; y < IMAGE_PIXEL_COUNT; ++y)
    {
        for (x = 0; x < IMAGE_PIXEL_COUNT; ++x)
        {
            drawPixel(xpos + x, ypos + y,
                    spr->red[y * IMAGE_PIXEL_COUNT + x],
                    spr->green[y * IMAGE_PIXEL_COUNT + x],
                    spr->blue[y * IMAGE_PIXEL_COUNT + x], true);
        }
    }
}
static void imageDraw(Image *img, int xpos, int ypos)
{
    int x, y;
    for (y = 0; y < img->sheetHeight; ++y)
    {
        for (x = 0; x < img->sheetWidth; ++x)
        {
            spriteDraw(&img->spriteSheet[y * img->sheetWidth + x], xpos + (x * IMAGE_PIXEL_COUNT), ypos + (y * IMAGE_PIXEL_COUNT));
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

#endif