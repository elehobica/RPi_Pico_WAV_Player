/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include <cstdlib>
#include <cstring>
#include "ReadBuffer.h"

// fillThreshold: auto fill if left is lower than fillThreshold
//                set fillThreshold = 0 if using manual fill instead of auto fill
//                set fillThreshold = size if auto fill everytime when shift (not recommended due to too many memmove)
ReadBuffer::ReadBuffer(size_t size, size_t fillThreshold) :
    _size(size), _left(0), _fillThreshold(fillThreshold)
{
    _head = reinterpret_cast<uint8_t*>(calloc(_size, sizeof(uint8_t)));
    _ptr = _head;
}

ReadBuffer::ReadBuffer(FIL *fp, size_t size, size_t fillThreshold) :
    _fp(fp), _size(size), _left(0), _fillThreshold(fillThreshold)
{
    _head = reinterpret_cast<uint8_t*>(calloc(_size, sizeof(uint8_t)));
    _ptr = _head;
    if (_left < _fillThreshold) { fill(); }
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
    if (_left < _fillThreshold) { fill(); }
}

bool ReadBuffer::fill()
{
    memmove(_head, _ptr, _left);
    _ptr = _head;
    size_t space = _size - _left;

    if (f_eof(_fp) || space == 0) { return false; }

    FRESULT fr;
    UINT br;
    fr = f_read(_fp, _head + _left, space, &br);
    if (fr != FR_OK || br == 0) { return false; }
    _left += br;
    if (space > br) { memset(_head + _left, 0, space - br); } // fill 0
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
    _ptr = _head;
    f_lseek(_fp, fpos);
    if (_left < _fillThreshold) { fill(); }
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
