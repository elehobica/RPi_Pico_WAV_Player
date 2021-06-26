/*-----------------------------------------------------------/
/ ConfigParam.h
/------------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/-----------------------------------------------------------*/

#include <cstdio>
#include <cstring>
#include <cinttypes>
#include <cassert>
#include "UserFlash.h"
#include "ConfigParam.h"

using namespace ConfigParam;

ConfigParamClass& ConfigParamClass::instance()
{
    static ConfigParamClass _instance; // Singleton
    return _instance;
}

ConfigParamClass::ConfigParamClass()
{
    for (int i = 0; i < NUM_CFG_PARAMS; i++) {
        ParamItem_t *item = &configParamItems[i];
        if (item->ptr == nullptr) {
            item->ptr = malloc(item->size);
        }
    }
}

ConfigParamClass::~ConfigParamClass()
{
    for (int i = 0; i < NUM_CFG_PARAMS; i++) {
        ParamItem_t *item = &configParamItems[i];
        if (item->ptr != nullptr) {
            free(item->ptr);
        }
    }
}

void ConfigParamClass::printInfo()
{
    printf("=== ConfigParam ===\n");
    for (int i = 0; i < NUM_CFG_PARAMS; i++) {
        ParamItem_t *item = &configParamItems[i];
        switch (item->paramType) {
            case CFG_BOOL_T:
            case CFG_UINT8_T:
                {
                    uint8_t *ptr = reinterpret_cast<uint8_t *>(item->ptr);
                    printf("0x%04x %s: %" PRIu8 "d (0x%" PRIx8 ")\n", item->flashAddr, item->name, *ptr, *ptr);
                }
                break;
            case CFG_UINT16_T:
                {
                    uint16_t *ptr = reinterpret_cast<uint16_t *>(item->ptr);
                    printf("0x%04x %s: %" PRIu16 "d (0x%" PRIx16 ")\n", item->flashAddr, item->name, *ptr, *ptr);
                }
                break;
            case CFG_UINT32_T:
                {
                    uint32_t *ptr = reinterpret_cast<uint32_t *>(item->ptr);
                    printf("0x%04x %s: %" PRIu32 "d (0x%" PRIx32 ")\n", item->flashAddr, item->name, *ptr, *ptr);
                }
                break;
            case CFG_UINT64_T:
                {
                    uint64_t *ptr = reinterpret_cast<uint64_t *>(item->ptr);
                    printf("0x%04x %s: %" PRIu64 "d (0x%" PRIx64 ")\n", item->flashAddr, item->name, *ptr, *ptr);
                }
                break;
            case CFG_INT8_T:
                {
                    int8_t *ptr = reinterpret_cast<int8_t *>(item->ptr);
                    printf("0x%04x %s: %" PRIi8 "d (0x%" PRIx8 ")\n", item->flashAddr, item->name, *ptr, *ptr);
                }
                break;
            case CFG_INT16_T:
                {
                    int16_t *ptr = reinterpret_cast<int16_t *>(item->ptr);
                    printf("0x%04x %s: %" PRIi16 "d (0x%" PRIx16 ")\n", item->flashAddr, item->name, *ptr, *ptr);
                }
                break;
            case CFG_INT32_T:
                {
                    int32_t *ptr = reinterpret_cast<int32_t *>(item->ptr);
                    printf("0x%04x %s: %" PRIi32 "d (0x%" PRIx32 ")\n", item->flashAddr, item->name, *ptr, *ptr);
                }
                break;
            case CFG_INT64_T:
                {
                    int64_t *ptr = reinterpret_cast<int64_t *>(item->ptr);
                    printf("0x%04x %s: %" PRIi64 "d (0x%" PRIx64 ")\n", item->flashAddr, item->name, *ptr, *ptr);
                }
                break;
            case CFG_STRING_T:
                {
                    char *ptr = reinterpret_cast<char *>(item->ptr);
                    printf("0x%04x %s: %s\n", item->flashAddr, item->name, ptr);
                }
                break;
            default:
                break;
        }
    }
}

