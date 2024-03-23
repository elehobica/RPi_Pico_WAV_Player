/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#ifndef __PLAY_WAV_H_INCLUDED__
#define __PLAY_WAV_H_INCLUDED__

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
    static PlayWav* g_inst;
    uint32_t dataSize;
    uint16_t blockBytes;
    uint16_t format;  // 1: PCM, 3: IEEE float
    void skipToDataChunk();
    void setBufPos(size_t fpos);
    void decode();
};

#endif // __PLAY_WAV_H_INCLUDED__