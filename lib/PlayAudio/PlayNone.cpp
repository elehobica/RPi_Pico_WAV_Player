/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include "PlayNone.h"

PlayNone* PlayNone::g_inst = nullptr;

void PlayNone::decode_func()
{
    if (g_inst == nullptr) { return; }
    g_inst->decode();
}

PlayNone::PlayNone() : PlayAudio()
{
    g_inst = this;
}

PlayNone::~PlayNone()
{
}