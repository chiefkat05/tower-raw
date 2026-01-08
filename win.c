#ifdef _WIN32

#include <windows.h>
#include <dsound.h>
#include <xinput.h>

#include "global_definitions.h"

#define GAME_LOOP(name) void name(GameMemory *game_mem)
typedef GAME_LOOP(gameLoop);
GAME_LOOP(gameLoopStub)
{
}
#define INPUT_SET(name) void name(i32 *inputArray, InputCode input, i32 value)
typedef INPUT_SET(inputSet);
INPUT_SET(inputSetStub)
{
}

READ_FILE(readFile)
{
    HANDLE file = CreateFile(path, GENERIC_READ, FILE_SHARE_READ,
                    NULL, OPEN_EXISTING, 0, 0);
    verify(file, "failed to open file");

    ReadFile(file, out, size, NULL, NULL);

    CloseHandle(file);
}
WRITE_FILE(writeFile)
{

}
static LARGE_INTEGER currentTime, prevTime, freqTime;
GET_DELTATIME(getDeltaTime)
{
    prevTime = currentTime;
    QueryPerformanceCounter(&currentTime);
    unsigned long deltaCycles = currentTime.QuadPart - prevTime.QuadPart;

    return (float)(deltaCycles) / (float)freqTime.QuadPart;
}

typedef struct
{
    HMODULE gameLib;
    gameLoop *gameLoop;
    inputSet *inputSet;
    bool isValid;
} gameFunctions;
static gameFunctions gameFunc;

static void loadGameCode()
{
    if (gameFunc.gameLib)
    { return; }
    
    gameFunc.isValid = false;

    CopyFile("game.dll", "game_temp.dll", FALSE);
    gameFunc.gameLib = LoadLibraryA("game_temp.dll");
    if (gameFunc.gameLib)
    {
        gameFunc.gameLoop = (gameLoop *)GetProcAddress(gameFunc.gameLib, "gameLoop");
        gameFunc.inputSet = (inputSet *)GetProcAddress(gameFunc.gameLib, "inputSet");
        gameFunc.isValid = (gameFunc.gameLoop && gameFunc.inputSet);
    }
    
    if (gameFunc.isValid)
    { return; }

    printf("game functions not valid\n");
    gameFunc.gameLoop = gameLoopStub;
    gameFunc.inputSet = inputSetStub;
}
static void unloadGameCode()
{
    if (gameFunc.gameLib)
    {
        FreeLibrary(gameFunc.gameLib);
        gameFunc.gameLib = NULL;
    }
    gameFunc.isValid = false;
    gameFunc.gameLoop = gameLoopStub;
    gameFunc.inputSet = inputSetStub;
}

LRESULT CALLBACK WinProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam);

static int WinWidth, WinHeight, WinOffsetX, WinOffsetY;
static int ScreenScaleX, ScreenScaleY;
static BITMAPINFO bmInfo;
static void getWindowSize(HWND window)
{
    RECT winRect;
    GetClientRect(window, &winRect);
    WinWidth = winRect.right - winRect.left;
    WinHeight = winRect.bottom - winRect.top;

    /* side closest to it's screen_width cousin */
    float WidthDiff = (float)WinWidth / (float)SCREEN_WIDTH;
    float HeightDiff = (float)WinHeight / (float)SCREEN_HEIGHT;
    float widthAspect = WidthDiff / MIN(WidthDiff, HeightDiff);
    float heightAspect = HeightDiff / MIN(WidthDiff, HeightDiff);

    ScreenScaleX = (float)WinWidth / widthAspect;
    ScreenScaleY = (float)WinHeight / heightAspect;
    WinOffsetX = WinWidth / 2 - (ScreenScaleX / 2.0f);
    WinOffsetY = WinHeight / 2 - (ScreenScaleY / 2.0f);
}
static void clearEntireWindow(HDC context)
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
static void windowResize(HWND window)
{
    getWindowSize(window);

    HDC context = GetDC(window);
    clearEntireWindow(context);
}

