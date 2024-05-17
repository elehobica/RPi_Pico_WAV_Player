/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#pragma once

#include <array>

#include "hardware/flash.h"

//=================================
// Interface of UserFlash class
//=================================

//#define userFlash UserFlash::instance()

class UserFlash
{
public:
    static UserFlash& instance(); // Singleton
    void printInfo();
    void read(const uint32_t& flash_ofs, const size_t& size, void* buf);
    void writeReserve(const uint32_t& flash_ofs, const size_t& size, const void* buf);
    bool program();

protected:
    static constexpr size_t FlashSize = 0x200000; // 2MB
    static constexpr size_t UserReqSize = 1024; // Byte
    static constexpr size_t EraseSize = ((UserReqSize + (FLASH_SECTOR_SIZE - 1)) / FLASH_SECTOR_SIZE) * FLASH_SECTOR_SIZE;
    static constexpr size_t PagePgrSize = ((UserReqSize + (FLASH_PAGE_SIZE - 1)) / FLASH_PAGE_SIZE) * FLASH_PAGE_SIZE;
    static constexpr uint32_t UserFlashOfs = FlashSize - EraseSize;
    const uint8_t* flashContents = reinterpret_cast<const uint8_t*>(XIP_BASE + UserFlashOfs);
    std::array<uint8_t, PagePgrSize> data;
    // Singleton
    UserFlash();
    virtual ~UserFlash();
    UserFlash(const UserFlash&) = delete;
    UserFlash& operator=(const UserFlash&) = delete;
    void _program_core();

    friend void _user_flash_program_core(void*);
};
