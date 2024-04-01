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
    void setBufPos(size_t fpos);
    void decode();
};