#define verifyDS() verifyDirectSound(err, __FILE__, __LINE__)
void verifyDirectSound(HRESULT err, cstr file, int line)
{
    verifyOrCrash(err == DS_OK, "DirectSound failure", file, line);
}
static LPDIRECTSOUND audioInit()
{
    LPDIRECTSOUND dsound;
    HRESULT err = DirectSoundCreate(NULL, &dsound, NULL);
    verifyDS();

    return dsound;
}
static LPDIRECTSOUNDBUFFER audioBuffer(LPDIRECTSOUND dsound)
{
    HRESULT err;

    WAVEFORMATEX format = {};
    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = CHANNELS;
    format.nSamplesPerSec = SAMPLERATE;
    format.wBitsPerSample = 32;
    format.nBlockAlign = (format.nChannels * format.wBitsPerSample) / 8;
    format.nAvgBytesPerSec = format.nSamplesPerSec *
            format.nBlockAlign;

    DSBUFFERDESC desc = {};
    desc.dwSize = sizeof(desc);
    desc.dwBufferBytes = 3 * format.nAvgBytesPerSec;
    desc.lpwfxFormat = &format;

    LPDIRECTSOUNDBUFFER buf;
    err = IDirectSound_CreateSoundBuffer(dsound,
                &desc, &buf, NULL);
    verifyDS();

    return buf;
}
static void audioWrite(LPDIRECTSOUNDBUFFER buf, void *inData, int length)
{
    HRESULT err;

    DWORD playCursor;
    err = IDirectSoundBuffer_GetCurrentPosition(buf, &playCursor, NULL);
    verifyDS();
    
    void *dataA, *dataB;
    DWORD lengthA, lengthB;
    int latency = GAME_SOUND_BUFFER_SIZE;
    int target = (playCursor + latency) % (3 * 8 * SAMPLERATE);
    err = IDirectSoundBuffer_Lock(buf, target, length, &dataA, &lengthA, &dataB, &lengthB, 0);
    verifyDS();
    
    if (GAME_SOUND_BUFFER_SIZE <= lengthA)
    {
        CopyMemory(dataA, inData, GAME_SOUND_BUFFER_SIZE);
    } else {
        CopyMemory(dataA, inData, lengthA);
        CopyMemory(dataB, inData + lengthA, GAME_SOUND_BUFFER_SIZE - lengthA);
    }

    err = IDirectSoundBuffer_Unlock(buf, dataA, lengthA, dataB, lengthB);
    verifyDS();

    err = IDirectSoundBuffer_Play(buf, 0, 0, 0);
    verifyDS();
}

