/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include "ReadBuffer.h"

#include <cstdlib>
#include <cstring>
#include "pico/multicore.h"

ReadBuffer* ReadBuffer::_inst = nullptr;

void readBufferCore1Process()
{
    ReadBuffer::_inst->fillLoop();
}

ReadBuffer* ReadBuffer::getInstance()
{
    if (_inst == nullptr) {
        // create Singleton instance
        _inst = new ReadBuffer();
        // start process on core1
        multicore_reset_core1();
        multicore_launch_core1(readBufferCore1Process);
    }
    return _inst;
}

// fillThreshold: auto fill if left is lower than fillThreshold
//                set fillThreshold = 0 if using manual fill instead of auto fill
//                set fillThreshold = size if auto fill everytime when shift (not recommended due to too many memmove)
ReadBuffer::ReadBuffer() :
    _size(PlayAudio::RDBUF_SIZE), _left(0), _fillThreshold(PlayAudio::RDBUF_THRESHOLD), _isEof(false)
{
    _head = reinterpret_cast<uint8_t*>(calloc(_size, sizeof(uint8_t)));
    _ptr = _head;
}

ReadBuffer::~ReadBuffer()
{
    free(_head);
}

const uint8_t* ReadBuffer::buf()
{
    return reinterpret_cast<const uint8_t*>(_ptr);
}

void ReadBuffer::bind(FIL* fp)
{
    _fp = fp;
    _ptr = _head;
    _left = 0;
}

bool ReadBuffer::fill()
{
    if (_isEof) { return false; }
    while (queue_is_empty(&fillBufferQueue)) { sleep_ms(1); }
    fillBufferItem_t item;
    queue_remove_blocking(&fillBufferQueue, &item);
    memmove(_head, _ptr, _left);
    _ptr = _head;
    size_t space = _size - _left;
    memcpy(_head + _left, &fillBuffer[FILL_BUFFER_SIZE * item.id], item.length);
    _left += item.length;
    _isEof = item.isEof;
    if (space > item.length) {
        memset(_head + _left, 0, space - item.length);  // fill 0
    }
    return true;
}

bool ReadBuffer::shift(size_t bytes)
{
    if (_left < bytes) { return false; }
    _ptr += bytes;
    _left -= bytes;
    if (_left < _fillThreshold) { fill(); }
    return true;
}

bool ReadBuffer::shiftAll()
{
    return shift(_left);
}

bool ReadBuffer::seek(size_t fpos)
{
    reqBind(_fp, false);  // disconnect fillBuffer (dispose current fillBuffer)
    f_lseek(_fp, fpos);   // seek (move reading point)
    reqBind(_fp);         // reconnect
    // fill fillBuffer again
    _ptr = _head;
    _left = 0;
    fill();
    return true;
}

size_t ReadBuffer::getLeft()
{
    return _left;
}

size_t ReadBuffer::tell()
{
    return (size_t) f_tell(_fp) - _left;
}

void ReadBuffer::reqBind(FIL* fp, bool flag)
{
    bindReq_t req;
    req.fp = fp;
    req.flag = flag;
    // send request
    queue_try_add(&bindReqQueue, &req);
    // wait response
    queue_remove_blocking(&bindRespQueue, &req);
}

void ReadBuffer::fillLoop()
{
    int id = 0;
    queue_init(&bindReqQueue, sizeof(bindReq_t), 1);
    queue_init(&bindRespQueue, sizeof(bindReq_t), 1);
    queue_init(&fillBufferQueue, sizeof(fillBufferItem_t), NUM_FILL_BUFFERS);

    while (true) {
        while (queue_is_empty(&bindReqQueue)) { sleep_ms(1); }
        bindReq_t req;
        queue_remove_blocking(&bindReqQueue, &req);
        if (!req.flag) continue;
        queue_try_add(&bindRespQueue, &req);
        FIL* fp = req.fp;
        bind(fp);
        _isEof = false;
        while (!f_eof(fp)) {
            while (queue_get_level(&fillBufferQueue) < NUM_FILL_BUFFERS) {
                FRESULT fr;
                UINT br;
                fr = f_read(fp, &fillBuffer[FILL_BUFFER_SIZE * id], FILL_BUFFER_SIZE, &br);
                if (fr != FR_OK || br == 0) { return; }
                fillBufferItem_t item;
                item.id = id;
                item.length = static_cast<size_t>(br);
                item.isEof = static_cast<bool>(f_eof(fp));
                queue_try_add(&fillBufferQueue, &item);
                id = (id + 1) % NUM_FILL_BUFFERS;
            }
            if (!queue_is_empty(&bindReqQueue)) {
                bindReq_t req;
                queue_remove_blocking(&bindReqQueue, &req);
                if (!req.flag) {
                    while (!queue_is_empty(&fillBufferQueue)) {
                        fillBufferItem_t item;
                        queue_remove_blocking(&fillBufferQueue, &item);
                    }
                    queue_try_add(&bindRespQueue, &req);
                    break;
                }
            }
        }
    }
}
