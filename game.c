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

    SoundStream music;
    Sound step;

    Image plImg;

    bool initiated;
} GameScene;

static void gameInit(GameMemory *game_mem)
{
    GameScene *scene = (GameScene *)game_mem->data;
    verify(game_mem->size >= sizeof(GameScene), "not enough game data allocated");

    scene->playerx = SPAWNX;
    scene->playery = SPAWNY;
    particleSystemInit(&scene->snow, PARTICLE_SNOW);
    particleSystemInit(&scene->playerdust, PARTICLE_DUST);

    scene->plImg.sector_x_count = IMAGE_SIDE_SECTOR_COUNT;
    scene->plImg.sector_y_count = IMAGE_SIDE_SECTOR_COUNT;

    game_mem->readFile("img/img.bmp", scene->plImg.pixel_data, IMAGE_PIXEL_COUNT);
    
    scene->initiated = true;
}

void gameLoop(GameMemory *game_mem)
{
    GameScene *scene = (GameScene *)game_mem->data;
    if (!scene->initiated)
    {
        gameInit(game_mem);
    }
    /*
        on gameExit():
            alcCloseDevice(aDevice);
            verify(alGetError() == AL_NO_ERROR, "alcCloseDevice failed");
    */

    scene->accumulatedTime += game_mem->getDeltaTime();

    const float speed = 160.0f;
    float tspeed = 0.0f;
    const float gravity = 320.0f;

    while (scene->accumulatedTime >= TICK_SPEED)
    {
        scene->prevplayerx = scene->playerx;
        scene->prevplayery = scene->playery;

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
        /*
        static float timer = 0.0f;
        timer -= 5000.0f * TICK_SPEED;
        if (timer <= 0.0f)
        {
            timer = 1.0f;
            particleSystemGenerate(&scene->snow, 1, MAX_PARTICLES, camera_x_position, camera_x_position + game_mem->screen_buffer.width,
                camera_y_position + game_mem->screen_buffer.height - 1, camera_y_position + game_mem->screen_buffer.height - 1);
        }
        particleSystemUpdate(&scene->snow, &game_mem->screen_buffer);
        */
        particleSystemUpdate(&scene->playerdust, &game_mem->screen_buffer);

        scene->accumulatedTime -= TICK_SPEED;
    }
    clearScreen(&game_mem->screen_buffer);
    /*
    camera_x_position = -scene->playerx + game_mem->screen_buffer.width / 2;
    camera_y_position = -scene->playery + game_mem->screen_buffer.height / 2;
    */
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

    imageDraw(&game_mem->screen_buffer, &scene->plImg, scene->playerx, scene->playery);
    particleSystemDraw(&scene->playerdust, &game_mem->screen_buffer, scene->accumulatedTime);

    /* floor */
    drawRect(&game_mem->screen_buffer, 0, 20, SCREEN_WIDTH, 40, 140 << 16 | 140 << 8 | 50);

    /* snow */
    particleSystemDraw(&scene->snow, &game_mem->screen_buffer, scene->accumulatedTime);

    inputUpdate(game_mem->InputValues, game_mem->PrevInputValues);
}

#endif