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

PlayWav* PlayWav::g_inst = nullptr;

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
    const char* buf = reinterpret_cast<const char*>(rdbuf->buf());
	if (buf[ 0]=='R' && buf[ 1]=='I' && buf[ 2]=='F' && buf[ 3]=='F' &&
	    buf[ 8]=='W' && buf[ 9]=='A' && buf[10]=='V' && buf[11]=='E')
	{
        size_t ofs = 12;
        while (true) {
            const char* chunk_id = buf + ofs;
            const uint32_t size = getU32LE(buf + ofs + 4);
            if (memcmp(chunk_id, "fmt ", 4) == 0) {
                i2s_samp_freq_t sf;
                channels      = static_cast<uint16_t>(getU16LE(buf + ofs + 4 + 4 + 2)); // channels
                sf            = static_cast<i2s_samp_freq_t>(getU32LE(buf + ofs + 4 + 4 + 2 + 2)); // samplerate
                bitRateKbps   = static_cast<uint16_t>(getU32LE(buf + ofs + 4 + 4 + 2 + 2 + 4) /* bytepersec */ * 8 / 1000); // Kbps
                bitsPerSample = static_cast<uint16_t>(getU16LE(buf + ofs + 4 + 4 + 2 + 2 + 4 + 4 + 2)); // bitswidth
                reinitI2s = (sampFreq != sf);
                sampFreq = sf;
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
    skipToDataChunk();
    PlayAudio::setBufPos(fpos);
}

void PlayWav::decode()
{
    if (ap == nullptr) { return; }

    if (!playing || paused) {
        PlayAudio::decode();
        return;
    }

    audio_buffer_t* buffer;
    if ((buffer = take_audio_buffer(ap, false)) == nullptr) { return; }

    #ifdef DEBUG_PLAYWAV
    {
        uint32_t time = to_ms_since_boot(get_absolute_time());
        printf("WAV::decode start at %d ms\n", time);
    }
    #endif // DEBUG_PLAYWAV

    int32_t* samples = reinterpret_cast<int32_t*>(buffer->buffer->bytes);
    uint32_t accumL = 0;
    uint32_t accumR = 0;
    bool stopFlag = false;
    if (bitsPerSample == 16) {
        if (rdbuf->getLeft()/4 >= buffer->max_sample_count) {
            buffer->sample_count = buffer->max_sample_count;
        } else {
            buffer->sample_count = rdbuf->getLeft()/4;
        }
        const uint8_t* buf = rdbuf->buf();
        for (int i = 0; i < buffer->sample_count; i++, buf += 4) {
            int32_t buf_s16[2];
            buf_s16[0] = static_cast<int32_t>(static_cast<int16_t>((buf[1] << 8) | buf[0]));
            buf_s16[1] = static_cast<int32_t>(static_cast<int16_t>((buf[3] << 8) | buf[2]));
            samples[i*2+0] = buf_s16[0] * vol_table[volume] + DAC_ZERO;
            samples[i*2+1] = buf_s16[1] * vol_table[volume] + DAC_ZERO;
            accumL += buf_s16[0] * buf_s16[0] / 32768;
            accumR += buf_s16[1] * buf_s16[1] / 32768;
        }
        rdbuf->shift(buffer->sample_count*4);
        if (rdbuf->getLeft()/4 == 0) { stopFlag = true; }
    } else if (bitsPerSample == 24) {
        if (rdbuf->getLeft()/6 >= buffer->max_sample_count) {
            buffer->sample_count = buffer->max_sample_count;
        } else {
            buffer->sample_count = rdbuf->getLeft()/6;
        }
        const uint8_t* buf = rdbuf->buf();
        for (int i = 0; i < buffer->sample_count; i++, buf += 6) {
            int32_t buf_s24[2];
            buf_s24[0] = static_cast<int32_t>((buf[ 2] << 24) | (buf[ 1] << 16) | (buf[ 0] << 8)) / 256;
            buf_s24[1] = static_cast<int32_t>((buf[ 5] << 24) | (buf[ 4] << 16) | (buf[ 3] << 8)) / 256;
            if (vol_table[volume] >= 256) {  // keep 24bit resolution
                int32_t vol_div_256 = vol_table[volume] / 256;
                samples[i*2+0] = buf_s24[0] * vol_div_256 + DAC_ZERO;
                samples[i*2+1] = buf_s24[1] * vol_div_256 + DAC_ZERO;
            } else {  // spoil 24bit resolution
                int32_t vol = vol_table[volume];
                samples[i*2+0] = buf_s24[0] * vol / 256 + DAC_ZERO;
                samples[i*2+1] = buf_s24[1] * vol / 256 + DAC_ZERO;
            }
            accumL += ((buf_s24[0]/256) * (buf_s24[0]/256)) / 32768;
            accumR += ((buf_s24[1]/256) * (buf_s24[1]/256)) / 32768;
        }
        rdbuf->shift(buffer->sample_count*6);
        if (rdbuf->getLeft()/6 == 0) { stopFlag = true; }
    }
    give_audio_buffer(ap, buffer);
    incSamplesPlayed(buffer->sample_count);
    setLevelInt(accumL / buffer->sample_count, accumR / buffer->sample_count);
    if (stopFlag) { stop(); }

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
        static_cast<uint32_t>(static_cast<uint64_t>(dataSize) * 1000 / (static_cast<uint32_t>(sampFreq) * channels * bitsPerSample/8)),
        elapsedMillis()
    );
}