/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include <cstdio>
#include <cstring>
#include <algorithm>
#include "pico/stdlib.h"
#include "PlayWav.h"

//#define DEBUG_PLAYWAV

PlayWav *PlayWav::g_inst = nullptr;

void PlayWav::decode_func()
{
    if (g_inst == nullptr) { return; }
    g_inst->decode();
}

PlayWav::PlayWav() : PlayAudio()
{
    g_inst = this;
}

PlayWav::~PlayWav()
{
}

void PlayWav::skipToDataChunk()
{
    const char *buf = (const char *) rdbuf->buf();
	if (buf[ 0]=='R' && buf[ 1]=='I' && buf[ 2]=='F' && buf[ 3]=='F' &&
	    buf[ 8]=='W' && buf[ 9]=='A' && buf[10]=='V' && buf[11]=='E')
	{
        size_t ofs = 12;
        while (true) {
            const char *chunk_id = buf + ofs;
            const uint32_t size = getU32LE(buf + ofs + 4);
            if (memcmp(chunk_id, "fmt ", 4) == 0) {
                channels =      (uint16_t) getU16LE(buf + ofs + 4 + 4 + 2); // channels
                sampRateHz =    (uint16_t) getU32LE(buf + ofs + 4 + 4 + 2 + 2); // samplerate
                bitRateKbps =   (uint16_t) (getU32LE(buf + ofs + 4 + 4 + 2 + 2 + 4) /* bytepersec */ * 8 / 1000); // Kbps
                bitsPerSample = (uint16_t) getU16LE(buf + ofs + 4 + 4 + 2 + 2 + 4 + 4 + 2); // bitswidth
            } else if (memcmp(chunk_id, "data", 4) == 0) {
                dataSize = size;
                break;
            }
            ofs += 8 + size;
            if (ofs + 8 > rdbuf->getLeft()) { return; }
        }
        rdbuf->shift(ofs + 8);
	}
}

void PlayWav::setBufPos(size_t fpos)
{
    if (fpos == 0) {
        skipToDataChunk();
    } else {
        rdbuf->seek(fpos);
    }
}

void PlayWav::decode()
{
    if (!playing || paused) {
        PlayAudio::decode();
        return;
    }

    audio_buffer_t *buffer;
    if ((buffer = take_audio_buffer(ap, false)) == nullptr) { return; }

    #ifdef DEBUG_PLAYWAV
    {
        uint32_t time = to_ms_since_boot(get_absolute_time());
        printf("WAV::decode start at %d ms\n", time);
    }
    #endif // DEBUG_PLAYWAV

    int32_t *samples = (int32_t *) buffer->buffer->bytes;
    if (rdbuf->getLeft()/4 >= buffer->max_sample_count) {
        buffer->sample_count = buffer->max_sample_count;
    } else {
        buffer->sample_count = rdbuf->getLeft()/4;
    }
    memcpy(buf_s16, rdbuf->buf(), buffer->sample_count*4); // This is 'decode' of WAV
    uint32_t accumL = 0;
    uint32_t accumR = 0;
    for (int i = 0; i < buffer->sample_count; i++) {
        samples[i*2+0] = (int32_t) buf_s16[i*2+0] * vol_table[volume] + DAC_ZERO;
        samples[i*2+1] = (int32_t) buf_s16[i*2+1] * vol_table[volume] + DAC_ZERO;
        accumL += ((int32_t) buf_s16[i*2+0] * buf_s16[i*2+0]) / 32768;
        accumR += ((int32_t) buf_s16[i*2+1] * buf_s16[i*2+1]) / 32768;
    }
    samplesPlayed += buffer->sample_count;
    give_audio_buffer(ap, buffer);

    setLevelInt(accumL / buffer->sample_count, accumR / buffer->sample_count);

    rdbuf->shift(buffer->sample_count*4);
    if (rdbuf->getLeft()/4 == 0) { stop(); }

    #ifdef DEBUG_PLAYWAV
    {
        uint32_t time = to_ms_since_boot(get_absolute_time());
        printf("WAV::decode end   at %d ms\n", time);
    }
    #endif // DEBUG_PLAYWAV
}

uint32_t PlayWav::totalMillis()
{
    return  std::max(
        (uint32_t) ((uint64_t) dataSize * 1000 / ((uint32_t) sampRateHz * channels * bitsPerSample/8)),
        elapsedMillis()
    );
}