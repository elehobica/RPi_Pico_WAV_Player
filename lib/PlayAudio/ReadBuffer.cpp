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
    size(size), left(0), fillThreshold(fillThreshold)
{
    head = reinterpret_cast<uint8_t*>(calloc(size, sizeof(uint8_t)));
    ptr = head;
}

ReadBuffer::ReadBuffer(FIL *fp, size_t size, size_t fillThreshold) :
    fp(fp), size(size), left(0), fillThreshold(fillThreshold)
{
    head = reinterpret_cast<uint8_t*>(calloc(size, sizeof(uint8_t)));
    ptr = head;
    if (left < fillThreshold) { fill(); }
}

ReadBuffer::~ReadBuffer()
{
    free(head);
}

const uint8_t* ReadBuffer::buf()
{
    return reinterpret_cast<const uint8_t*>(ptr);
}

void ReadBuffer::bind(FIL* fp)
{
    this->fp = fp;
    ptr = head;
    left = 0;
    if (left < fillThreshold) { fill(); }
}

bool ReadBuffer::fill()
{
    memmove(head, ptr, left);
    ptr = head;
    size_t space = size - left;

    if (f_eof(fp) || space == 0) { return false; }

    FRESULT fr;
    UINT br;
    fr = f_read(fp, head + left, space, &br);
    if (fr != FR_OK || br == 0) { return false; }
    left += br;
    if (space > br) { memset(head + left, 0, space - br); } // fill 0
    return true;
}

bool ReadBuffer::shift(size_t bytes)
{
    if (left < bytes) { return false; }
    ptr += bytes;
    left -= bytes;
    if (left < fillThreshold) { fill(); }
    return true;
}

bool ReadBuffer::shiftAll()
{
    return shift(left);
}

bool ReadBuffer::seek(size_t fpos)
{
    ptr = head;
    f_lseek(fp, fpos);
    if (left < fillThreshold) { fill(); }
    return true;
}

size_t ReadBuffer::getLeft()
{
    return left;
}

size_t ReadBuffer::tell()
{
    return (size_t) f_tell(fp) - left;
}
