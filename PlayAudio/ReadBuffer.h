/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#ifndef __READ_BUFFER_H_INCLUDED__
#define __READ_BUFFER_H_INCLUDED__

#include "fatfs/ff.h"

//=================================
// Interface of ReadBuffer Class
//=================================
class ReadBuffer
{
public:
    ReadBuffer(size_t size, bool autoFill = false);
    ReadBuffer(FIL *fp, size_t size, bool autoFill = false);
    ~ReadBuffer();
    const uint8_t *buf();
    void bind(FIL *fp);
    bool fill();
    bool shift(size_t bytes);
    bool shiftAndFill(size_t bytes);
    size_t getLeft();
private:
    FIL *fp;
    size_t size;
    size_t left;
    uint8_t *head;
    uint8_t *ptr;
    bool autoFill;
};

#endif // __READ_BUFFER_H_INCLUDED__