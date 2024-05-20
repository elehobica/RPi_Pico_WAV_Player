/*------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include "UserFlash.h"

#include <cstdio>
#include <cstring>

#include "pico/flash.h"

namespace FlashParamNs {
void _user_flash_program_core(void* ptr)
{
    UserFlash* inst = static_cast<UserFlash*>(ptr);
    inst->_program_core();
}

//=================================
// Implementation of UserFlash class
//=================================
UserFlash& UserFlash::instance()
{
    static UserFlash instance; // Singleton
    return instance;
}

UserFlash::UserFlash()
{
    std::copy(flashContents, flashContents + data.size(), data.begin());
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
    flash_range_program(UserFlashOfs, data.data(), FLASH_PAGE_SIZE);
    std::copy(flashContents, flashContents + data.size(), data.begin());
}
}
