/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#ifndef __PLAY_NONE_H_INCLUDED__
#define __PLAY_NONE_H_INCLUDED__

#include "PlayAudio.h"

//=================================
// Definition of PlayNone Class
//=================================
class PlayNone : public PlayAudio
{
public:
    static void decode_func();
    PlayNone();
    ~PlayNone();
    uint32_t elapsedMillis() { return 0U; }
    uint32_t totalMillis() { return 1U; }
protected:
    static PlayNone *g_inst;
};

#endif // __PLAY_NONE_H_INCLUDED__