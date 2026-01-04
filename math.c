#ifndef MATH_C
#define MATH_C

#include "global_definitions.h"

#define random(min, height) rand() % ((height) - (min)) + min
#define PI 3.1415926535f

float lerp(float x, float y, float a)
{
    return (1.0f - a) * x + a * y;
}

int round_nearest(int num, int multi)
{
    if (num >= 0)
    {
        return ((num + multi / 2) / multi) * multi;
    } else
    {
        return ((num - multi / 2) / multi) * multi;
    }
}

#endif