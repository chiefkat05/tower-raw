#ifndef CHIEFKAT_AUDIODEBUGGING_H
#define CHIEFKAT_AUDIODEBUGGING_H

/*  
    ** chiefkat05's audio-debugging header **

    A few functions and values purely intended for testing audio in a development environment.
    Not at all appropriate for a release build of any sort.

    This file shouldn't ever leave my personal stash, but in the unlikely case
    it makes it into the wild, consider this code to be licensed under zlib:

    Copyright (c) 2026 James Mathis

    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.

*/

#define DEBUG_SAMPLERATE 44100
#define DEBUG_CHANNELS 2

#include <stdlib.h>
#include <stdint.h>

void *DEBUG_squareWaveDataInt(int length, int tone, int volume)
{
    void *data = malloc(length);
    int *dataI = (int *)data;
    int i;
    volume <<= ((sizeof(int) * 8) - 8);
    for (i = 0; i < length / sizeof(int); ++i)
    {
        *dataI++ = (i % tone) > (tone / 2) ? volume : -volume;
    }

    return data;
}
/* length: number of bytes in buffer Example: 512 | tone: pitch of wave. Example: 128 | volume: how loud the wave should be. Example: 5 (10 and above is very loud) */
void *DEBUG_squareWaveDataInt16(int length, int tone, int volume)
{
    int period = DEBUG_SAMPLERATE / tone;
    int periodIndex = 0;

    void *data = malloc(length);
    int16_t *dataI = (int16_t *)data;
    int i;
    volume <<= ((sizeof(int16_t) * 8) - 8);
    for (i = 0; i < length / sizeof(int16_t); ++i)
    {
        *dataI++ = (periodIndex % period) > (period / 2) ? volume : -volume;
        ++periodIndex;
    }

    return data;
}

#endif