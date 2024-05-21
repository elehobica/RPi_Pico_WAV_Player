/*-----------------------------------------------------------/
/ ConfigParam.h
/------------------------------------------------------------/
/ Copyright (c) 2024, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/-----------------------------------------------------------*/

#include "ConfigParam.h"

//=================================
// Implementation of ConfigParam class
//=================================
ConfigParam& ConfigParam::instance()  // Singleton
{
    static ConfigParam instance;
    return instance;
}

uint32_t ConfigParam::getBootCountFromFlash()
{
    auto& param = P_CFG_BOOT_COUNT;
    FlashParamNs::ReadFromFlashVisitor visitor;
    visitor(&param);
    return param.get();
}

void ConfigParam::incBootCount()
{
    auto& param = P_CFG_BOOT_COUNT;
    param.set(param.get() + 1);
}

void ConfigParam::initialize()
{
    loadDefault();
    // don't load from Flash if flash is blank
    if (getBootCountFromFlash() == 0xffffffffUL) { return; }
    // load from flash and get format revision
    uint32_t formatRevExpected = P_CFG_FORMAT_REV.get();
    loadFromFlash();
    uint32_t formatRev = P_CFG_FORMAT_REV.get();
    // Force to reset to default due to format revision changed (parameter/address) to avoid mulfunction
    if (formatRevExpected != formatRev) { loadDefault(); }
}
