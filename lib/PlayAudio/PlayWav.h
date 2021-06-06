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
    void play(const char *filename, size_t fpos = 0, uint32_t samplesPlayed = 0);
    uint32_t totalMillis();
protected:
    static PlayWav *g_inst;
    uint32_t dataSize;
    void skipToDataChunk();
    void decode();
};

#endif // __PLAY_WAV_H_INCLUDED__