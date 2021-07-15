/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#ifndef _USER_FLASH_H_
#define _USER_FLASH_H_

#include "hardware/flash.h"
#include "hardware/sync.h"

//=================================
// Interface of UserFlash class
//=================================

#define userFlash UserFlash::instance()

class UserFlash
{
public:
    static UserFlash& instance(); // Singleton
    void printInfo();
    void read(uint32_t flash_ofs, size_t size, void *buf);
    void writeReserve(uint32_t flash_ofs, size_t size, const void *buf);
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
