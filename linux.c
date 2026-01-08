#ifdef __linux__

#include <xcb/xcb.h>
#include <xcb/xkb.h>
#include <xcb/xcb_image.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <dlfcn.h>

#include "global_definitions.h"
#include "chiefkat-pulseaudio.h"

#define GAME_LOOP(name) void name(GameMemory *game_mem)
typedef GAME_LOOP(game_loop);
GAME_LOOP(gameLoopStub)
{
}
#define INPUT_SET(name) void name(i32 *inputArray, InputCode input, i32 value)
typedef INPUT_SET(input_set);
INPUT_SET(inputSetStub)
{
}
#define PIXEL_DATASCALE(name) void name(void *src, int srcw, int srch, void *dest, int destw, int desth)
typedef PIXEL_DATASCALE(pixel_datascale);
PIXEL_DATASCALE(pixelDataScaleStub)
{
}

READ_FILE(readFile)
{
    FILE *file = fopen(path, "rb");
    verify(file, "failed to open file");

    fread(out, size, 1, file);

    fclose(file);
}
WRITE_FILE(writeFile)
{

}

static struct timespec currentTime, prevTime;
GET_DELTATIME(getDeltaTime)
{
    prevTime = currentTime;
    clock_gettime(1, &currentTime);

    float time_passed = (currentTime.tv_sec - prevTime.tv_sec) * 1000000000 + (currentTime.tv_nsec - prevTime.tv_nsec);
    float delta = time_passed / 1000000000.0f;

    return delta;
}

typedef struct
{
    void *gameLib;
    game_loop *gameLoop;
    input_set *inputSet;
    pixel_datascale *pixelDataScale;
    bool isValid;
} gameFunctions;
static gameFunctions gameFunc;

static void loadGameCode()
{
    if (gameFunc.gameLib)
    { return; }

    gameFunc.isValid = false;

    system("cp libgame.so libgame_temp.so");
    gameFunc.gameLib = dlopen("./libgame.so", RTLD_LAZY);

    if (gameFunc.gameLib)
    {
        gameFunc.gameLoop = dlsym(gameFunc.gameLib, "gameLoop");
        gameFunc.inputSet = dlsym(gameFunc.gameLib, "inputSet");
        gameFunc.pixelDataScale = dlsym(gameFunc.gameLib, "pixelDataScale");
        gameFunc.isValid = (gameFunc.gameLoop && gameFunc.inputSet && gameFunc.pixelDataScale);
    }

    if (gameFunc.isValid)
    { return; }

    printf("game functions not valid\n");
    gameFunc.gameLoop = gameLoopStub;
    gameFunc.inputSet = inputSetStub;
    gameFunc.pixelDataScale = pixelDataScaleStub;
}
static void unloadGameCode()
{
    if (gameFunc.gameLib)
    {
        dlclose(gameFunc.gameLib);
        gameFunc.gameLib = NULL;
    }
    gameFunc.isValid = false;
    gameFunc.gameLoop = gameLoopStub;
    gameFunc.inputSet = inputSetStub;
    gameFunc.pixelDataScale = pixelDataScaleStub;
}

