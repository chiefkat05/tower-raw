#ifndef PARTICLES_C
#define PARTICLES_C

#include "global_definitions.h"

static void particleManagerAdd(ParticleSystem *manager, float xpos, float ypos)
{
    int particleIndex = -1;

    int i;
    for (i = 0; i < manager->particle_limit; ++i)
    {
        if (!manager->alive[i])
        {
            particleIndex = i;
            break;
        }
    }
    if (particleIndex == -1)
    { return; }

    manager->x_positions[particleIndex] = xpos;
    manager->y_positions[particleIndex] = ypos;
    manager->alive[particleIndex] = true;

    manager->current_editing_particle = particleIndex;
}
static void particleManagerPush(ParticleSystem *manager, float xpush, float ypush)
{
    manager->x_velocity[manager->current_editing_particle] = xpush;
    manager->y_velocity[manager->current_editing_particle] = ypush;
}
static void particleManagerColor(ParticleSystem *manager, u8 red, u8 green, u8 blue)
{
    manager->red[manager->current_editing_particle] = red;
    manager->green[manager->current_editing_particle] = green;
    manager->blue[manager->current_editing_particle] = blue;
}
static void particleManagerGenerate(ParticleSystem *manager, int particles, int limit)
{
    manager->particle_limit = limit;
    int i;

    switch(manager->style)
    {
    case PARTICLE_SNOW:
        for (i = 0; i < particles; ++i)
        {
            particleManagerAdd(manager, random(0, screen_buffer.width), screen_buffer.height - 1);
            int shade = random(200, 255);
            particleManagerColor(manager, shade, shade, shade);
            particleManagerPush(manager, (float)(random(400, 600)) / 4.0f, -(float)(random(200, 500)) / 4.0f);
        }
        break;
    default:
        break;
    }
}
static void particleManagerUpdate(ParticleSystem *manager)
{
    int p;
    for (p = 0; p < MAX_PARTICLES; ++p)
    {
        if (!manager->alive[p])
        { return; }
        manager->prev_x_positions[p] = manager->x_positions[p];
        manager->prev_y_positions[p] = manager->y_positions[p];
        manager->x_positions[p] += manager->x_velocity[p] * TICK_SPEED;
        manager->y_positions[p] += manager->y_velocity[p] * TICK_SPEED;
    }
}
static void particleManagerDraw(ParticleSystem *manager, float accumulatedTime)
{
    int p;
    for (p = 0; p < MAX_PARTICLES; ++p)
    {
        if (!manager->alive[p])
        { continue; }
        float xAlpha = lerp(manager->prev_x_positions[p], manager->x_positions[p], accumulatedTime);
        float yAlpha = lerp(manager->prev_y_positions[p], manager->y_positions[p], accumulatedTime);

        drawPixel(manager->x_positions[p] + xAlpha, manager->y_positions[p] + yAlpha,
                manager->red[p], manager->green[p], manager->blue[p], true);
    }
}

#endif