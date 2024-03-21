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
    bool fill();
    void reqBind(FIL* fp, bool flag = true);
    const uint8_t* buf();
    bool shift(size_t bytes);
    bool shiftAll();
    bool seek(size_t fpos);
    size_t getLeft();
    size_t tell();
private:
    static constexpr size_t NUM_FILL_BUFFERS = 16;
    static constexpr size_t FILL_BUFFER_SIZE = PlayAudio::RDBUF_SIZE - PlayAudio::RDBUF_THRESHOLD;
    static ReadBuffer* _inst;  // Singleton instance
    uint8_t fillBuffer[FILL_BUFFER_SIZE * NUM_FILL_BUFFERS];
    typedef struct _fillBufferItem_t {
        int    id;
        size_t length;
        bool   isEof;
    } fillBufferItem_t;
    typedef struct _bindReq_t {
        FIL* fp;
        bool flag;
    } bindReq_t;
    queue_t fillBufferQueue;
    queue_t bindReqQueue;
    queue_t bindRespQueue;
    FIL* _fp;
    size_t _size;
    size_t _left;
    uint8_t* _head;
    uint8_t* _ptr;
    size_t _fillThreshold;
    bool _isEof;
    void bind(FIL* fp);
    void fillLoop();
    friend void readBufferCore1Process();
};