#define BYTES_PER_PIXEL 4
static float win_width, win_height, screen_width, screen_height, screen_offset_x, screen_offset_y;
static void *window_buffer;
static void windowResize(xcb_connection_t *connection, xcb_screen_t *screen, xcb_window_t window, int width, int height)
{
    win_width = width;
    win_height = height;

    /* side closest to it's screen_width cousin */
    float WidthDiff = (float)width / (float)SCREEN_WIDTH;
    float HeightDiff = (float)height / (float)SCREEN_HEIGHT;
    float widthAspect = WidthDiff / MIN(WidthDiff, HeightDiff);
    float heightAspect = HeightDiff / MIN(WidthDiff, HeightDiff);

    screen_width = (float)width / widthAspect;
    screen_height = (float)height / heightAspect;
    screen_offset_x = width / 2 - (screen_width / 2.0f);
    screen_offset_y = height / 2 - (screen_height / 2.0f);

    if (window_buffer)
    {
        free(window_buffer);
        window_buffer = malloc(screen_width * screen_height * BYTES_PER_PIXEL);
        verify(window_buffer, "failed to allocate window buffer");
    }
}
static void xcbEventLoop(xcb_connection_t *connection, GameMemory *game_memory);

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
int main()
{
    xcb_connection_t *connection = xcb_connect(NULL, NULL);

    xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

    xcb_window_t window = xcb_generate_id(connection);
    u32 window_values[] = { 0,
        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_KEY_PRESS };
    xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
                            10, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual,
                            XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK, window_values);

    xcb_map_window(connection, window);
    xcb_flush(connection);

    xcb_gcontext_t graphics_context = xcb_generate_id(connection);
    xcb_create_gc(connection, graphics_context, window, 0, NULL);

    loadGameCode();

    GameMemory game_memory = {};
    game_memory.size = GAME_MEMORY_SIZE;
    game_memory.data = malloc(game_memory.size);
    verify(game_memory.data, "failed to allocate game data");
    game_memory.readFile = readFile;
    game_memory.writeFile = writeFile;
    game_memory.getDeltaTime = getDeltaTime;

    game_memory.screen_buffer = (PixelBuffer){
        .width = SCREEN_WIDTH,
        .height = SCREEN_HEIGHT,
        .bytes_per_pixel = 4
    };

    game_memory.screen_buffer.data = malloc(game_memory.screen_buffer.width *
        game_memory.screen_buffer.bytes_per_pixel * game_memory.screen_buffer.height);
    verify(game_memory.screen_buffer.data, "failed to allocate game_memory.screen_buffer data");
    void *screen_data_flipped = malloc(game_memory.screen_buffer.width * game_memory.screen_buffer.height *
        game_memory.screen_buffer.bytes_per_pixel);
    verify(screen_data_flipped, "failed to allocate flipped screen data");

    windowResize(connection, screen, window, WINDOW_WIDTH, WINDOW_HEIGHT);

    window_buffer = malloc(screen_width * screen_height * BYTES_PER_PIXEL);
    verify(window_buffer, "failed to allocate window buffer");

    bool running = true;
    getDeltaTime();

    /* Big thanks to user CLearner over at https://stackoverflow.com/questions/31616651/xcb-ignoring-repeated-keys */
    xcb_xkb_use_extension(connection, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);
    xcb_xkb_per_client_flags(connection, XCB_XKB_ID_USE_CORE_KBD, XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT, 1, 0, 0, 0);

    xcb_pixmap_t blackpixmap = xcb_generate_id(connection);

    audioInit();

    int currentWinWidth = win_width, currentWinHeight = win_height;
    int frame;
    while (running)
    {
        if (frame > 600)
        {
            unloadGameCode();
            loadGameCode();
            frame = 0;
        }
        /* make this better somehow */
        ++frame;
        xcbEventLoop(connection, &game_memory);
        if (frame % 60 == 0)
        {
            xcb_get_geometry_cookie_t cookie = xcb_get_geometry(connection, window);
            xcb_get_geometry_reply_t *reply = xcb_get_geometry_reply(connection, cookie, NULL);

            if (currentWinWidth != reply->width || currentWinHeight != reply->height)
            {
                windowResize(connection, screen, window, reply->width, reply->height);
                currentWinWidth = reply->width;
                currentWinHeight = reply->height;

                xcb_create_pixmap(connection, screen->root_depth, blackpixmap, window, win_width, win_height);
                xcb_copy_area(connection, blackpixmap, window, graphics_context, 0, 0, 0, 0, win_width, win_height);
                
                xcb_free_pixmap(connection, blackpixmap);
            }

            free(reply);
        }

        static int tone = 128;
        if (game_memory.InputValues[KEY_SPACE] == PRESSED && game_memory.PrevInputValues[KEY_SPACE] == RELEASED)
        {
            ++tone;
        }
        gameFunc.gameLoop(&game_memory);

        /* vertically flip image (this could use optimization) */
        int y = 0, x = 0;
        for (y = 0; y < game_memory.screen_buffer.height; ++y)
        {
            u32 *pixel = (u32 *)game_memory.screen_buffer.data + (y * game_memory.screen_buffer.width);
            u32 *outpixel = (u32 *)screen_data_flipped + (game_memory.screen_buffer.height * game_memory.screen_buffer.width) - ((y + 1) * game_memory.screen_buffer.width);
            for (x = 0; x < game_memory.screen_buffer.width; ++x)
            {
                *outpixel++ = *pixel++;
            }
        }

        gameFunc.pixelDataScale(screen_data_flipped, game_memory.screen_buffer.width, game_memory.screen_buffer.height,
                        window_buffer, screen_width, screen_height);
        /* make a screenscaled buffer and have it go screenbuf->flipbuf->scalebuf->windowbuf */

        xcb_image_t *img = xcb_image_create_native(connection, screen_width, screen_height,
                XCB_IMAGE_FORMAT_Z_PIXMAP, screen->root_depth, window_buffer,
                screen_width * screen_height * BYTES_PER_PIXEL, NULL);
        verify(img, "failed to create native image");
        xcb_image_put(connection, window, graphics_context, img, screen_offset_x, screen_offset_y, 0); /* use this for winOffsetX and Y */
        
        xcb_flush(connection);
    }
    free(game_memory.data);
    audioExit();

    xcb_free_gc(connection, graphics_context);
    xcb_disconnect(connection);

    return 0;
}

