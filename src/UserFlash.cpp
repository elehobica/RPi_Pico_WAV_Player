/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include <cstdio>
#include <cstring>

#include "UserFlash.h"

//=================================
// Implementation of UserFlash class
//=================================
UserFlash UserFlash::_instance; // Singleton

UserFlash& UserFlash::instance()
{
    return _instance;
}

UserFlash::UserFlash()
{
    memcpy(data, flashContents, sizeof(data));
}

UserFlash::~UserFlash()
{
}

void UserFlash::printInfo()
{
    printf("=== UserFlash ===\n");
    printf("  FlashSize: 0x%x (%d)\n", FlashSize, FlashSize);
    printf("  UserReqSize: 0x%x (%d)\n", UserReqSize, UserReqSize);
    printf("  EraseSize: 0x%x (%d)\n", EraseSize, EraseSize);
    printf("  PagePgrSize: 0x%x (%d)\n", PagePgrSize, PagePgrSize);
    printf("  UserFlashOfs: 0x%x (%d)\n", UserFlashOfs, UserFlashOfs);
    for (int i = 0; i < 16; i++) {
        printf("Flash[%i] = 0x%02x\n", i, data[i]);
    }
}

uint64_t UserFlash::read64(uint32_t flash_ofs)
{
    return *(reinterpret_cast<const uint64_t *>(&flashContents[flash_ofs & 0xfffffff8]));
}

uint32_t UserFlash::read32(uint32_t flash_ofs)
{
    return *(reinterpret_cast<const uint32_t *>(&flashContents[flash_ofs & 0xfffffffc]));
}

uint16_t UserFlash::read16(uint32_t flash_ofs)
{
    return *(reinterpret_cast<const uint16_t *>(&flashContents[flash_ofs & 0xfffffffe]));
}

uint8_t UserFlash::read8(uint32_t flash_ofs)
{
    return flashContents[flash_ofs];
}

void UserFlash::write64Reserve(uint32_t flash_ofs, uint64_t value)
{
    flash_ofs &= 0xfffffff8;
    if (flash_ofs < PagePgrSize) {
        data[flash_ofs + 0] = static_cast<uint8_t>((value >>  0) & 0xff);
        data[flash_ofs + 1] = static_cast<uint8_t>((value >>  8) & 0xff);
        data[flash_ofs + 2] = static_cast<uint8_t>((value >> 16) & 0xff);
        data[flash_ofs + 3] = static_cast<uint8_t>((value >> 24) & 0xff);
        data[flash_ofs + 4] = static_cast<uint8_t>((value >> 32) & 0xff);
        data[flash_ofs + 5] = static_cast<uint8_t>((value >> 40) & 0xff);
        data[flash_ofs + 6] = static_cast<uint8_t>((value >> 48) & 0xff);
        data[flash_ofs + 7] = static_cast<uint8_t>((value >> 56) & 0xff);
    }
}

void UserFlash::write32Reserve(uint32_t flash_ofs, uint32_t value)
{
    flash_ofs &= 0xfffffffc;
    if (flash_ofs < PagePgrSize) {
        data[flash_ofs + 0] = static_cast<uint8_t>((value >>  0) & 0xff);
        data[flash_ofs + 1] = static_cast<uint8_t>((value >>  8) & 0xff);
        data[flash_ofs + 2] = static_cast<uint8_t>((value >> 16) & 0xff);
        data[flash_ofs + 3] = static_cast<uint8_t>((value >> 24) & 0xff);
    }
}

void UserFlash::write16Reserve(uint32_t flash_ofs, uint16_t value)
{
    flash_ofs &= 0xfffffffe;
    if (flash_ofs < PagePgrSize) {
        data[flash_ofs + 0] = static_cast<uint8_t>((value >>  0) & 0xff);
        data[flash_ofs + 1] = static_cast<uint8_t>((value >>  8) & 0xff);
    }
}

void UserFlash::write8Reserve(uint32_t flash_ofs, uint8_t value)
{
    if (flash_ofs < PagePgrSize) {
        data[flash_ofs] = value;
    }
}

void UserFlash::program()
{
    // Need to stop interrupt during erase and program
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(UserFlashOfs, EraseSize);
    flash_range_program(UserFlashOfs, data, FLASH_PAGE_SIZE);
    restore_interrupts(ints);
    memcpy(data, flashContents, sizeof(data));
}
