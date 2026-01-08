#ifndef GAME_C
#define GAME_C

#include "global_definitions.h"
#include "math.c"
#include "gfx.c"
#include "particles.c"
#include "input.c"

#define SPAWNX 10.0f
#define SPAWNY 60.0f
typedef struct
{
    float accumulatedTime;

    float playerx, playery, prevplayerx, prevplayery, playerfallspeed;
    bool playerjumped;
    ParticleSystem snow;
    ParticleSystem playerdust;

    bool playerhit;
    float enemyX, enemyY;

    Image plImg;

    bool initiated;
} GameScene;

#include <math.h>
static void gameInit(GameMemory *game_mem)
{
    GameScene *scene = (GameScene *)game_mem->data;
    verify(game_mem->size >= sizeof(GameScene), "not enough game data allocated");

    scene->playerx = SPAWNX;
    scene->playery = SPAWNY;
    particleSystemInit(&scene->snow, PARTICLE_SNOW);
    particleSystemInit(&scene->playerdust, PARTICLE_DUST);

    scene->plImg.sector_x_count = 1;
    scene->plImg.sector_y_count = 1;

    game_mem->readFile("img/img.bmp", scene->plImg.pixel_data, IMAGE_PIXEL_COUNT);
    
    scene->initiated = true;
}

static void soundSine(GameMemory *game_mem)
{
    int tone = 128;
    int volume = 25;
    float period = SAMPLERATE / tone;

    volume <<= ((sizeof(i32) * 8) - 8);
    int i;
    for (i = 0; i < GAME_SOUND_BUFFER_SIZE; ++i)
    {
        float t = 2.0f * PI * game_mem->sineIndex / period;
        game_mem->soundBuffer[i] = (i32)(sin(t) * volume);
        ++game_mem->sineIndex;
    }
}
static void soundNone(GameMemory *game_mem)
{
    int i;
    for (i = 0; i < GAME_SOUND_BUFFER_SIZE; ++i)
    {
        game_mem->soundBuffer[i] = 0;
    }
}

void gameLoop(GameMemory *game_mem)
{
    GameScene *scene = (GameScene *)game_mem->data;
    if (!scene->initiated)
    {
        gameInit(game_mem);
    }

    scene->accumulatedTime += game_mem->getDeltaTime();

    float speed = 160.0f, enemySpeed = 0.3f;
    float tspeed = 0.0f;
    float gravity = 320.0f;

    while (scene->accumulatedTime >= TICK_SPEED)
    {
        scene->prevplayerx = scene->playerx;
        scene->prevplayery = scene->playery;

        if (inputGet(game_mem->InputValues, KEY_S))
        {
            if (scene->playerfallspeed > 0.0f)
            { scene->playerfallspeed = 0.0f; }
            gravity *= 100.0f;
        }
        
        if (scene->playery > 60.0)
        {
            scene->playerjumped = true;
            scene->playerfallspeed -= gravity * TICK_SPEED / 2;
        } else
        {
            if (scene->playerjumped)
            {
                particleSystemGenerate(&scene->playerdust, 15, 15, scene->playerx, scene->playerx + 20, scene->playery, scene->playery);
            }
            scene->playery = 60.0;
            scene->playerjumped = false;
            scene->playerfallspeed = 0.0f;
        }
        if (inputGet(game_mem->InputValues, KEY_W) && !scene->playerjumped)
        {
            scene->playerfallspeed = speed;
            scene->playerjumped = true;
        }
        if (inputGet(game_mem->InputValues, KEY_SHIFT))
        {
            speed *= 2;
        }
        if (inputGet(game_mem->InputValues, KEY_A))
        {
            tspeed = -speed;
        }
        if (inputGet(game_mem->InputValues, KEY_D))
        {
            tspeed = speed;
        }
        
        scene->playerx += tspeed * TICK_SPEED;
        scene->playery += scene->playerfallspeed * TICK_SPEED;
        if (scene->playery > 60.0)
        {
            scene->playerfallspeed -= gravity * TICK_SPEED / 2;
        }

        scene->enemyY -= enemySpeed;
        if (scene->enemyY < 0.0f)
        {
            scene->enemyY = 200.0f;
            scene->enemyX = scene->playerx;
        }
        if ((int)scene->playerx == (int)scene->enemyX && (int)scene->playery == (int)scene->enemyY)
        {
            scene->playerhit = true;
        }

        particleSystemUpdate(&scene->playerdust, &game_mem->screen_buffer);

        scene->accumulatedTime -= TICK_SPEED;
    }
    clearScreen(&game_mem->screen_buffer);

    /* just keep up with handmade hero now that you've done this and fix it when he implements wav files (also make sure linux version is up to par with alsa)  */
    /*soundSine(game_mem);*/
    if (scene->playerhit)
    {
        soundNone(game_mem);
        return;
    }
    if (scene->playerx < game_mem->screen_buffer.camera_x_position + game_mem->screen_buffer.width / (float)(5.0f/1.5f))
    {
        game_mem->screen_buffer.camera_x_position = scene->playerx - game_mem->screen_buffer.width / (float)(5.0f/1.5f);
    }
    if (scene->playerx > game_mem->screen_buffer.camera_x_position + game_mem->screen_buffer.width / (float)(5.0f/3.0f))
    {
        game_mem->screen_buffer.camera_x_position = scene->playerx - game_mem->screen_buffer.width / (float)(5.0f/3.0f);
    }

    /* player (please fix the pxAlpha and pyAlpha to be not wiggly) */
    /*float pxAlpha = lerp(scene->prevplayerx, scene->playerx, scene->accumulatedTime);
    float pyAlpha = lerp(scene->prevplayery, scene->playery, scene->accumulatedTime);*/

    drawRect(&game_mem->screen_buffer, scene->enemyX, scene->enemyY, 2, 2, 255 << 16);

    imageDraw(&game_mem->screen_buffer, &scene->plImg, scene->playerx, scene->playery);
    particleSystemDraw(&scene->playerdust, &game_mem->screen_buffer, scene->accumulatedTime);

    /* floor */
    drawRect(&game_mem->screen_buffer, 0, 20, SCREEN_WIDTH, 40, 140 << 16 | 140 << 8 | 50);

    /* snow */
    particleSystemDraw(&scene->snow, &game_mem->screen_buffer, scene->accumulatedTime);

    inputUpdate(game_mem->InputValues, game_mem->PrevInputValues);
}

#endif