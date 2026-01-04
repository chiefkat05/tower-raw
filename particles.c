#ifndef PARTICLES_C
#define PARTICLES_C

#include "global_definitions.h"

/* particles */

typedef enum
{
    PARTICLE_SNOW,
    PARTICLE_DUST
} ParticleStyle;

#define MAX_PARTICLES 9000
typedef struct
{
    ParticleStyle style;

    float prev_x_positions[MAX_PARTICLES];
    float prev_y_positions[MAX_PARTICLES];
    float x_positions[MAX_PARTICLES];
    float y_positions[MAX_PARTICLES];
    float x_velocity[MAX_PARTICLES];
    float y_velocity[MAX_PARTICLES];

    float x_velocitypush[MAX_PARTICLES];
    float y_velocitypush[MAX_PARTICLES];

    bool x_loop;
    bool y_loop;

    bool alive[MAX_PARTICLES];
    float red[MAX_PARTICLES];
    float green[MAX_PARTICLES];
    float blue[MAX_PARTICLES];
    float red_velocity[MAX_PARTICLES];
    float green_velocity[MAX_PARTICLES];
    float blue_velocity[MAX_PARTICLES];

    float life_time[MAX_PARTICLES];
    bool life_limited[MAX_PARTICLES];

    int particle_limit;
    int current_editing_particle;
} ParticleSystem;

static void particleSystemInit(ParticleSystem *system, ParticleStyle style)
{
    system->style = style;

    switch(style)
    {
        case PARTICLE_SNOW:
            system->x_loop = true;
            break;
        case PARTICLE_DUST:
            break;
        default:
            break;
    }
}

static void particleSystemAdd(ParticleSystem *system, float xpos, float ypos)
{
    int particleIndex = -1;

    int i;
    for (i = 0; i < system->particle_limit; ++i)
    {
        if (!system->alive[i])
        {
            particleIndex = i;
            break;
        }
    }
    if (particleIndex == -1)
    { return; }

    system->x_positions[particleIndex] = xpos;
    system->y_positions[particleIndex] = ypos;
    system->alive[particleIndex] = true;

    system->current_editing_particle = particleIndex;
}
static void particleSystemPush(ParticleSystem *system, float xpush, float ypush) /* maybe multiply the push variables instead of setting them? */
{
    system->x_velocitypush[system->current_editing_particle] = xpush;
    system->y_velocitypush[system->current_editing_particle] = ypush;
}
static void particleSystemMove(ParticleSystem *system, float xvel, float yvel)
{
    system->x_velocity[system->current_editing_particle] = xvel;
    system->y_velocity[system->current_editing_particle] = yvel;
}
static void particleSystemColorMove(ParticleSystem *system, float redvel, float greenvel, float bluevel)
{
    system->red_velocity[system->current_editing_particle] = redvel;
    system->green_velocity[system->current_editing_particle] = greenvel;
    system->blue_velocity[system->current_editing_particle] = bluevel;
}
static void particleSystemLifespan(ParticleSystem *system, float time)
{
    system->life_limited[system->current_editing_particle] = true;
    system->life_time[system->current_editing_particle] = time;
}
static void particleSystemColor(ParticleSystem *system, u8 red, u8 green, u8 blue)
{
    system->red[system->current_editing_particle] = red;
    system->green[system->current_editing_particle] = green;
    system->blue[system->current_editing_particle] = blue;
}
static void particleSystemGenerate(ParticleSystem *system, int particles, int limit, int xmin, int xmax, int ymin, int ymax)
{
    system->particle_limit = limit;
    int i;
    for (i = 0; i < particles; ++i)
    {
        int xpos = xmin;
        int ypos = ymin;
        if (xmax > xmin) { xpos = random(xmin, xmax); }
        if (ymax > ymin) { ypos = random(ymin, ymax); }
        particleSystemAdd(system, xpos, ypos);

        switch(system->style)
        {
            case PARTICLE_SNOW:
                {
                int shade = random(200, 255);
                particleSystemColor(system, shade, shade, shade);
                particleSystemMove(system, (float)(random(100, 150)), -(float)(random(50, 125)));
                } break;
            case PARTICLE_DUST:
                {
                int shade = random(100, 150);
                particleSystemColor(system, shade, shade, shade);
                particleSystemMove(system, (float)(random(-80, 80)), (float)(random(60, 120)));
                particleSystemLifespan(system, ((float)random(200, 300)) / 1000.0f);
                } break;
            default:
                break;
        }
    }
}
static void particleSystemUpdate(ParticleSystem *system)
{
    int p;
    for (p = 0; p < MAX_PARTICLES; ++p)
    {
        if (!system->alive[p])
        { continue; }

        system->prev_x_positions[p] = system->x_positions[p];
        system->prev_y_positions[p] = system->y_positions[p];
        system->x_velocity[p] += system->x_velocitypush[p] * TICK_SPEED / 2;
        system->y_velocity[p] += system->y_velocitypush[p] * TICK_SPEED / 2;

        system->x_positions[p] += system->x_velocity[p] * TICK_SPEED;
        system->y_positions[p] += system->y_velocity[p] * TICK_SPEED;

        system->x_velocity[p] += system->x_velocitypush[p] * TICK_SPEED / 2;
        system->y_velocity[p] += system->y_velocitypush[p] * TICK_SPEED / 2;

        system->red[p] -= system->red_velocity[p] * TICK_SPEED;
        system->green[p] -= system->green_velocity[p] * TICK_SPEED;
        system->blue[p] -= system->blue_velocity[p] * TICK_SPEED;

        if (system->life_limited[p])
        {
            system->life_time[p] -= TICK_SPEED;

            if (system->life_time[p] <= 0.0f)
            {
                system->alive[p] = false;
            }
        }

        /* system-specific activities */
        switch(system->style)
        {
            case PARTICLE_SNOW:
                if (system->y_positions[p] < 0)
                {
                    system->alive[p] = false;
                }
                break;
            case PARTICLE_DUST:
                system->y_velocitypush[p] = -800;
                break;
            default:
                break;
        }
    }

}
static void particleSystemDraw(ParticleSystem *system, float accumulatedTime)
{
    int p;
    for (p = 0; p < MAX_PARTICLES; ++p)
    {
        if (!system->alive[p])
        { continue; }
        float xAlpha = lerp(system->prev_x_positions[p], system->x_positions[p], accumulatedTime);
        float yAlpha = lerp(system->prev_y_positions[p], system->y_positions[p], accumulatedTime);

        drawPixel(xAlpha, yAlpha,
                system->red[p], system->green[p], system->blue[p], system->x_loop, system->y_loop);
    }
}

#endif