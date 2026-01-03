#ifndef MATH_C
#define MATH_C

#include "global_definitions.h"

#define random(min, height) rand() % ((height) - (min)) + min
#define PI 3.1415926535f

float lerp(float x, float y, float a)
{
    return (1.0f - a) * x + a * y;
}

#endif