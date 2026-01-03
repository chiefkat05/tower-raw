#ifndef GAME_C
#define GAME_C

#include "global_definitions.h"
#include "math.c"
#include "gfx.c"
#include "particles.c"
#include "input.c"
#include "audio.c"

static void *Allocate(u32 bytes);
static void Free(void *data);

#define SPAWNX 10.0f
#define SPAWNY 60.0f
typedef struct
{
    float playerx, playery, prevplayerx, prevplayery, playerfallspeed;
    bool playerjumped;
    ParticleSystem snow;

    SoundStream music;
    Sound step;

    Sprite plSprite[12];
    Image plImg;
} GameScene;
static GameScene game;

static void gameInit()
{
    game = (GameScene){};

    game.playerx = SPAWNX;
    game.playery = SPAWNY;
    game.snow = (ParticleSystem){.style=PARTICLE_SNOW, .x = 0, .y = 0, .w = screen_buffer.width, .h = screen_buffer.height};

    audioInit();
    audioStreamInit(&game.music, 128);
    audioPlayStream(&game.music);
    audioSoundInit(&game.step, 512, 256);

    int i;
    for (i = 0; i < 6; ++i)
    { spriteFillRandom(&game.plSprite[i]); }
    
    game.plImg.spriteSheet = &game.plSprite[0];
    game.plImg.sheetWidth = 2;
    game.plImg.sheetHeight = 3;
}

static float accumulatedTime;
static void gameLoop()
{
    accumulatedTime += getDeltaTime();

    audioStreamUpdateBuffers(&game.music, 128);

    const float speed = 80.0f * TICK_SPEED;
    const float gravity = 8.0f * TICK_SPEED;
    if (inputJustPressed(KEY_X))
    { audioPlay(&game.step); }

    while (accumulatedTime >= TICK_SPEED)
    {
        game.prevplayerx = game.playerx;
        game.prevplayery = game.playery;

        if (game.playery < 0)
        {
            game.playerx = SPAWNX;
            game.playery = SPAWNY;
        }
        if (game.playery > 60.0)
        {
            /* make the velocity work at any tickspeeds and such (something to do with dividing by half or something idk) */
            game.playery += game.playerfallspeed;
            game.playerjumped = true;
            game.playerfallspeed -= gravity;
        } else
        {
            if (game.playerjumped)
            { audioPlay(&game.step); }
            game.playery = 60.0;
            game.playerjumped = false;
            game.playerfallspeed = 0.0f;
        }
        if (inputGet(KEY_W) && !game.playerjumped)
        {
            audioPlay(&game.step);
            game.playerfallspeed = 200.0f * TICK_SPEED;
            game.playery += game.playerfallspeed;
            game.playerjumped = true;
        }
        if (inputGet(KEY_A))
        {
            game.playerx -= speed;
        }
        if (inputGet(KEY_D))
        {
            game.playerx += speed;
        }

        static float timer = 0.0f;
        timer -= 5000.0f * TICK_SPEED;
        if (timer <= 0.0f)
        {
            timer = 1.0f;
            particleManagerGenerate(&game.snow, 1, MAX_PARTICLES);
        }
        particleManagerUpdate(&game.snow);

        accumulatedTime -= TICK_SPEED;
    }
    float pxAlpha = lerp(game.prevplayerx, game.playerx, accumulatedTime);
    float pyAlpha = lerp(game.prevplayery, game.playery, accumulatedTime);

    clearScreen(&screen_buffer);
    
    /* floor */
    drawRect(SCREEN_WIDTH / 2, 20, SCREEN_WIDTH, 40, 140, 140, 50);

    /* snow */
    particleManagerDraw(&game.snow, accumulatedTime);

    /* player */
    imageDraw(&game.plImg, game.playerx, game.playery);

    inputUpdate();
}

void gameExit()
{
    audioExit();
}

#endif