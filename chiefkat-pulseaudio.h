#ifndef CHIEFKAT_PORTAUDIO_H
#define CHIEFKAT_PORTAUDIO_H

#include <portaudio.h>

#include "chiefkat-audiodebugging.h"

#define verifyPA() verifyPAOrCrash(err, __FILE__, __LINE__)
static void verifyPAOrCrash(int err, cstr file, int line)
{
    verifyOrCrash(err == paNoError, Pa_GetErrorText(err), file, line);
}

static int offset = 0;
static int callback(const void *input, void *output,
        unsigned long frames,
        const PaStreamCallbackTimeInfo *timeInfo,
        PaStreamCallbackFlags status,
        void *userData)
{
    i16 *data = (i16 *)userData + offset;
    i16 *out = (i16 *)output;
    (void) input;
    int i;
    for (i = 0; i < frames; ++i)
    {
        *out++ = *data;
        *out++ = *data++;
    }

    return 0;
}

static i16 *sndData;
static void audioInit()
{
    int err = 0;

    err = Pa_Initialize();
    verifyPA();

    sndData = DEBUG_squareWaveDataInt16(DEBUG_SAMPLERATE, 256, 5);

    PaStream *stream;
    err = Pa_OpenDefaultStream(
        &stream, 0, DEBUG_CHANNELS, paInt16,
        DEBUG_SAMPLERATE, 256,
        callback, sndData);
    verifyPA();

    err = Pa_StartStream(stream);
    verifyPA();
}
static void audioBuffer()
{

}
static void audioWrite()
{

}
static void audioPlay()
{

}
static void audioStop()
{

}
static void audioExit()
{
    int err;

    err = Pa_Terminate();
    verifyPA();
}

#endif
