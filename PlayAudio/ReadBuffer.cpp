/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include <cstdlib>
#include <cstring>
#include "ReadBuffer.h"

ReadBuffer::ReadBuffer(size_t size, bool autoFill) : size(size), left(0), autoFill(autoFill)
{
    head = (uint8_t *) calloc(size, sizeof(uint8_t));
    ptr = head;
}

ReadBuffer::ReadBuffer(FIL *fp, size_t size, bool autoFill) : fp(fp), size(size), left(0), autoFill(autoFill)
{
    head = (uint8_t *) calloc(size, sizeof(uint8_t));
    ptr = head;
}

ReadBuffer::~ReadBuffer()
{
    free(head);
}

const uint8_t *const ReadBuffer::buf()
{
    return (const uint8_t *const) ptr;
}

void ReadBuffer::reCouple(FIL *fp)
{
    this->fp = fp;
    ptr = head;
    left = 0;
    if (autoFill) { fill(); }
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
    if (autoFill) { fill(); }
    return true;
}

size_t ReadBuffer::getLeft()
{
    return left;
}