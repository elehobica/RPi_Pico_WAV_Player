/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#pragma once

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
