/*-----------------------------------------------------------/
/ ConfigParam.h
/------------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/-----------------------------------------------------------*/

#pragma once

#include <map>
#include <string>
#include <cinttypes>  // this must be located at later than <string>
#include <variant>
#include <vector>

#define CFG_STACK_HEAD(x)       ((x == 4) ? ConfigParam::CFG_STACK_HEAD4 : (x == 3) ? ConfigParam::CFG_STACK_HEAD3 : (x == 2) ? ConfigParam::CFG_STACK_HEAD2 : (x == 1) ? ConfigParam::CFG_STACK_HEAD1 : ConfigParam::CFG_STACK_HEAD0)
#define CFG_STACK_COLUMN(x)     ((x == 4) ? ConfigParam::CFG_STACK_COLUMN4 : (x == 3) ? ConfigParam::CFG_STACK_COLUMN3 : (x == 2) ? ConfigParam::CFG_STACK_COLUMN2 : (x == 1) ? ConfigParam::CFG_STACK_COLUMN1 : ConfigParam::CFG_STACK_COLUMN0)

#define GETBOOL(identifier)     (ConfigParam::instance().getValue<bool>(identifier))
#define GETU8(identifier)       (ConfigParam::instance().getValue<uint8_t>(identifier))
#define GETU16(identifier)      (ConfigParam::instance().getValue<uint16_t>(identifier))
#define GETU32(identifier)      (ConfigParam::instance().getValue<uint32_t>(identifier))
#define GETU64(identifier)      (ConfigParam::instance().getValue<uint64_t>(identifier))
#define GETI8(identifier)       (ConfigParam::instance().getValue<int8_t>(identifier))
#define GETI16(identifier)      (ConfigParam::instance().getValue<int16_t>(identifier))
#define GETI32(identifier)      (ConfigParam::instance().getValue<int32_t>(identifier))
#define GETI64(identifier)      (ConfigParam::instance().getValue<int64_t>(identifier))
#define GETSTR(identifier)      (ConfigParam::instance().getValue<std::string>(identifier))

#define GET_CFG_BOOT_COUNT      GETU32(ConfigParam::CFG_BOOT_COUNT)
#define GET_CFG_FORMAT_REV      GETU32(ConfigParam::CFG_FORMAT_REV)
#define GET_CFG_SEED            GETU32(ConfigParam::CFG_BOOT_COUNT)
#define GET_CFG_VOLUME          GETU8(ConfigParam::CFG_VOLUME)
#define GET_CFG_STACK_COUNT     GETU8(ConfigParam::CFG_STACK_COUNT)
#define GET_CFG_STACK_HEAD(x)   GETU16(CFG_STACK_HEAD(x))
#define GET_CFG_STACK_COLUMN(x) GETU16(CFG_STACK_COLUMN(x))
#define GET_CFG_UIMODE          GETU32(ConfigParam::CFG_UIMODE)
#define GET_CFG_IDX_HEAD        GETU16(ConfigParam::CFG_IDX_HEAD)
#define GET_CFG_IDX_COLUMN      GETU16(ConfigParam::CFG_IDX_COLUMN)
#define GET_CFG_IDX_PLAY        GETU16(ConfigParam::CFG_IDX_PLAY)
#define GET_CFG_PLAY_POS        GETU64(ConfigParam::CFG_PLAY_POS)
#define GET_CFG_SAMPLES_PLAYED  GETU32(ConfigParam::CFG_SAMPLES_PLAYED)

//=================================
// Interface of ConfigParam class
//=================================
class ConfigParam
{
public:
    typedef enum {
        CFG_BOOT_COUNT = 0,
        CFG_FORMAT_REV,
        CFG_SEED,
        CFG_VOLUME,
        CFG_STACK_COUNT,
        CFG_STACK_HEAD0,
        CFG_STACK_COLUMN0,
        CFG_STACK_HEAD1,
        CFG_STACK_COLUMN1,
        CFG_STACK_HEAD2,
        CFG_STACK_COLUMN2,
        CFG_STACK_HEAD3,
        CFG_STACK_COLUMN3,
        CFG_STACK_HEAD4,
        CFG_STACK_COLUMN4,
        CFG_UIMODE,
        CFG_IDX_HEAD,
        CFG_IDX_COLUMN,
        CFG_IDX_PLAY,
        CFG_PLAY_POS,
        CFG_SAMPLES_PLAYED,
        CFG_MENU_IDX_GENERAL_TIME_TO_POWER_OFF,
        CFG_MENU_IDX_GENERAL_TIME_TO_LEAVE_CONFIG,
        CFG_MENU_IDX_DISPLAY_LCD_CONFIG,
        CFG_MENU_IDX_DISPLAY_ROTATION,
        CFG_MENU_IDX_DISPLAY_BACKLIGHT_LOW_LEVEL,
        CFG_MENU_IDX_DISPLAY_BACKLIGHT_HIGH_LEVEL,
        CFG_MENU_IDX_DISPLAY_TIME_TO_BACKLIGHT_LOW,
        CFG_MENU_IDX_PLAY_TIME_TO_NEXT_PLAY,
        CFG_MENU_IDX_PLAY_NEXT_PLAY_ALBUM,
        CFG_MENU_IDX_PLAY_RANDOM_DIR_DEPTH,
    } ParamId_t;

