#ifndef INPUT_C
#define INPUT_C

#include "global_definitions.h"

static void inputSet(InputCode input, i32 value)
{
    InputValues[input] = value;
}
static void inputUpdate()
{
    int i;
    for (i = 0; i < total_key_count; ++i)
    {
        PrevInputValues[i] = InputValues[i];
    }
}
i32 inputGet(InputCode input)
{
    return InputValues[input];
}
bool inputJustReleased(InputCode input)
{
    if (InputValues[input] != PrevInputValues[input] && InputValues[input] == RELEASED)
    {
        return true;
    }
    return false;
}
bool inputJustPressed(InputCode input)
{
    if (InputValues[input] != PrevInputValues[input] && InputValues[input] == PRESSED)
    {
        return true;
    }
    return false;
}

#endif