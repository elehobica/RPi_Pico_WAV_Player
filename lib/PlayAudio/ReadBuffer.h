/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#pragma once

#include "pico/util/queue.h"
#include "PlayAudio.h"
#include "ff.h"

//=================================
// Interface of ReadBuffer Class
//=================================
class ReadBuffer
{
public:
    static ReadBuffer* getInstance();  // Singleton
    ReadBuffer();
    virtual ~ReadBuffer();
    void reqBind(FIL* fp, bool flag = true);
    const uint8_t* buf();
    bool shift(size_t bytes);
    bool shiftAll();
    bool seek(size_t fpos);
    size_t getLeft();
    size_t tell();
private:
    static constexpr size_t SECONDARY_BUFFER_SIZE = PlayAudio::RDBUF_SIZE - PlayAudio::RDBUF_THRESHOLD;
    static constexpr size_t NUM_SECONDARY_BUFFERS = 16;
    static ReadBuffer* _inst;  // Singleton instance
    uint8_t secondaryBuffer[SECONDARY_BUFFER_SIZE * NUM_SECONDARY_BUFFERS];
    typedef struct _secondaryBufferItem_t {
        uint8_t* ptr;
        size_t   pos;
        size_t   length;
        bool     reachedEof;
    } secondaryBufferItem_t;
    typedef struct _bindReq_t {
        FIL* fp;
        bool flag;
    } bindReq_t;
    queue_t secondaryBufferQueue;
    queue_t bindReqQueue;
    queue_t bindRespQueue;
    FIL* _fp;
    size_t _size;
    size_t _pos;
    size_t _left;
    uint8_t* _head;
    uint8_t* _ptr;
    size_t _fillThreshold;
    bool _isEof;
    void bind(FIL* fp);
    bool fill();
    void fillLoop();
    friend void readBufferCore1Process();
};
