#ifndef GLOBAL_DEFINITIONS_H
#define GLOBAL_DEFINITIONS_H

#include <AL/al.h>
#include <AL/alc.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef const char * cstr;
typedef char * str;

#define verify(check, msg) crashIfFalse(check, msg, __FILE__, __LINE__)
static void crashIfFalse(bool check, cstr msg, cstr file, u64 line)
{
    if (check)
    { return; }

    printf("Program failed with message: %s from file %s, line %d\n", msg, file, line);

    exit(1);
}

static bool running;
#define TICK_SPEED 10.0f / 1000.0f
static float getDeltaTime();

/* particles */

typedef enum
{
    PARTICLE_SNOW
} ParticleStyle;

#define MAX_PARTICLES 9000
typedef struct
{
    ParticleStyle style;
    int x, y, w, h;

    float prev_x_positions[MAX_PARTICLES];
    float prev_y_positions[MAX_PARTICLES];
    float x_positions[MAX_PARTICLES];
    float y_positions[MAX_PARTICLES];
    float x_velocity[MAX_PARTICLES];
    float y_velocity[MAX_PARTICLES];
    bool alive[MAX_PARTICLES];
    u8 red[MAX_PARTICLES];
    u8 green[MAX_PARTICLES];
    u8 blue[MAX_PARTICLES];

    int particle_limit;
    int current_editing_particle;
} ParticleSystem;

/* screen */

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 300
#define SCREEN_ASPECT ((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)

typedef struct
{
    void *data;
    int width;
    int height;
    u32 bytes_per_pixel;
} PixelBuffer;
static PixelBuffer screen_buffer;

/* input */

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

/* audio */

#define SAMPLERATE 44100
#define CHANNELS 2

#define STREAMBUFFERSIZE SAMPLERATE
#define SNDBUFFERCOUNT 5
typedef struct
{
    ALuint buffers[SNDBUFFERCOUNT];
    ALint bufferCursor;
    ALuint source;
} SoundStream;

typedef struct
{
    ALuint buffer;
    ALuint source;
} Sound;

/*
typedef struct
{
    int channels;
    int samplerate;
    int bitsPerSample;
    int etc;

    void *data;
    int dataLength;
} WavFile;
*/

/* gfx */

#define IMAGE_PIXEL_COUNT 8
typedef struct _Sprite
{
    u8 red[IMAGE_PIXEL_COUNT * IMAGE_PIXEL_COUNT];
    u8 green[IMAGE_PIXEL_COUNT * IMAGE_PIXEL_COUNT];
    u8 blue[IMAGE_PIXEL_COUNT * IMAGE_PIXEL_COUNT];
} Sprite;
typedef struct _Image
{
    Sprite *spriteSheet;
    u32 sheetWidth, sheetHeight;
    struct _Image *next;
} Image;

#define MAX_IMAGES 128
typedef struct
{
    Image images[MAX_IMAGES];
    u32 head;
    Image *free_images;
} ImagePool;

#endif