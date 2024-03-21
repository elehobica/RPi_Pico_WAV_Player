/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include "PlayWav.h"

#include <cstdio>
#include <cstring>
#include <algorithm>
#include "pico/stdlib.h"
#include "ReadBuffer.h"

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
    static int decodeCount = 0;
    uint64_t start = to_us_since_boot(get_absolute_time());
    #endif // DEBUG_PLAYWAV

    int32_t* samples = reinterpret_cast<int32_t*>(buffer->buffer->bytes);
    uint32_t accum[2] = {};
    bool stopFlag = false;
    const uint8_t* buf = rdbuf->buf();
    if (bitsPerSample == 16) {
        if (rdbuf->getLeft()/4 >= buffer->max_sample_count) {
            buffer->sample_count = buffer->max_sample_count;
        } else {
            buffer->sample_count = rdbuf->getLeft()/4;
        }
        for (int i = 0; i < buffer->sample_count; i++, buf += 4) {
            for (int j = 0; j < 2; j++) {
                int32_t buf_s16 = static_cast<int32_t>(static_cast<int16_t>((buf[j*2+1] << 8) | buf[j*2+0]));
                samples[i*2+j] = buf_s16 * vol_table[volume] + DAC_ZERO;  // keep 16bit resolution
                accum[j] += buf_s16 * buf_s16 / 32768;
            }
        }
        rdbuf->shift(buffer->sample_count*4);
        if (rdbuf->getLeft()/4 == 0) { stopFlag = true; }
    } else if (bitsPerSample == 24) {
        if (rdbuf->getLeft()/6 >= buffer->max_sample_count) {
            buffer->sample_count = buffer->max_sample_count;
        } else {
            buffer->sample_count = rdbuf->getLeft()/6;
        }
        int32_t vol_div256 = vol_table[volume] / 256;
        for (int i = 0; i < buffer->sample_count; i++, buf += 6) {
            for (int j = 0; j < 2; j++) {
                int32_t buf_s24 = static_cast<int32_t>((buf[j*3+2] << 24) | (buf[j*3+1] << 16) | (buf[j*3+0] << 8)) / 256;
                if (vol_div256 > 0) {  // keep 24bit resolution
                    samples[i*2+j] = buf_s24 * vol_div256 + DAC_ZERO;
                } else {  // spoil 24bit resolution
                    samples[i*2+j] = buf_s24 * vol_table[volume] / 256 + DAC_ZERO;
                }
                accum[j] += ((buf_s24/256) * (buf_s24/256)) / 32768;
            }
        }
        rdbuf->shift(buffer->sample_count*6);
        if (rdbuf->getLeft()/6 == 0) { stopFlag = true; }
    }
    give_audio_buffer(ap, buffer);
    incSamplesPlayed(buffer->sample_count);
    setLevelInt(accum[0] / buffer->sample_count, accum[1] / buffer->sample_count);
    if (stopFlag) { stop(); }

    #ifdef DEBUG_PLAYWAV
    uint32_t time = static_cast<uint32_t>(to_us_since_boot(get_absolute_time()) - start);
    if (decodeCount++ % 97 == 0) {  // use prime number to avoid sync
        printf("WAV::decode %d us\n", time);
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