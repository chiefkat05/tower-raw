#ifndef AUDIO_C
#define AUDIO_C

#include "global_definitions.h"
void verifyAL()
{
    ALenum err = alGetError();
    
    if (err == AL_NO_ERROR)
    { return; }

    printf("OpenAL error %010x, message %s\n", err, alGetString(err));
    verify(false, "OpenAL failure");
}

static ALCdevice *aDevice;
static ALCcontext *aContext;
static void audioInit()
{
    aDevice = alcOpenDevice(NULL);

    aContext = alcCreateContext(aDevice, NULL);
    alcMakeContextCurrent(aContext);
    verifyAL();
}

static void audioStreamSquare(SoundStream *stream, int buffer, int tone)
{
    i16 sound[STREAMBUFFERSIZE * CHANNELS];
    int i, j, flip = 1;
    for (i = 0; i < STREAMBUFFERSIZE; ++i)
    {
        for (j = 0; j < CHANNELS; ++j)
        {
            sound[i * CHANNELS + j] = 260000 * flip;
        }
        if (i % tone > (tone / 2)) flip = -flip;
    }

    alBufferData(stream->buffers[buffer], AL_FORMAT_STEREO16, sound, STREAMBUFFERSIZE, SAMPLERATE);
}
static void audioSoundInit(Sound *snd, int length, int tone)
{
    alGenBuffers(1, &snd->buffer);

    i16 sound[length * CHANNELS];
    int i, j, flip = 1;
    for (i = 0; i < length; ++i)
    {
        for (j = 0; j < CHANNELS; ++j)
        {
            sound[i * CHANNELS + j] = 20000 * flip;
        }
        if (i % tone > (tone / 2)) flip = -flip;
    }

    alBufferData(snd->buffer, AL_FORMAT_STEREO16, sound, length, SAMPLERATE);

    alGenSources(1, &snd->source);
    alSourcei(snd->source, AL_BUFFER, snd->buffer);
}
static void audioStreamInit(SoundStream *stream, int tone) /* make this take a first buffer and a buffer count that loops? */
{
    alGenBuffers(SNDBUFFERCOUNT, stream->buffers);
    
    int i;
    for (i = 0; i < SNDBUFFERCOUNT; ++i)
    {
        audioStreamSquare(stream, i, tone);
    }

    alGenSources(1, &stream->source);
    alSourceQueueBuffers(stream->source, SNDBUFFERCOUNT, stream->buffers);
    stream->bufferCursor = 1;
}
static void audioStreamUpdateBuffers(SoundStream *stream, int tone)
{
    ALint processed;
    alGetSourcei(stream->source, AL_BUFFERS_PROCESSED, &processed);
    
    if (processed < 1)
    { return; }

    int endBuffer = (stream->bufferCursor + processed) % SNDBUFFERCOUNT;
    int i;
    if (endBuffer > stream->bufferCursor)
    {
        alSourceUnqueueBuffers(stream->source, processed, stream->buffers + stream->bufferCursor);
        for (i = 0; i < processed; ++i)
        {
            audioStreamSquare(stream, stream->bufferCursor + i, tone);
        }
        alSourceQueueBuffers(stream->source, processed, stream->buffers + stream->bufferCursor);

        stream->bufferCursor = endBuffer;
    } else if (endBuffer < stream->bufferCursor)
    {
        alSourceUnqueueBuffers(stream->source, SNDBUFFERCOUNT - stream->bufferCursor, stream->buffers + stream->bufferCursor);
        for (i = stream->bufferCursor; i < SNDBUFFERCOUNT; ++i)
        {
            audioStreamSquare(stream, i, tone);
        }
        alSourceQueueBuffers(stream->source, SNDBUFFERCOUNT - stream->bufferCursor, stream->buffers + stream->bufferCursor);

        alSourceUnqueueBuffers(stream->source, endBuffer, stream->buffers);
        for (i = 0; i < endBuffer; ++i)
        {
            audioStreamSquare(stream, i, tone);
        }
        alSourceQueueBuffers(stream->source, endBuffer, stream->buffers);

        stream->bufferCursor = endBuffer;
    }
}
static void audioPlayStream(SoundStream *stream)
{
    alSourcePlay(stream->source);
    verify(alGetError() == AL_NO_ERROR, "alSourcePlay failed");
}
static void audioPlay(Sound *snd)
{
    alSourcePlay(snd->source);
    verify(alGetError() == AL_NO_ERROR, "alSourcePlay failed");
}
static void audioExit()
{
    alcCloseDevice(aDevice);
    verify(alGetError() == AL_NO_ERROR, "alcCloseDevice failed");
}

#endif