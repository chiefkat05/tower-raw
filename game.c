#ifndef GAME_C
#define GAME_C

typedef unsigned int uint;
typedef signed char i8;
typedef unsigned char u8;
typedef int i32;
typedef unsigned int u32;

typedef int bool;

#define true 1
#define false 0

typedef const char * cstr;
typedef char * str;
#define strlen(s) sizeof(s)/sizeof(s[0])

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 256

#define PRESSED 1
#define RELEASED 0
typedef enum
{
    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I,
    KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R,
    KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
    total_key_count
} InputCode;
i32 InputValues[total_key_count];
i32 PrevInputValues[total_key_count];

void inputSet(InputCode input, i32 value)
{
    PrevInputValues[input] = InputValues[input];
    InputValues[input] = value;
}
void inputUpdate()
{
    int i;
    for (i = 0; i < total_key_count; ++i)
    {
        PrevInputValues[i] = InputValues[i];
    }
}
i32 inputGet(InputCode input)
{
    return InputValues[input];
}
bool inputJustReleased(InputCode input)
{
    return (InputValues[input] == PrevInputValues[input] == RELEASED);
}
bool inputJustPressed(InputCode input)
{
    return (InputValues[input] == PrevInputValues[input] == PRESSED);
}


typedef struct
{
    void *data;
    int width;
    int height;
    uint bytes_per_pixel;
} PixelBuffer;
static PixelBuffer screen_buffer;

static bool running;

void drawPixel(int xpos, int ypos, u8 red, u8 green, u8 blue, bool looping)
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

    *pixel = 255 << 24 | blue << 16 | green << 8 | red;
}
void drawRect(int xpos, int ypos, int width, int height)
{
    int x, y;
    int pitch = screen_buffer.width * screen_buffer.bytes_per_pixel;
    u8 *row = (u8 *)screen_buffer.data + (ypos * pitch);
    for (y = 0; y < height; ++y)
    {
        u32 *pixel = (u32 *)(row) + xpos;
        for (x = 0; x < width; ++x)
        {
            drawPixel(xpos + x, ypos + y, 255, 100, 150, false);
        }
        row += pitch;
    }
}

typedef enum
{
    PARTICLE_SNOW
} ParticleStyle;

#define MAX_PARTICLES 200
typedef struct
{
    ParticleStyle style;
    int x, y, w, h;

    float x_positions[MAX_PARTICLES];
    float y_positions[MAX_PARTICLES];
    float x_velocity[MAX_PARTICLES];
    float y_velocity[MAX_PARTICLES];
    int particle_count;
} ParticleManager;

void particleManagerAdd(ParticleManager *manager, float xpos, float ypos)
{
    manager->x_positions[manager->particle_count] = xpos;
    manager->y_positions[manager->particle_count] = ypos;
    ++manager->particle_count;
}
void particleManagerPush(ParticleManager *manager, float xpush, float ypush)
{
    manager->x_velocity[manager->particle_count - 1] = xpush;
    manager->y_velocity[manager->particle_count - 1] = ypush;
}
#include <time.h>
void particleManagerGenerate(ParticleManager *manager)
{
    srand(time(0));
    int i;
    for (i = 0; i < MAX_PARTICLES; ++i)
    {
        particleManagerAdd(manager, rand() % screen_buffer.width, rand() % screen_buffer.height);
        particleManagerPush(manager, (float)(rand() % 400 + 300.0) / 12000.0, -(float)(rand() % 400 + 300.0) / 12000.0);
    }
}
void particleManagerUpdateAndDraw(ParticleManager *manager)
{
    int p;
    for (p = 0; p < manager->particle_count; ++p)
    {
        drawPixel(manager->x_positions[p], manager->y_positions[p], 255, 255, 255, true);
        manager->x_positions[p] += manager->x_velocity[p];
        manager->y_positions[p] += manager->y_velocity[p];
    }
}

void clearScreen(PixelBuffer *screen)
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

void print(cstr msg);

#define verify(check, msg) crashIfFalse(check, msg)
static void crashIfFalse(bool check, cstr msg)
{
    if (check)
    { return; }

    print(msg);

    /* crash */
    *(char *)0 = 0;
}

#endif