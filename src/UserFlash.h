/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#ifndef _USER_FLASH_H_
#define _USER_FLASH_H_

extern "C" {
#include "hardware/flash.h"
#include "hardware/sync.h"
}

//=================================
// Interface of UserFlash class
//=================================

#define userFlash UserFlash::instance()

class UserFlash
{
public:
    static UserFlash& instance(); // Singleton
    void printInfo();
    uint64_t read64(uint32_t flash_ofs);
    uint32_t read32(uint32_t flash_ofs);
    uint16_t read16(uint32_t flash_ofs);
    uint8_t read8(uint32_t flash_ofs);
    void write64Reserve(uint32_t flash_ofs, uint64_t value);
    void write32Reserve(uint32_t flash_ofs, uint32_t value);
    void write16Reserve(uint32_t flash_ofs, uint16_t value);
    void write8Reserve(uint32_t flash_ofs, uint8_t value);
    void program();
protected:
    static const size_t FlashSize = 0x200000; // 2MB
    static const size_t UserReqSize = 1024; // Byte
    static const size_t EraseSize = ((UserReqSize + (FLASH_SECTOR_SIZE - 1)) / FLASH_SECTOR_SIZE) * FLASH_SECTOR_SIZE;
    static const size_t PagePgrSize = ((UserReqSize + (FLASH_PAGE_SIZE - 1)) / FLASH_PAGE_SIZE) * FLASH_PAGE_SIZE;
    static const uint32_t UserFlashOfs = FlashSize - EraseSize;
    const uint8_t *flashContents = (const uint8_t *) (XIP_BASE + UserFlashOfs);
	static UserFlash _instance; // Singleton
    uint8_t data[PagePgrSize];
	// Singleton
    UserFlash();
    virtual ~UserFlash();
    UserFlash(const UserFlash&) = delete;
	UserFlash& operator=(const UserFlash&) = delete;
};

#endif // _USER_FLASH_H_
