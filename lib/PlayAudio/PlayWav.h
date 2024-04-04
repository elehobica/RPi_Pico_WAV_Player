/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#pragma once

#include "PlayAudio.h"

//=================================
// Definition of PlayWav Class
//=================================
class PlayWav : public PlayAudio
{
public:
    static void decode_func();
    PlayWav();
    ~PlayWav();
    void play(const char* filename, size_t fpos = 0, uint32_t samplesPlayed = 0);
    uint32_t totalMillis();
protected:
    static constexpr uint16_t FMT_PCM   = 1;
    static constexpr uint16_t FMT_FLOAT = 3;
    static PlayWav* g_inst;
    uint32_t dataSize;
    uint16_t blockBytes;
    uint16_t format;  // 1: PCM, 3: IEEE float
    uint32_t accum[2] = {};
    uint32_t accumCount;
    void skipToDataChunk();
    bool parseSetPos(size_t fpos);
    void decode();
};