void ConfigParamClass::loadDefault()
{
    for (int i = 0; i < NUM_CFG_PARAMS; i++) {
        ParamItem_t *item = &configParamItems[i];
        switch (item->paramType) {
            case CFG_BOOL_T:
            case CFG_UINT8_T:
                {
                    uint8_t *ptr = reinterpret_cast<uint8_t *>(item->ptr);
                    *ptr = static_cast<uint8_t>(atoi(item->defaultValue));
                }
                break;
            case CFG_UINT16_T:
                {
                    uint16_t *ptr = reinterpret_cast<uint16_t *>(item->ptr);
                    *ptr = static_cast<uint16_t>(atoi(item->defaultValue));
                }
                break;
            case CFG_UINT32_T:
                {
                    uint32_t *ptr = reinterpret_cast<uint32_t *>(item->ptr);
                    *ptr = static_cast<uint32_t>(atoi(item->defaultValue));
                }
                break;
            case CFG_UINT64_T:
                {
                    uint64_t *ptr = reinterpret_cast<uint64_t *>(item->ptr);
                    *ptr = static_cast<uint64_t>(atoi(item->defaultValue));
                }
                break;
            case CFG_INT8_T:
                {
                    int8_t *ptr = reinterpret_cast<int8_t *>(item->ptr);
                    *ptr = static_cast<int8_t>(atoi(item->defaultValue));
                }
                break;
            case CFG_INT16_T:
                {
                    int16_t *ptr = reinterpret_cast<int16_t *>(item->ptr);
                    *ptr = static_cast<int16_t>(atoi(item->defaultValue));
                }
                break;
            case CFG_INT32_T:
                {
                    int32_t *ptr = reinterpret_cast<int32_t *>(item->ptr);
                    *ptr = static_cast<int32_t>(atoi(item->defaultValue));
                }
                break;
            case CFG_INT64_T:
                {
                    int64_t *ptr = reinterpret_cast<int64_t *>(item->ptr);
                    *ptr = static_cast<int64_t>(atoi(item->defaultValue));
                }
                break;
            case CFG_STRING_T:
                {
                    char *ptr = reinterpret_cast<char *>(item->ptr);
                    memset(ptr, 0, item->size);
                    strncpy(ptr, item->defaultValue, item->size-1);
                }
                break;
            default:
                break;
        }
    }
}

uint32_t ConfigParamClass::getBootCountFromFlash()
{
    ParamItem_t *item = &configParamItems[CFG_BOOT_COUNT];
    uint32_t bootCount;
    userFlash.read(item->flashAddr, item->size, &bootCount);
    return bootCount;
}

void ConfigParamClass::incBootCount()
{
    uint32_t bootCount;
    this->read(CFG_BOOT_COUNT, &bootCount);
    bootCount++;
    this->write(CFG_BOOT_COUNT, &bootCount);
}

void ConfigParamClass::initialize(LoadDefaultBehavior_t loadDefaultBehavior)
{
    // load default
    loadDefault();
    // switch to step to load from Flash
    if (loadDefaultBehavior == FORCE_LOAD_DEFAULT) { return; }
    if (loadDefaultBehavior == LOAD_DEFAULT_IF_FLASH_IS_BLANK && getBootCountFromFlash() == 0xffffffffUL) { return; }
    // load from Flash
    for (int i = 0; i < NUM_CFG_PARAMS; i++) {
        ParamItem_t *item = &configParamItems[i];
        userFlash.read(item->flashAddr, item->size, item->ptr);
    }
}

void ConfigParamClass::finalize()
{
    // store to Flash
    for (int i = 0; i < NUM_CFG_PARAMS; i++) {
        ParamItem_t *item = &configParamItems[i];
        userFlash.writeReserve(item->flashAddr, item->size, item->ptr);
    }
    userFlash.program();
}

void ConfigParamClass::read(ParamID_t id, void *ptr)
{
    if (id < NUM_CFG_PARAMS) {
        ParamItem_t *item = &configParamItems[id];
        memcpy(ptr, item->ptr, item->size);
    }
}

void ConfigParamClass::write(ParamID_t id, const void *ptr)
{
    if (id < NUM_CFG_PARAMS) {
        ParamItem_t *item = &configParamItems[id];
        memcpy(item->ptr, ptr, item->size);
    }
}

bool ConfigParamClass::getBool(ParamID_t id)
{
    assert(configParamItems[id].paramType == CFG_BOOL_T);
    uint8_t value = *(reinterpret_cast<uint8_t *>(configParamItems[id].ptr));
    return (value != 0) ? true : false;
}

