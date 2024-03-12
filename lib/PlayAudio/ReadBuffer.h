/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#ifndef __READ_BUFFER_H_INCLUDED__
#define __READ_BUFFER_H_INCLUDED__

#include "ff.h"

//=================================
// Interface of ReadBuffer Class
//=================================
class ReadBuffer
{
public:
    ReadBuffer(size_t size, size_t fillThreshold);
    ReadBuffer(FIL* fp, size_t size, size_t fillThreshold);
    ~ReadBuffer();
    const uint8_t* buf();
    void bind(FIL* fp);
    bool fill();
    bool shift(size_t bytes);
    bool shiftAll();
    bool seek(size_t fpos);
    size_t getLeft();
    size_t tell();
private:
    FIL* _fp;
    size_t _size;
    size_t _left;
    uint8_t* _head;
    uint8_t* _ptr;
    size_t _fillThreshold;
};

#endif // __READ_BUFFER_H_INCLUDED__