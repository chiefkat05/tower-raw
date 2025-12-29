#ifdef _WIN32

#include <windows.h>
#include <debugapi.h>

#include "game.c"

void print(cstr msg)
{
    MessageBox(NULL, msg, "Program Failure", MB_OK);
}

LRESULT CALLBACK WinProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam);

static int WinWidth, WinHeight, WinOffset;
static float WinAspect;
static BITMAPINFO bmInfo;
void getWindowSize(HWND window)
{
    RECT winRect;
    GetClientRect(window, &winRect);
    WinWidth = winRect.right - winRect.left;
    WinHeight = winRect.bottom - winRect.top;
    WinAspect = (float)WinWidth / (float)WinHeight;
    WinOffset = WinWidth / 4;
}
void clearEntireWindow(HDC context)
{
    PixelBuffer window_buffer = {
        .width = WinWidth,
        .height = WinHeight,
        .bytes_per_pixel = 4
    };
    BITMAPINFOHEADER bmHeader = {
        .biSize = sizeof(bmHeader),
        .biWidth = window_buffer.width,
        .biHeight = window_buffer.height,
        .biPlanes = 1,
        .biBitCount = 8 * window_buffer.bytes_per_pixel,
        .biCompression = BI_RGB
    };
    BITMAPINFO bitmapInfo = (BITMAPINFO){
        .bmiHeader = bmHeader
    };

    window_buffer.data = VirtualAlloc(0, 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    verify(window_buffer.data != NULL, "failed to allocate window buffer data");

    u8 *byte = (u8 *)window_buffer.data;
    *byte = 0;
    StretchDIBits(context, 0, 0, window_buffer.width, window_buffer.height,
        0, 0, 1, 1,
        window_buffer.data, &bmInfo, DIB_RGB_COLORS, SRCCOPY);

    VirtualFree(window_buffer.data, 0, MEM_RELEASE);
}
void windowResize(HWND window)
{
    if (screen_buffer.data)
    { VirtualFree(screen_buffer.data, 0, MEM_RELEASE); }

    getWindowSize(window);
    screen_buffer = (PixelBuffer){
            .width = SCREEN_WIDTH,
            .height = SCREEN_HEIGHT,
            .bytes_per_pixel = 4,
    };

    BITMAPINFOHEADER bmHeader = {
        .biSize = sizeof(bmHeader),
        .biWidth = screen_buffer.width,
        .biHeight = screen_buffer.height,
        .biPlanes = 1,
        .biBitCount = 8 * screen_buffer.bytes_per_pixel,
        .biCompression = BI_RGB
    };
    bmInfo = (BITMAPINFO){
        .bmiHeader = bmHeader
    };

    int memory_size = screen_buffer.width * screen_buffer.height * screen_buffer.bytes_per_pixel;
    screen_buffer.data = VirtualAlloc(0, memory_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    verify(screen_buffer.data != NULL, "failed to allocate screen buffer data");

    HDC context = GetDC(window);
    clearEntireWindow(context);
}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE prevInst, LPSTR cmd, int show)
{
    
    WNDCLASS class = {
        .lpfnWndProc = WinProc,
        .hInstance = inst,
        .lpszClassName = "Window Class",
        .style = CS_HREDRAW | CS_VREDRAW
    };

    RegisterClass(&class);

    HWND window = CreateWindowExA(
            0, class.lpszClassName, "Tower", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            0, 0, inst, 0);

    verify(window != NULL, "failed to create window");

    HDC context = GetDC(window);
    
    windowResize(window);

    ShowWindow(window, show);
    running = true;

    float playerx = 0.0f, playery = 0.0f;
    ParticleManager snow = {.style=PARTICLE_SNOW, .x = 0, .y = 0, .w = 256, .h = 256};
    particleManagerGenerate(&snow);

    while (running)
    {
        inputUpdate();
        MSG msg = {};
        while (PeekMessageA(&msg, window, 0, 0, PM_REMOVE) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        const float speed = 0.2f;
        if (inputGet(KEY_W))
        {
            playery += speed;
        }
        if (inputGet(KEY_A))
        {
            playerx -= speed;
        }
        if (inputGet(KEY_S))
        {
            playery -= speed;
        }
        if (inputGet(KEY_D))
        {
            playerx += speed;
        }

        clearScreen(&screen_buffer);
        drawRect(playerx, playery, 16, 16);
        particleManagerUpdateAndDraw(&snow);
        StretchDIBits(context, WinOffset, 0, WinHeight, WinHeight,
            0, 0, screen_buffer.width, screen_buffer.height,
            screen_buffer.data, &bmInfo, DIB_RGB_COLORS, SRCCOPY);
    }

    if (screen_buffer.data)
    { VirtualFree(screen_buffer.data, 0, MEM_RELEASE); }

    return 0;
}

LRESULT CALLBACK WinProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (msg == WM_CLOSE || msg == WM_DESTROY || msg == WM_QUIT)
    {
        running = false;
        PostQuitMessage(0);
        return 0;
    }
    if (msg == WM_SIZE)
    {
        windowResize(window);
        return 0;
    }
    if (msg == WM_KEYDOWN || msg == WM_KEYUP)
    {
        switch(wparam)
        {
            case 'A':
                inputSet(KEY_A, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'B':
                inputSet(KEY_B, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'C':
                inputSet(KEY_C, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'D':
                inputSet(KEY_D, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'E':
                inputSet(KEY_E, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'F':
                inputSet(KEY_F, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'G':
                inputSet(KEY_G, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'H':
                inputSet(KEY_H, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'I':
                inputSet(KEY_I, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'J':
                inputSet(KEY_J, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'K':
                inputSet(KEY_K, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'L':
                inputSet(KEY_L, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'M':
                inputSet(KEY_M, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'N':
                inputSet(KEY_N, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'O':
                inputSet(KEY_O, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'P':
                inputSet(KEY_P, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'Q':
                inputSet(KEY_Q, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'R':
                inputSet(KEY_R, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'S':
                inputSet(KEY_S, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'T':
                inputSet(KEY_T, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'U':
                inputSet(KEY_U, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'V':
                inputSet(KEY_V, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'W':
                inputSet(KEY_W, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'X':
                inputSet(KEY_X, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'Y':
                inputSet(KEY_Y, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            case 'Z':
                inputSet(KEY_Z, msg == WM_KEYDOWN ? PRESSED : RELEASED);
                break;
            default:
                break;
        }
        return 0;
    }
    return DefWindowProc(window, msg, wparam, lparam);
}

#endif