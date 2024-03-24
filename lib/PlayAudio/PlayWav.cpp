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
                format        = static_cast<uint16_t>(       getU16LE(buf + ofs + 4 + 4)); // format
                channels      = static_cast<uint16_t>(       getU16LE(buf + ofs + 4 + 4 + 2)); // channels
                sf            = static_cast<i2s_samp_freq_t>(getU32LE(buf + ofs + 4 + 4 + 2 + 2)); // samplerate
                bitRateKbps   = static_cast<uint16_t>(       getU32LE(buf + ofs + 4 + 4 + 2 + 2 + 4) /* bytepersec */ * 8 / 1000); // Kbps
                blockBytes    = static_cast<uint16_t>(       getU16LE(buf + ofs + 4 + 4 + 2 + 2 + 4 + 4)); // blockBytes
                bitsPerSample = static_cast<uint16_t>(       getU16LE(buf + ofs + 4 + 4 + 2 + 2 + 4 + 4 + 2)); // bitswidth
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

    if (isMuteCondition()) {
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
    const uint8_t* buf = rdbuf->buf();
    uint32_t accum[2] = {};
    buffer->sample_count = std::min(static_cast<uint32_t>(rdbuf->getLeft()/blockBytes), buffer->max_sample_count);
    for (int i = 0; i < buffer->sample_count; i++, buf += blockBytes) {
        for (int j = 0; j < 2; j++) {
            int base = (channels == 2) ? j * bitsPerSample / 8 : 0;
            int32_t buf_s32;
            switch ((format << 8) | bitsPerSample) {
                case ((FMT_PCM   << 8) | 16): buf_s32 = static_cast<int32_t>((buf[base+1] << 24) | (buf[base+0] << 16)); break;
                case ((FMT_PCM   << 8) | 24): buf_s32 = static_cast<int32_t>((buf[base+2] << 24) | (buf[base+1] << 16) | (buf[base+0] << 8)); break;
                case ((FMT_PCM   << 8) | 32): buf_s32 = static_cast<int32_t>((buf[base+3] << 24) | (buf[base+2] << 16) | (buf[base+1] << 8) | (buf[base+0] << 0)); break;
                case ((FMT_FLOAT << 8) | 32): buf_s32 = 0; break;
                default: buf_s32 = 0; break;
            }
            samples[i*2+j] = static_cast<int32_t>((static_cast<int64_t>(buf_s32) * vol_table[volume] / 65536)) + DAC_ZERO;
            accum[j] += (buf_s32/65536) * (buf_s32/65536) / 32768;
        }
    }
    give_audio_buffer(ap, buffer);
    incSamplesPlayed(buffer->sample_count);
    setLevelInt(accum[0] / buffer->sample_count, accum[1] / buffer->sample_count);
    rdbuf->shift(buffer->sample_count*blockBytes);
    if (rdbuf->getLeft()/channels/blockBytes == 0) { stop(); }

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