static void xcbEventLoop(xcb_connection_t *connection, GameMemory *game_memory)
{
    xcb_generic_event_t *event;
    while ((event = xcb_poll_for_event(connection)))
    {
        if (event->response_type == XCB_KEY_PRESS || event->response_type == XCB_KEY_RELEASE)
        {
            xcb_key_press_event_t *key_event = (xcb_key_press_event_t *)event;

            /* magic numbers */
            switch(key_event->detail)
            {
                case 9:
                    gameFunc.inputSet(game_memory->InputValues, KEY_ESC, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 23:
                    gameFunc.inputSet(game_memory->InputValues, KEY_TAB, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 24:
                    gameFunc.inputSet(game_memory->InputValues, KEY_Q, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 25:
                    gameFunc.inputSet(game_memory->InputValues, KEY_W, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 26:
                    gameFunc.inputSet(game_memory->InputValues, KEY_E, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 27:
                    gameFunc.inputSet(game_memory->InputValues, KEY_R, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 28:
                    gameFunc.inputSet(game_memory->InputValues, KEY_T, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 29:
                    gameFunc.inputSet(game_memory->InputValues, KEY_Y, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 30:
                    gameFunc.inputSet(game_memory->InputValues, KEY_U, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 31:
                    gameFunc.inputSet(game_memory->InputValues, KEY_I, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 32:
                    gameFunc.inputSet(game_memory->InputValues, KEY_O, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 33:
                    gameFunc.inputSet(game_memory->InputValues, KEY_P, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 36:
                    gameFunc.inputSet(game_memory->InputValues, KEY_ENTER, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 37:
                    gameFunc.inputSet(game_memory->InputValues, KEY_CTRL, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 38:
                    gameFunc.inputSet(game_memory->InputValues, KEY_A, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 39:
                    gameFunc.inputSet(game_memory->InputValues, KEY_S, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 40:
                    gameFunc.inputSet(game_memory->InputValues, KEY_D, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 41:
                    gameFunc.inputSet(game_memory->InputValues, KEY_F, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 42:
                    gameFunc.inputSet(game_memory->InputValues, KEY_G, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 43:
                    gameFunc.inputSet(game_memory->InputValues, KEY_H, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 44:
                    gameFunc.inputSet(game_memory->InputValues, KEY_J, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 45:
                    gameFunc.inputSet(game_memory->InputValues, KEY_K, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 46:
                    gameFunc.inputSet(game_memory->InputValues, KEY_L, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 50:
                    gameFunc.inputSet(game_memory->InputValues, KEY_SHIFT, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 52:
                    gameFunc.inputSet(game_memory->InputValues, KEY_Z, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 53:
                    gameFunc.inputSet(game_memory->InputValues, KEY_X, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 54:
                    gameFunc.inputSet(game_memory->InputValues, KEY_C, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 55:
                    gameFunc.inputSet(game_memory->InputValues, KEY_V, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 56:
                    gameFunc.inputSet(game_memory->InputValues, KEY_B, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 57:
                    gameFunc.inputSet(game_memory->InputValues, KEY_N, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 58:
                    gameFunc.inputSet(game_memory->InputValues, KEY_M, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 65:
                    gameFunc.inputSet(game_memory->InputValues, KEY_SPACE, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
            }
        }
        free(event);
    }
}

#endif