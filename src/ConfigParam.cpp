/*-----------------------------------------------------------/
/ ConfigParam.h
/------------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/-----------------------------------------------------------*/

#include "ConfigParam.h"
#include "UserFlash.h"

ConfigParam& ConfigParam::instance()
{
    static ConfigParam _instance; // Singleton
    return _instance;
}

ConfigParam::ConfigParam()
{
    loadDefault();
}

void ConfigParam::printInfo() const
{
    const auto& fmt = this->printFormat;
    printf("=== ConfigParam ===\n");
    for (const auto& [key, param] : configParamItems) {
        std::visit([&fmt, &param](auto&& arg) {
            // visit() is needed to covert variant type into original type with arg
            printf(fmt.at(param.value.index()).c_str(), param.flashAddr, param.name, arg, arg);
        }, param.value);
    }
}

void ConfigParam::loadDefault()
{
    for (auto& [key, param] : configParamItems) {
        param.loadDefault();
    }
}

uint32_t ConfigParam::getBootCountFromFlash()
{
    const auto& param = configParamItems.at(CFG_BOOT_COUNT);
    uint32_t bootCount;
    userFlash.read(param.flashAddr, param.size, &bootCount);
    return bootCount;
}

void ConfigParam::incBootCount()
{
    auto bootCount = getValue<uint32_t>(CFG_BOOT_COUNT);
    bootCount++;
    setValue<uint32_t>(CFG_BOOT_COUNT, bootCount);
}

void ConfigParam::initialize()
{
    // load default
    loadDefault();
    // switch to step to load from Flash
    if (getBootCountFromFlash() == 0xffffffffUL) { return; }

    uint32_t formatRev = GET_CFG_FORMAT_REV;
    // load from Flash
    for (auto& [key, param] : configParamItems) {
        std::visit([&param](auto&& arg) mutable {
            userFlash.read(param.flashAddr, param.size, &arg);
        }, param.value);
    }
    uint32_t formatRevStored = GET_CFG_FORMAT_REV;

    // Force to reset to default due to format revision changed (parameter/address) to avoid mulfunction
    if (formatRev != formatRevStored) {
        loadDefault();
    }
}

void ConfigParam::finalize()
{
    // store to Flash
    for (const auto& [key, param] : configParamItems) {
        std::visit([&param](auto&& arg) {
            userFlash.writeReserve(param.flashAddr, param.size, &arg);
        }, param.value);
    }
    userFlash.program();
}
