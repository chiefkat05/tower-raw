#ifndef INPUT_C
#define INPUT_C

#include "global_definitions.h"

void inputSet(i32 *inputArray, InputCode input, i32 value)
{
    inputArray[input] = value;
}
static void inputUpdate(i32 *inputArray, i32 *prevInputArray)
{
    int i;
    for (i = 0; i < total_key_count; ++i)
    {
        prevInputArray[i] = inputArray[i];
    }
}
i32 inputGet(i32 *inputArray, InputCode input)
{
    return inputArray[input];
}
bool inputJustReleased(i32 *inputArray, i32 *prevInputArray, InputCode input)
{
    if (inputArray[input] != prevInputArray[input] && inputArray[input] == RELEASED)
    {
        return true;
    }
    return false;
}
bool inputJustPressed(i32 *inputArray, i32 *prevInputArray, InputCode input)
{
    if (inputArray[input] != prevInputArray[input] && inputArray[input] == PRESSED)
    {
        return true;
    }
    return false;
}

#endif