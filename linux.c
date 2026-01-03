#ifdef __linux__

#include <xcb/xcb.h>
#include <xcb/xkb.h>
#include <xcb/xcb_image.h>

#include "game.c"

static void *Allocate(u32 bytes)
{
    return malloc(bytes);
}
static void Free(void *data)
{
    free(data);
}
static float getDeltaTime()
{
    return 0.0;
}
static void windowResize(xcb_connection_t *connection, xcb_screen_t *screen, xcb_window_t window, int width, int height)
{
    printf("time to resize the window\n");
}
static void xcbEventLoop(xcb_connection_t *connection, xcb_screen_t *screen, xcb_window_t window);

void flipScreen()
{
    void *screen_data_flipped = malloc(sizeof(screen_buffer.data));
    int y = 0, x = 0;
    for (y = 0; y < screen_buffer.height; ++y)
    {
        u32 *pixel = (u32 *)screen_buffer.data + ((screen_buffer.height - y) * screen_buffer.width);
        u32 *fpixel = (u32 *)screen_data_flipped + (y * screen_buffer.width);
        for (x = 0; x < screen_buffer.width; ++x)
        {
            *fpixel++ = *pixel++;
        }
    }

    free(screen_buffer.data);
    screen_buffer.data = screen_data_flipped;
}

int main()
{
    xcb_connection_t *connection = xcb_connect(NULL, NULL);

    xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

    xcb_window_t window = xcb_generate_id(connection);
    u32 window_values[] = { screen->black_pixel,
        XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_POINTER_MOTION | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_KEY_PRESS |
        XCB_EVENT_MASK_RESIZE_REDIRECT };
    xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                            10, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual,
                            XCB_CW_BACK_PIXMAP | XCB_CW_EVENT_MASK, window_values);

    xcb_map_window(connection, window);
    xcb_flush(connection);

    xcb_gcontext_t graphics_context = xcb_generate_id(connection);
    u32 gcontext_values[] = { screen->white_pixel, 0 };
    xcb_create_gc(connection, graphics_context, window, XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES, gcontext_values);

    screen_buffer = (PixelBuffer){
        .width = SCREEN_WIDTH,
        .height = SCREEN_HEIGHT,
        .bytes_per_pixel = 4
    };

    screen_buffer.data = malloc(screen_buffer.width * screen_buffer.bytes_per_pixel * screen_buffer.height);
    verify(screen_buffer.data, "failed to allocate screen_buffer data");

    windowResize(connection, screen, window, screen_buffer.width, screen_buffer.height);

    running = true;
    gameInit();
    getDeltaTime();

    /* Big thanks to user CLearner over at https://stackoverflow.com/questions/31616651/xcb-ignoring-repeated-keys */
    xcb_xkb_use_extension(connection, XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);
    xcb_xkb_per_client_flags(connection, XCB_XKB_ID_USE_CORE_KBD, XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT, 1, 0, 0, 0);

    /* image test */

    while (running)
    {
        xcbEventLoop(connection, screen, window);

        gameLoop();

        flipScreen();
        /* put screen_buffer on screen here */
        xcb_image_t *img = xcb_image_create_native(connection, screen_buffer.width, screen_buffer.height,
                XCB_IMAGE_FORMAT_Z_PIXMAP, screen->root_depth, screen_buffer.data,
                screen_buffer.width * screen_buffer.height * screen_buffer.bytes_per_pixel, NULL);
        verify(img, "failed to create native image");
        xcb_image_put(connection, window, graphics_context, img, 0, 0, 0);

        xcb_flush(connection);
    }

    xcb_free_gc(connection, graphics_context);
    xcb_disconnect(connection);
}

static void xcbEventLoop(xcb_connection_t *connection, xcb_screen_t *screen, xcb_window_t window)
{
    xcb_generic_event_t *event;
    while ((event = xcb_poll_for_event(connection)))
    {
        if (event->response_type == XCB_RESIZE_REQUEST)
        {
            xcb_resize_request_event_t *resize_event = (xcb_resize_request_event_t *)event;
            windowResize(connection, screen, window, resize_event->width, resize_event->height);
        }
        if (event->response_type == XCB_KEY_PRESS || event->response_type == XCB_KEY_RELEASE)
        {
            xcb_key_press_event_t *key_event = (xcb_key_press_event_t *)event;

            /* magic numbers */
            switch(key_event->detail)
            {
                case 24:
                    inputSet(KEY_Q, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 25:
                    inputSet(KEY_W, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 26:
                    inputSet(KEY_E, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 27:
                    inputSet(KEY_R, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 28:
                    inputSet(KEY_T, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 29:
                    inputSet(KEY_Y, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 30:
                    inputSet(KEY_U, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 31:
                    inputSet(KEY_I, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 32:
                    inputSet(KEY_O, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 33:
                    inputSet(KEY_P, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 38:
                    inputSet(KEY_A, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 39:
                    inputSet(KEY_S, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 40:
                    inputSet(KEY_D, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 41:
                    inputSet(KEY_F, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 42:
                    inputSet(KEY_G, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 43:
                    inputSet(KEY_H, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 44:
                    inputSet(KEY_J, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 45:
                    inputSet(KEY_K, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 46:
                    inputSet(KEY_L, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 52:
                    inputSet(KEY_Z, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 53:
                    inputSet(KEY_X, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 54:
                    inputSet(KEY_C, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 55:
                    inputSet(KEY_V, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 56:
                    inputSet(KEY_B, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 57:
                    inputSet(KEY_N, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
                case 58:
                    inputSet(KEY_M, event->response_type == XCB_KEY_PRESS ? PRESSED : RELEASED);
                    break;
            }
        }

        xcb_flush(connection);
        free(event);
    }
}

#endif