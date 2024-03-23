/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include "ReadBuffer.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "pico/multicore.h"

ReadBuffer* ReadBuffer::_inst = nullptr;

void readBufferCore1Process()
{
    ReadBuffer::_inst->fillLoop();
    printf("ERROR: ReadBuffer::fillLoop() exit\r\n");
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
    _isEof = false;
}

bool ReadBuffer::fill()
{
    if (_isEof) { return false; }
    if (queue_is_empty(&secondaryBufferQueue)) {
        // insert zeros to avoid blank noise
        memmove(_head, _ptr, _left);
        _ptr = _head;
        size_t space = _size - _left;
        memset(_head + _left, 0, space);
        _left += space;
        printf("ERROR: ReadBuffer::secondaryBuffer is empty\r\n");
        return false;
    }
    secondaryBufferItem_t item;
    queue_remove_blocking(&secondaryBufferQueue, &item);
    memmove(_head, _ptr, _left);
    _ptr = _head;
    size_t space = _size - _left;
    memcpy(_head + _left, item.ptr, item.length);
    _pos = item.pos;
    _left += item.length;
    _isEof = item.reachedEof;
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
    reqBind(_fp, false);  // disconnect secondaryBuffer (dispose current secondaryBuffer)
    f_lseek(_fp, fpos);   // seek (move reading point)
    reqBind(_fp);         // reconnect
    return true;
}

size_t ReadBuffer::getLeft()
{
    return _left;
}

size_t ReadBuffer::tell()
{
    return _pos - _left;
}

bool ReadBuffer::isFull()
{
    return queue_is_full(&secondaryBufferQueue);
}

bool ReadBuffer::isNearEmpty()
{
    return (!static_cast<bool>(f_eof(_fp)) && queue_get_level(&secondaryBufferQueue) <= NUM_SECONDARY_BUFFERS / 4);
}

void ReadBuffer::reqBind(FIL* fp, bool flag)
{
    bindReq_t req = {fp, flag};
    // send request
    queue_try_add(&bindReqQueue, &req);
    // wait response
    queue_remove_blocking(&bindRespQueue, &req);
    if (flag) {
        while (!queue_is_full(&secondaryBufferQueue)) {}
        fill();
    }
}

void ReadBuffer::fillLoop()
{
    int id = 0;
    FIL* fp;
    queue_init(&bindReqQueue, sizeof(bindReq_t), 1);
    queue_init(&bindRespQueue, sizeof(bindReq_t), 1);
    queue_init(&secondaryBufferQueue, sizeof(secondaryBufferItem_t), NUM_SECONDARY_BUFFERS);
    bindReq_t req;
    secondaryBufferItem_t item;

    while (true) {
        // expecting reqBind(true)
        while (queue_is_empty(&bindReqQueue)) {}
        queue_remove_blocking(&bindReqQueue, &req);
        if (req.flag) {
            fp = req.fp;
            bind(fp);
            item.pos = f_tell(fp);
            item.reachedEof = static_cast<bool>(f_eof(fp));
        }
        queue_try_add(&bindRespQueue, &req);  // response regardless of flag
        if (!req.flag) { continue; }  // retry if reqBind(false)

        while (!item.reachedEof) {
            // read from file to store secondaryBuffer
            while (!queue_is_full(&secondaryBufferQueue)) {
                // read at once for max efficiency as min of either till the end of buffer or spare number of queue
                int level = static_cast<int>(queue_get_level(&secondaryBufferQueue));
                int reqN = NUM_SECONDARY_BUFFERS - ((id > level) ? id : level);
                UINT br;
                FRESULT fr = f_read(fp, &secondaryBuffer[SECONDARY_BUFFER_SIZE * id], SECONDARY_BUFFER_SIZE * reqN, &br);
                if (fr != FR_OK || br == 0) { return; }
                // put on queue divided by SECONDARY_BUFFER_SIZE
                int readN = (br + SECONDARY_BUFFER_SIZE - 1) / SECONDARY_BUFFER_SIZE;
                for (int i = 0; i < readN; i++) {
                    if (i < readN - 1) {
                        item.length = SECONDARY_BUFFER_SIZE;
                        item.reachedEof = false;
                    } else {
                        item.length = br - SECONDARY_BUFFER_SIZE * i;
                        item.reachedEof = static_cast<bool>(f_eof(fp));
                    }
                    item.ptr = &secondaryBuffer[SECONDARY_BUFFER_SIZE * id];
                    item.pos += item.length;
                    queue_try_add(&secondaryBufferQueue, &item);
                    id = (id + 1) % NUM_SECONDARY_BUFFERS;
                }
                if (item.reachedEof) { break; }
            }
            // acceptance of reqBind(false)
            if (!queue_is_empty(&bindReqQueue)) {
                queue_remove_blocking(&bindReqQueue, &req);
                if (!req.flag) {
                    // discard all data in senondaryBufferQueue
                    while (!queue_is_empty(&secondaryBufferQueue)) {
                        queue_remove_blocking(&secondaryBufferQueue, &item);
                    }
                }
                queue_try_add(&bindRespQueue, &req);  // response regardless of flag
                if (!req.flag) { break; }  // start over if reqBind(false), otherwise ignore
            }
        }
    }
}
