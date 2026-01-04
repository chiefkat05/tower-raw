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

static void readFile(cstr path, void *out, u64 size);
static void writeFile(cstr path, void *in, u64 size);

#define SPAWNX 10.0f
#define SPAWNY 60.0f
typedef struct
{
    float playerx, playery, prevplayerx, prevplayery, playerfallspeed;
    bool playerjumped;
    ParticleSystem snow;
    ParticleSystem playerdust;

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
    particleSystemInit(&game.snow, PARTICLE_SNOW);
    particleSystemInit(&game.playerdust, PARTICLE_DUST);

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

    const float speed = 160.0f * TICK_SPEED;
    float tspeed = 0.0f;
    const float gravity = 320.0f * TICK_SPEED;
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
            game.playerjumped = true;
            game.playerfallspeed -= gravity * TICK_SPEED / 2;
        } else
        {
            if (game.playerjumped)
            {
                audioPlay(&game.step);
                particleSystemGenerate(&game.playerdust, 15, 15, game.playerx, game.playerx + 20, game.playery, game.playery);
            }
            game.playery = 60.0;
            game.playerjumped = false;
            game.playerfallspeed = 0.0f;
        }
        if (inputGet(KEY_W) && !game.playerjumped)
        {
            audioPlay(&game.step);
            game.playerfallspeed = 200.0f * TICK_SPEED;
            game.playerjumped = true;
        }
        if (inputGet(KEY_A))
        {
            tspeed = -speed;
        }
        if (inputGet(KEY_D))
        {
            tspeed = speed;
        }

        game.playery += game.playerfallspeed;
        game.playerx += tspeed;
        if (game.playery > 60.0)
        {
            game.playerfallspeed -= gravity * TICK_SPEED / 2;
        }

        static float timer = 0.0f;
        timer -= 5000.0f * TICK_SPEED;
        if (timer <= 0.0f)
        {
            timer = 1.0f;
            particleSystemGenerate(&game.snow, 1, MAX_PARTICLES, 0, screen_buffer.width, screen_buffer.height - 1, screen_buffer.height - 1);
        }
        particleSystemUpdate(&game.snow);
        particleSystemUpdate(&game.playerdust);

        accumulatedTime -= TICK_SPEED;
    }
    clearScreen(&screen_buffer);

    /* player (please fix the pxAlpha and pyAlpha to be not wiggly) */
    float pxAlpha = lerp(game.prevplayerx, game.playerx, accumulatedTime);
    float pyAlpha = lerp(game.prevplayery, game.playery, accumulatedTime);
    imageDraw(&game.plImg, game.playerx, game.playery);
    particleSystemDraw(&game.playerdust, accumulatedTime);


    /* floor */
    drawRect(0, 20, SCREEN_WIDTH, 40, 140, 140, 50);

    /* snow */
    particleSystemDraw(&game.snow, accumulatedTime);

    inputUpdate();
}

void gameExit()
{
    audioExit();
}

#endif