uint8_t ConfigParamClass::getU8(ParamID_t id)
{
    assert(configParamItems[id].paramType == CFG_UINT8_T);
    return *(reinterpret_cast<uint8_t *>(configParamItems[id].ptr));
}

uint16_t ConfigParamClass::getU16(ParamID_t id)
{
    assert(configParamItems[id].paramType == CFG_UINT16_T);
    return *(reinterpret_cast<uint16_t *>(configParamItems[id].ptr));
}

uint32_t ConfigParamClass::getU32(ParamID_t id)
{
    assert(configParamItems[id].paramType == CFG_UINT32_T);
    return *(reinterpret_cast<uint32_t *>(configParamItems[id].ptr));
}

uint64_t ConfigParamClass::getU64(ParamID_t id)
{
    assert(configParamItems[id].paramType == CFG_UINT64_T);
    return *(reinterpret_cast<uint64_t *>(configParamItems[id].ptr));
}

int8_t ConfigParamClass::getI8(ParamID_t id)
{
    assert(configParamItems[id].paramType == CFG_INT8_T);
    return *(reinterpret_cast<int8_t *>(configParamItems[id].ptr));
}

int16_t ConfigParamClass::getI16(ParamID_t id)
{
    assert(configParamItems[id].paramType == CFG_INT16_T);
    return *(reinterpret_cast<int16_t *>(configParamItems[id].ptr));
}

int32_t ConfigParamClass::getI32(ParamID_t id)
{
    assert(configParamItems[id].paramType == CFG_INT32_T);
    return *(reinterpret_cast<int32_t *>(configParamItems[id].ptr));
}

int64_t ConfigParamClass::getI64(ParamID_t id)
{
    assert(configParamItems[id].paramType == CFG_INT64_T);
    return *(reinterpret_cast<int64_t *>(configParamItems[id].ptr));
}

char *ConfigParamClass::getStr(ParamID_t id)
{
    assert(configParamItems[id].paramType == CFG_STRING_T);
    return reinterpret_cast<char *>(configParamItems[id].ptr);
}

void ConfigParamClass::setU8(ParamID_t id, uint8_t val)
{
    assert(configParamItems[id].paramType == CFG_UINT8_T);
    *(reinterpret_cast<uint8_t *>(configParamItems[id].ptr)) = val;
}

void ConfigParamClass::setU16(ParamID_t id, uint16_t val)
{
    assert(configParamItems[id].paramType == CFG_UINT16_T);
    *(reinterpret_cast<uint16_t *>(configParamItems[id].ptr)) = val;
}

void ConfigParamClass::setU32(ParamID_t id, uint32_t val)
{
    assert(configParamItems[id].paramType == CFG_UINT32_T);
    *(reinterpret_cast<uint32_t *>(configParamItems[id].ptr)) = val;
}

void ConfigParamClass::setU64(ParamID_t id, uint64_t val)
{
    assert(configParamItems[id].paramType == CFG_UINT64_T);
    *(reinterpret_cast<uint64_t *>(configParamItems[id].ptr)) = val;
}

void ConfigParamClass::setI8(ParamID_t id, int8_t val)
{
    assert(configParamItems[id].paramType == CFG_INT8_T);
    *(reinterpret_cast<int8_t *>(configParamItems[id].ptr)) = val;
}

void ConfigParamClass::setI16(ParamID_t id, int16_t val)
{
    assert(configParamItems[id].paramType == CFG_INT16_T);
    *(reinterpret_cast<int16_t *>(configParamItems[id].ptr)) = val;
}

void ConfigParamClass::setI32(ParamID_t id, int32_t val)
{
    assert(configParamItems[id].paramType == CFG_INT32_T);
    *(reinterpret_cast<int32_t *>(configParamItems[id].ptr)) = val;
}

void ConfigParamClass::setI64(ParamID_t id, int64_t val)
{
    assert(configParamItems[id].paramType == CFG_INT64_T);
    *(reinterpret_cast<int64_t *>(configParamItems[id].ptr)) = val;
}

void ConfigParamClass::setStr(ParamID_t id, char *str)
{
    assert(configParamItems[id].paramType == CFG_STRING_T);
    char *ptr = reinterpret_cast<char *>(configParamItems[id].ptr);
    memset(ptr, 0, configParamItems[id].size);
    strncpy(ptr, str, configParamItems[id].size-1);
}