static bool running;
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
            CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
            0, 0, inst, 0);

    verify(window != NULL, "failed to create window");

    HDC context = GetDC(window);

    loadGameCode();

    GameMemory game_memory = {};
    game_memory.size = GAME_MEMORY_SIZE;
    game_memory.data = VirtualAlloc(0, game_memory.size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    verify(game_memory.data, "failed to allocate game data");
    game_memory.readFile = readFile;
    game_memory.writeFile = writeFile;
    game_memory.getDeltaTime = getDeltaTime;
    
    game_memory.screen_buffer = (PixelBuffer){
            .width = SCREEN_WIDTH,
            .height = SCREEN_HEIGHT,
            .bytes_per_pixel = 4,
    };

    BITMAPINFOHEADER bmHeader = {
        .biSize = sizeof(bmHeader),
        .biWidth = game_memory.screen_buffer.width,
        .biHeight = game_memory.screen_buffer.height,
        .biPlanes = 1,
        .biBitCount = 8 * game_memory.screen_buffer.bytes_per_pixel,
        .biCompression = BI_RGB
    };
    bmInfo = (BITMAPINFO){
        .bmiHeader = bmHeader
    };

    int memory_size = game_memory.screen_buffer.width * game_memory.screen_buffer.height * game_memory.screen_buffer.bytes_per_pixel;
    game_memory.screen_buffer.data = VirtualAlloc(0, memory_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    verify(game_memory.screen_buffer.data != NULL, "failed to allocate screen buffer data");

    windowResize(window);

    ShowWindow(window, show);

    running = true;
    QueryPerformanceFrequency(&freqTime);
    getDeltaTime();

    LPDIRECTSOUND dSound = audioInit();
    LPDIRECTSOUNDBUFFER sound = audioBuffer(dSound);

    int frame = 200;
    while (running)
    {
        if (frame-- <= 0)
        {
            unloadGameCode();
            loadGameCode();
            frame = 200;
        }
        MSG msg = {};
        while (PeekMessageA(&msg, window, 0, 0, PM_REMOVE) > 0)
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_KEYDOWN || msg.message == WM_KEYUP)
            {
                switch(msg.wParam)
                {
                    case 'A':
                        gameFunc.inputSet(game_memory.InputValues, KEY_A, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'B':
                        gameFunc.inputSet(game_memory.InputValues, KEY_B, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'C':
                        gameFunc.inputSet(game_memory.InputValues, KEY_C, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'D':
                        gameFunc.inputSet(game_memory.InputValues, KEY_D, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'E':
                        gameFunc.inputSet(game_memory.InputValues, KEY_E, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'F':
                        gameFunc.inputSet(game_memory.InputValues, KEY_F, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'G':
                        gameFunc.inputSet(game_memory.InputValues, KEY_G, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'H':
                        gameFunc.inputSet(game_memory.InputValues, KEY_H, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'I':
                        gameFunc.inputSet(game_memory.InputValues, KEY_I, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'J':
                        gameFunc.inputSet(game_memory.InputValues, KEY_J, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'K':
                        gameFunc.inputSet(game_memory.InputValues, KEY_K, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'L':
                        gameFunc.inputSet(game_memory.InputValues, KEY_L, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'M':
                        gameFunc.inputSet(game_memory.InputValues, KEY_M, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'N':
                        gameFunc.inputSet(game_memory.InputValues, KEY_N, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'O':
                        gameFunc.inputSet(game_memory.InputValues, KEY_O, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'P':
                        gameFunc.inputSet(game_memory.InputValues, KEY_P, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'Q':
                        gameFunc.inputSet(game_memory.InputValues, KEY_Q, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'R':
                        gameFunc.inputSet(game_memory.InputValues, KEY_R, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'S':
                        gameFunc.inputSet(game_memory.InputValues, KEY_S, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'T':
                        gameFunc.inputSet(game_memory.InputValues, KEY_T, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'U':
                        gameFunc.inputSet(game_memory.InputValues, KEY_U, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'V':
                        gameFunc.inputSet(game_memory.InputValues, KEY_V, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'W':
                        gameFunc.inputSet(game_memory.InputValues, KEY_W, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'X':
                        gameFunc.inputSet(game_memory.InputValues, KEY_X, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'Y':
                        gameFunc.inputSet(game_memory.InputValues, KEY_Y, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case 'Z':
                        gameFunc.inputSet(game_memory.InputValues, KEY_Z, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case VK_RETURN:
                        gameFunc.inputSet(game_memory.InputValues, KEY_ENTER, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case VK_SHIFT:
                        gameFunc.inputSet(game_memory.InputValues, KEY_SHIFT, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case VK_CONTROL:
                        gameFunc.inputSet(game_memory.InputValues, KEY_CTRL, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case VK_TAB:
                        gameFunc.inputSet(game_memory.InputValues, KEY_TAB, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case VK_SPACE:
                        gameFunc.inputSet(game_memory.InputValues, KEY_SPACE, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    case VK_ESCAPE:
                        gameFunc.inputSet(game_memory.InputValues, KEY_ESC, msg.message == WM_KEYDOWN ? PRESSED : RELEASED);
                        break;
                    default:
                        break;
                }
            }
        }

        gameFunc.gameLoop(&game_memory);
        audioWrite(sound, game_memory.soundBuffer, GAME_SOUND_BUFFER_SIZE);

        StretchDIBits(context,
            WinOffsetX, WinOffsetY,
            ScreenScaleX, ScreenScaleY,
            0, 0, game_memory.screen_buffer.width, game_memory.screen_buffer.height,
            game_memory.screen_buffer.data, &bmInfo, DIB_RGB_COLORS, SRCCOPY);
    }

    VirtualFree(game_memory.data, 0, MEM_RELEASE);

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
    return DefWindowProc(window, msg, wparam, lparam);
}

#endif