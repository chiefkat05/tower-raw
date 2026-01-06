#ifndef GLOBAL_DEFINITIONS_H
#define GLOBAL_DEFINITIONS_H

#include <AL/al.h>
#include <AL/alc.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

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

#define CRASH *(char *)0 = 0

#define KILOBYTES(i) (i * 1024)
#define MEGABYTES(i) (KILOBYTES(i) * 1024)
#define GIGABYTES(i) (MEGABYTES(i) * 1024)

/* TODO: there's apparently a superior way to do this macro so implement that */
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifdef DEBUG
#define verify(check, msg) crashIfFalse(check, msg, __FILE__, __LINE__)
#else
#define verify(check, msg)
#endif
static void crashIfFalse(bool check, cstr msg, cstr file, u64 line)
{
    if (check)
    { return; }

    printf("Program failed with message: %s from file %s, line %ld\n", msg, file, line);

    CRASH;
}

static void swap(void *a, void *b)
{
    void *temp = a;
    a = b;
    b = temp;
}

#define TICK_SPEED 10.0f / 1000.0f

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
    int camera_x_position, camera_y_position;
} PixelBuffer;

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
extern i32 InputValues[total_key_count];
extern i32 PrevInputValues[total_key_count];


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

#define IMAGE_SECTOR_SIDE_PIXEL_COUNT 8
#define IMAGE_SECTOR_PIXEL_COUNT (IMAGE_SECTOR_SIDE_PIXEL_COUNT * IMAGE_SECTOR_SIDE_PIXEL_COUNT)
#define IMAGE_SIDE_SECTOR_COUNT 54
#define IMAGE_SECTOR_COUNT (IMAGE_SIDE_SECTOR_COUNT * IMAGE_SIDE_SECTOR_COUNT)
#define IMAGE_SIDE_PIXEL_COUNT (IMAGE_SECTOR_SIDE_PIXEL_COUNT * IMAGE_SIDE_SECTOR_COUNT)
#define IMAGE_PIXEL_COUNT (IMAGE_SECTOR_PIXEL_COUNT * IMAGE_SECTOR_COUNT)
typedef struct _Image
{
    u32 pixel_data[IMAGE_PIXEL_COUNT];
    u32 sector_x_count, sector_y_count;
    struct _Image *next;
} Image;

#define MAX_IMAGES 128
typedef struct
{
    Image images[MAX_IMAGES];
    u32 head;
    Image *free_images;
} ImagePool;

/*
typedef struct
{
    u16 bytes;
    char bmptext[4];
    u8 *other stuff;
} BMPheader;
 
/* functions */
void pixelDataScale(void *src, int srcw, int srch, void *dest, int destw, int desth);

/* memory */
#define READ_FILE(name) void name(char *path, void *out, u64 size)
typedef READ_FILE(read_file);

#define WRITE_FILE(name) void name(char *path, void *in, u64 size)
typedef WRITE_FILE(write_file);

#define GET_DELTATIME(name) float name()
typedef GET_DELTATIME(get_deltatime);

#define GAME_MEMORY_SIZE MEGABYTES((u64)32)
typedef struct
{
    u64 size;
    void *data;
    read_file *readFile;
    write_file *writeFile;
    get_deltatime *getDeltaTime;

    PixelBuffer screen_buffer;
    i32 InputValues[total_key_count];
    i32 PrevInputValues[total_key_count];
} GameMemory;


#endif