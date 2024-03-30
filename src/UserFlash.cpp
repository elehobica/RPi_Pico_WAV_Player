/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include "UserFlash.h"

#include <cstdio>
#include <cstring>
#include "pico/flash.h"

void _user_flash_program_core(void* ptr)
{
    UserFlash* inst = static_cast<UserFlash*>(ptr);
    inst->_program_core();
}

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
}

void UserFlash::read(uint32_t flash_ofs, size_t size, void *buf)
{
    if (flash_ofs + size <=  PagePgrSize) {
        memcpy(buf, &flashContents[flash_ofs], size);
    }
}

void UserFlash::writeReserve(uint32_t flash_ofs, size_t size, const void *buf)
{
    if (flash_ofs + size <=  PagePgrSize) {
        memcpy(&data[flash_ofs], buf, size);
    }
}

bool UserFlash::program()
{
    // Need to stop interrupt during erase and program
    // noted that if core1 is running, it must be stopped also if accessing flash
    int result = flash_safe_execute(_user_flash_program_core, this, 100);
    if (result != PICO_OK) {
        return false;
    }
    return true;
}

void UserFlash::_program_core()
{
    flash_range_erase(UserFlashOfs, EraseSize);
    flash_range_program(UserFlashOfs, data, FLASH_PAGE_SIZE);
    memcpy(data, flashContents, sizeof(data));
}