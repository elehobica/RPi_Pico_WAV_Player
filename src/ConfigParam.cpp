/*-----------------------------------------------------------/
/ ConfigParam.h
/------------------------------------------------------------/
/ Copyright (c) 2024, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/-----------------------------------------------------------*/

#include "ConfigParam.h"

namespace ConfigParamNs {

//=================================
// Implementation of Parameter class
//=================================
template <class T>
Parameter<T>::Parameter(const ParamId_t& id, const char* name, const uint32_t& flashAddr, const valueType& defaultValue, size_t& size)
    : id(id), name(name), flashAddr(flashAddr), defaultValue(defaultValue), size(size)
{
    Params::instance().add(id, this);
}

template <class T>
Parameter<T>::Parameter(const ParamId_t& id, const char* name, const uint32_t& flashAddr, const valueType& defaultValue)
    : id(id), name(name), flashAddr(flashAddr), defaultValue(defaultValue), size(sizeof(T))
{
    Params::instance().add(id, this);
}

//=================================
// Implementation of Params class
//=================================
Params& Params::instance()
{
    static Params _instance; // Singleton
    return _instance;
}

void Params::printInfo() const
{
    for (const auto& [key, item] : paramMap) {
        const auto& format = printFormat.at(item.index());
        std::visit([&format](auto&& param) {
            printf(format.c_str(), param->flashAddr, param->name, param->value, param->value);
        }, item);
    }
}

void Params::loadDefault()
{
    for (auto& [key, item] : paramMap) {
        std::visit([](auto&& param) {
            param->loadDefault();
        }, item);
    }
}

void Params::loadFromFlash()
{
    for (auto& [key, item] : paramMap) {
        std::visit([](auto&& param) mutable {
            param->readFromFlash();
        }, item);
    }
}

void Params::storeToFlash() const
{
    for (const auto& [key, item] : Params::instance().paramMap) {
        std::visit([](auto&& param) {
            param->writeReserve();
        }, item);
    }
    userFlash.program();
}

//=================================
// Implementation of ConfigParamClass class
//=================================
ConfigParamClass& ConfigParamClass::instance()
{
    static ConfigParamClass _instance; // Singleton
    return _instance;
}

void ConfigParamClass::printInfo() const
{
    Params::instance().printInfo();
}

void ConfigParamClass::loadDefault()
{
    Params::instance().loadDefault();
}

uint32_t ConfigParamClass::getBootCountFromFlash()
{
    auto& param = P_CFG_BOOT_COUNT;
    param.readFromFlash();
    return param.get();
}

void ConfigParamClass::incBootCount()
{
    auto& param = P_CFG_BOOT_COUNT;
    param.set(param.get() + 1);
}

void ConfigParamClass::initialize()
{
    // load default
    loadDefault();
    // don't load from Flash if flash is blank
    if (getBootCountFromFlash() == 0xffffffffUL) { return; }

    // load from flash and get format revision
    uint32_t formatRevExpected = P_CFG_FORMAT_REV.get();
    Params::instance().loadFromFlash();
    uint32_t formtRev = P_CFG_FORMAT_REV.get();

    // Force to reset to default due to format revision changed (parameter/address) to avoid mulfunction
    if (formatRevExpected != formtRev) {
        loadDefault();
    }
}

void ConfigParamClass::finalize() const
{
    Params::instance().storeToFlash();
}
}
