/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#pragma once

#include <array>

#include "hardware/flash.h"

namespace FlashParamNs {
//=================================
// Interface of UserFlash class
//=================================
class UserFlash
{
public:
    static UserFlash& instance(); // Singleton
    void printInfo();
    template <typename T>
    void read(const uint32_t& flash_ofs, const size_t& size, T& value) {
        if (flash_ofs + size <= PagePgrSize) {
            uint8_t* ptr = reinterpret_cast<uint8_t*>(&value);
            std::copy(flashContents + flash_ofs, flashContents + flash_ofs + size, ptr);
        }
    }

    template <typename T>
    void writeReserve(const uint32_t& flash_ofs, const size_t& size, const T& value) {
        if (flash_ofs + size <= PagePgrSize) {
            const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&value);
            std::copy(ptr, ptr + size, std::next(data.begin(), flash_ofs));
        }
    }
    bool program();

protected:
    static constexpr size_t FlashSize = 0x200000; // 2MB
    static constexpr size_t UserReqSize = 1024; // Byte
    static constexpr size_t EraseSize = ((UserReqSize + (FLASH_SECTOR_SIZE - 1)) / FLASH_SECTOR_SIZE) * FLASH_SECTOR_SIZE;
    static constexpr size_t PagePgrSize = ((UserReqSize + (FLASH_PAGE_SIZE - 1)) / FLASH_PAGE_SIZE) * FLASH_PAGE_SIZE;
    static constexpr uint32_t UserFlashOfs = FlashSize - EraseSize;
    UserFlash();
    virtual ~UserFlash();
    UserFlash(const UserFlash&) = delete;
    UserFlash& operator=(const UserFlash&) = delete;
    void _program_core();
    const uint8_t* flashContents = reinterpret_cast<const uint8_t*>(XIP_BASE + UserFlashOfs);
    std::array<uint8_t, PagePgrSize> data;

    friend void _user_flash_program_core(void*);
    friend class FlashParam;
};
}
