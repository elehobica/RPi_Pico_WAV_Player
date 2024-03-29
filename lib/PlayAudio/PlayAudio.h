/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#ifndef __PLAY_AUDIO_H_INCLUDED__
#define __PLAY_AUDIO_H_INCLUDED__

#include "hardware/sync.h"
#include "audio_init.h"
#include "ff.h"
#include "ReadBuffer.h"
#include "audio_init.h"

//=================================
// Interface of PlayAudio Class
//=================================
class PlayAudio
{
public:
    typedef enum {
        AUDIO_CODEC_NONE = 0,
        AUDIO_CODEC_WAV
    } audio_codec_t;
    static void initialize();
    static void finalize();
    static void volumeUp();
    static void volumeDown();
    static void setVolume(uint8_t value);
    static uint8_t getVolume();
    PlayAudio();
    virtual ~PlayAudio();
    virtual void play(const char *filename, size_t fpos = 0, uint32_t samplesPlayed = 0);
    void pause(bool flg = true);
    void stop();
    bool isPlaying();
    bool isPaused();
    uint32_t elapsedMillis();
    virtual uint32_t totalMillis() = 0;
    virtual void getCurrentPosition(size_t *fpos, uint32_t *samplesPlayed);
    void getLevel(float *levelL, float *levelR);
protected:
    static const int RDBUF_SIZE = SAMPLES_PER_BUFFER * 4;
    static spin_lock_t *spin_lock;
    static audio_buffer_pool_t *ap;
    static ReadBuffer *rdbuf; // Read buffer for Audio codec stream
    static int16_t buf_s16[SAMPLES_PER_BUFFER*2]; // 16bit 2ch data before applying volume
    static uint8_t volume;
    static const uint32_t vol_table[101];
    FIL fil;
    bool playing;
    bool paused;
    uint16_t channels;
    uint16_t sampRateHz;
    uint16_t bitRateKbps;
    uint16_t bitsPerSample;
    uint32_t samplesPlayed;
    float levelL;
    float levelR;
    uint16_t getU16LE(const char *ptr);
    uint32_t getU32LE(const char *ptr);
    uint32_t getU28BE(const char *ptr);
    void setSamplesPlayed(uint32_t value);
    void incSamplesPlayed(uint32_t inc);
    uint32_t getSamplesPlayed();
    void setLevelInt(uint32_t levelIntL, uint32_t levelIntR);
    virtual void setBufPos(size_t fpos);
    virtual void decode();
private:
    float convLevelCurve(uint32_t levelInt);
};

#endif // __PLAY_AUDIO_H_INCLUDED__