    static ConfigParam& instance(); // Singleton
    void printInfo() const;
    void initialize();
    void finalize();
    void incBootCount();
    template <typename T>
    void setValue(const ParamId_t& id, const T& value) {
        auto& param = configParamItems.at(id);
        std::get<T>(param.value) = value;
    }
    template <typename T>
    T getValue(const ParamId_t& id) const {
        const auto& param = configParamItems.at(id);
        return std::get<T>(param.value);
    }
    template <int N>
    decltype(auto) getValue(const ParamId_t& id) const {
        const auto& param = configParamItems.at(id);
        return std::get<N>(param.value);
    }
private:
    using variant_t = std::variant<bool, uint8_t, uint16_t, uint32_t, uint64_t, int8_t, int16_t, int32_t, int64_t, std::string>;
    const std::vector<std::string> printFormat = {  // this must be matched with variant order due to being referred by index()
        "0x%04x %s: %" PRIi32 "d (0x%" PRIx32 ")\n",  // bool
        "0x%04x %s: %" PRIu8 "d (0x%" PRIx8 ")\n",    // uint8_t
        "0x%04x %s: %" PRIu16 "d (0x%" PRIx16 ")\n",  // uint16_t
        "0x%04x %s: %" PRIu32 "d (0x%" PRIx32 ")\n",  // uint32_t
        "0x%04x %s: %" PRIu64 "d (0x%" PRIx64 ")\n",  // uint64_t
        "0x%04x %s: %" PRIi8 "d (0x%" PRIx8 ")\n",    // int8_t
        "0x%04x %s: %" PRIi16 "d (0x%" PRIx16 ")\n",  // int16_t
        "0x%04x %s: %" PRIi32 "d (0x%" PRIx32 ")\n",  // int32_t
        "0x%04x %s: %" PRIi64 "d (0x%" PRIx64 ")\n",  // int64_t
        "0x%04x %s: %s\n",                            // std::string
    };
    struct Parameter {
        Parameter(const char* name, const std::string defaultValue, const uint32_t flashAddr, size_t size)
            : name(name), defaultValue(defaultValue), flashAddr(flashAddr), size(size) {}
        template <typename T>
        Parameter(const char* name, const T defaultValue, const uint32_t flashAddr)
            : name(name), defaultValue(defaultValue), flashAddr(flashAddr), size(sizeof(T)) {}
        const char* name;
        const variant_t defaultValue;
        const uint32_t flashAddr;
        const size_t size;
        variant_t value = defaultValue;
        void loadDefault() { value = defaultValue; }
    };
    std::map<const ParamId_t, Parameter> configParamItems = {
    //  id                    name                  default             flashAddr
        {CFG_BOOT_COUNT,     {"CFG_BOOT_COUNT",     uint32_t{10},       0x000}},
        {CFG_FORMAT_REV,     {"CFG_FORMAT_REV",     uint32_t{20240401}, 0x004}},  // update value when updated to reset user flash
        {CFG_SEED,           {"CFG_SEED",           uint32_t{0},        0x008}},
        {CFG_VOLUME,         {"CFG_VOLUME",         uint8_t{65},        0x00c}},
        {CFG_STACK_COUNT,    {"CFG_STACK_COUNT",    uint8_t{0},         0x00d}},
        {CFG_STACK_HEAD0,    {"CFG_STACK_HEAD0",    uint16_t{0},        0x010}},
        {CFG_STACK_COLUMN0,  {"CFG_STACK_COLUMN0",  uint16_t{0},        0x012}},
        {CFG_STACK_HEAD1,    {"CFG_STACK_HEAD1",    uint16_t{0},        0x014}},
        {CFG_STACK_COLUMN1,  {"CFG_STACK_COLUMN1",  uint16_t{0},        0x016}},
        {CFG_STACK_HEAD2,    {"CFG_STACK_HEAD2",    uint16_t{0},        0x018}},
        {CFG_STACK_COLUMN2,  {"CFG_STACK_COLUMN2",  uint16_t{0},        0x01a}},
        {CFG_STACK_HEAD3,    {"CFG_STACK_HEAD3",    uint16_t{0},        0x01c}},
        {CFG_STACK_COLUMN3,  {"CFG_STACK_COLUMN3",  uint16_t{0},        0x020}},
        {CFG_STACK_HEAD4,    {"CFG_STACK_HEAD4",    uint16_t{0},        0x022}},
        {CFG_STACK_COLUMN4,  {"CFG_STACK_COLUMN4",  uint16_t{0},        0x024}},
        {CFG_UIMODE,         {"CFG_UIMODE",         uint32_t{0},        0x028}},
        {CFG_IDX_HEAD,       {"CFG_IDX_HEAD",       uint16_t{0},        0x02c}},
        {CFG_IDX_COLUMN,     {"CFG_IDX_COLUMN",     uint16_t{0},        0x02e}},
        {CFG_IDX_PLAY,       {"CFG_IDX_PLAY",       uint16_t{0},        0x030}},
        {CFG_PLAY_POS,       {"CFG_PLAY_POS",       uint64_t{0},        0x038}},
        {CFG_SAMPLES_PLAYED, {"CFG_SAMPLES_PLAYED", uint32_t{0},        0x040}},
        // type of CFG_MENU must be uint32_t and default values indicates index of selection (see ConfigMenu.h)
        {CFG_MENU_IDX_GENERAL_TIME_TO_POWER_OFF,     {"CFG_MENU_IDX_GENERAL_TIME_TO_POWER_OFF",     uint32_t{0},    0x080}},
        {CFG_MENU_IDX_GENERAL_TIME_TO_LEAVE_CONFIG,  {"CFG_MENU_IDX_GENERAL_TIME_TO_LEAVE_CONFIG",  uint32_t{1},    0x084}},
        {CFG_MENU_IDX_DISPLAY_LCD_CONFIG,            {"CFG_MENU_IDX_DISPLAY_LCD_CONFIG",            uint32_t{0},    0x088}},
        {CFG_MENU_IDX_DISPLAY_ROTATION,              {"CFG_MENU_IDX_DISPLAY_ROTATION",              uint32_t{0},    0x08c}},
        {CFG_MENU_IDX_DISPLAY_BACKLIGHT_LOW_LEVEL,   {"CFG_MENU_IDX_DISPLAY_BACKLIGHT_LOW_LEVEL",   uint32_t{7},    0x090}},
        {CFG_MENU_IDX_DISPLAY_BACKLIGHT_HIGH_LEVEL,  {"CFG_MENU_IDX_DISPLAY_BACKLIGHT_HIGH_LEVEL",  uint32_t{12},   0x094}},
        {CFG_MENU_IDX_DISPLAY_TIME_TO_BACKLIGHT_LOW, {"CFG_MENU_IDX_DISPLAY_TIME_TO_BACKLIGHT_LOW", uint32_t{1},    0x098}},
        {CFG_MENU_IDX_PLAY_TIME_TO_NEXT_PLAY,        {"CFG_MENU_IDX_PLAY_TIME_TO_NEXT_PLAY",        uint32_t{2},    0x09c}},
        {CFG_MENU_IDX_PLAY_NEXT_PLAY_ALBUM,          {"CFG_MENU_IDX_PLAY_NEXT_PLAY_ALBUM",          uint32_t{1},    0x0a0}},
        {CFG_MENU_IDX_PLAY_RANDOM_DIR_DEPTH,         {"CFG_MENU_IDX_PLAY_RANDOM_DIR_DEPTH",         uint32_t{1},    0x0a4}},
    };

    ConfigParam();
    ~ConfigParam() = default;
    ConfigParam(const ConfigParam&) = delete;
    ConfigParam& operator=(const ConfigParam&) = delete;
    void loadDefault();
    uint32_t getBootCountFromFlash();
};
