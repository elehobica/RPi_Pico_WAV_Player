/*-----------------------------------------------------------/
/ ConfigParam.h
/------------------------------------------------------------/
/ Copyright (c) 2024, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/-----------------------------------------------------------*/

#pragma once

#include <map>
#include <string>
#include <cinttypes>  // this must be located at later than <string>
#include <variant>

#include "UserFlash.h"


namespace ConfigParamNs {
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
    CFG_MENU_IDX_GENERAL_PUSH_BUTTON_LAYOUT,
    CFG_MENU_IDX_GENERAL_HP_BUTTON_LAYOUT,
    CFG_MENU_IDX_DISPLAY_LCD_CONFIG,
    CFG_MENU_IDX_DISPLAY_ROTATION,
    CFG_MENU_IDX_DISPLAY_BACKLIGHT_LOW_LEVEL,
    CFG_MENU_IDX_DISPLAY_BACKLIGHT_HIGH_LEVEL,
    CFG_MENU_IDX_DISPLAY_TIME_TO_BACKLIGHT_LOW,
    CFG_MENU_IDX_PLAY_TIME_TO_NEXT_PLAY,
    CFG_MENU_IDX_PLAY_NEXT_PLAY_ALBUM,
    CFG_MENU_IDX_PLAY_RANDOM_DIR_DEPTH,
} ParamId_t;

//=================================
// Interface of Parameter class
//=================================
template <class T>
class Parameter {
    using valueType = T;
public:
    void set(valueType value_) { value = value_; }
    valueType get() const { return value; }
private:
    Parameter(const ParamId_t& id, const char* name, const uint32_t& flashAddr, const valueType& defaultValue, size_t& size);
    Parameter(const ParamId_t& id, const char* name, const uint32_t& flashAddr, const valueType& defaultValue);
    Parameter(const Parameter&) = delete;
    Parameter& operator=(const Parameter&) = delete;  // don't permit copy
    void loadDefault() { value = defaultValue; }
    const ParamId_t id;
    const char* name;
    const uint32_t flashAddr;
    const valueType defaultValue;
    const size_t size;
    valueType value = defaultValue;
    friend class Params;
    friend class ConfigParamClass;
    friend class ReadFromFlashVisitor;
    friend class WriteReserveVisitor;
    friend class PrintInfoVisitor;
};

using variant_t = std::variant<
    Parameter<bool>*,
    Parameter<uint8_t>*, Parameter<uint16_t>*, Parameter<uint32_t>*, Parameter<uint64_t>*,
    Parameter<int8_t>*, Parameter<int16_t>*, Parameter<int32_t>*, Parameter<int64_t>*,
    Parameter<char*>*>;

//=================================
// Interface of Visitors
//=================================
struct FlashVisitor {
    UserFlash& userFlash = UserFlash::instance();
};

struct ReadFromFlashVisitor : FlashVisitor {
    template <typename T>
    void operator()(const T& param) const {
        userFlash.read(param->flashAddr, param->size, param->value);
    }
};

struct WriteReserveVisitor : FlashVisitor {
    template <typename T>
    void operator()(const T& param) const {
        userFlash.writeReserve(param->flashAddr, param->size, param->value);
    }
};

struct PrintInfoVisitor {
    template <typename T>
    void operator()(T& param) const {
        const variant_t item = param;
        const auto& format = printFormat.at(item.index());
        printf(format.c_str(), param->flashAddr, param->name, param->value, param->value);
    }
    const std::array<std::string, 10> printFormat = {  // this must be matched with variant order due to being referred by index()
        "0x%04x %s: %" PRIi32 "d (0x%" PRIx32 ")\n",  // bool
        "0x%04x %s: %" PRIu8 "d (0x%" PRIx8 ")\n",    // uint8_t
        "0x%04x %s: %" PRIu16 "d (0x%" PRIx16 ")\n",  // uint16_t
        "0x%04x %s: %" PRIu32 "d (0x%" PRIx32 ")\n",  // uint32_t
        "0x%04x %s: %" PRIu64 "d (0x%" PRIx64 ")\n",  // uint64_t
        "0x%04x %s: %" PRIi8 "d (0x%" PRIx8 ")\n",    // int8_t
        "0x%04x %s: %" PRIi16 "d (0x%" PRIx16 ")\n",  // int16_t
        "0x%04x %s: %" PRIi32 "d (0x%" PRIx32 ")\n",  // int32_t
        "0x%04x %s: %" PRIi64 "d (0x%" PRIx64 ")\n",  // int64_t
        "0x%04x %s: %s\n",                            // char*
    };
};

//=================================
// Interface of Params class
//=================================
class Params
{
// all private except for friend classes
    static Params& instance(); // Singleton
    void printInfo() const;
    void loadDefault();
    void loadFromFlash();
    void storeToFlash() const;
    template <typename T>
    void add(const ParamId_t& id, T* paramPtr) { paramMap[id] = paramPtr; }
    template <typename T>
    T& getParam(const ParamId_t& id) {
        auto& item = paramMap.at(id);
        auto& paramPtr = std::get<T*>(item);
        return *paramPtr;
    }
    std::map<const ParamId_t, variant_t> paramMap;
    template<typename> friend class Parameter;  // for all Parameter<> classes
    friend class ConfigParamClass;
};

//=================================
// Interface of ConfigParamClass class
//=================================
class ConfigParamClass
{
public:
    static ConfigParamClass& instance(); // Singleton
    void printInfo() const;
    void initialize();
    void finalize() const;
    void incBootCount();
    // accessor by Parameter<> instance on template T = Parameter<>
    template <typename T>
    decltype(auto) getValue(const T& param) const { return param.get(); }
    template <typename T>
    void setValue(T& param, const typename T::valueType value) { param.set(value); }
    // accessor by ParamId_t on template T = primitive type
    template <typename T>
    decltype(auto) getValue(const ParamId_t& id) const { return _getValue<Parameter<T>>(id); }
    template <typename T>
    void setValue(const ParamId_t& id, const T& value) { _setValue<Parameter<T>>(id, value); }

    // Parameter<T>     inst                                         id                                          name                                          addr   default
    Parameter<uint32_t> P_CFG_BOOT_COUNT                            {CFG_BOOT_COUNT,                             "CFG_BOOT_COUNT",                             0x000, 10};
    Parameter<uint32_t> P_CFG_FORMAT_REV                            {CFG_FORMAT_REV,                             "CFG_FORMAT_REV",                             0x004, 20240513};  // update value when updated to reset user flash
    Parameter<uint32_t> P_CFG_SEED                                  {CFG_SEED,                                   "CFG_SEED",                                   0x008, 0};
    Parameter<uint8_t>  P_CFG_VOLUME                                {CFG_VOLUME,                                 "CFG_VOLUME",                                 0x00c, 65};
    Parameter<uint8_t>  P_CFG_STACK_COUNT                           {CFG_STACK_COUNT,                            "CFG_STACK_COUNT",                            0x00d, 0};
    Parameter<uint16_t> P_CFG_STACK_HEAD0                           {CFG_STACK_HEAD0,                            "CFG_STACK_HEAD0",                            0x010, 0};
    Parameter<uint16_t> P_CFG_STACK_COLUMN0                         {CFG_STACK_COLUMN0,                          "CFG_STACK_COLUMN0",                          0x012, 0};
    Parameter<uint16_t> P_CFG_STACK_HEAD1                           {CFG_STACK_HEAD1,                            "CFG_STACK_HEAD1",                            0x014, 0};
    Parameter<uint16_t> P_CFG_STACK_COLUMN1                         {CFG_STACK_COLUMN1,                          "CFG_STACK_COLUMN1",                          0x016, 0};
    Parameter<uint16_t> P_CFG_STACK_HEAD2                           {CFG_STACK_HEAD2,                            "CFG_STACK_HEAD2",                            0x018, 0};
    Parameter<uint16_t> P_CFG_STACK_COLUMN2                         {CFG_STACK_COLUMN2,                          "CFG_STACK_COLUMN2",                          0x01a, 0};
    Parameter<uint16_t> P_CFG_STACK_HEAD3                           {CFG_STACK_HEAD3,                            "CFG_STACK_HEAD3",                            0x01c, 0};
    Parameter<uint16_t> P_CFG_STACK_COLUMN3                         {CFG_STACK_COLUMN3,                          "CFG_STACK_COLUMN3",                          0x020, 0};
    Parameter<uint16_t> P_CFG_STACK_HEAD4                           {CFG_STACK_HEAD4,                            "CFG_STACK_HEAD4",                            0x022, 0};
    Parameter<uint16_t> P_CFG_STACK_COLUMN4                         {CFG_STACK_COLUMN4,                          "CFG_STACK_COLUMN4",                          0x024, 0};
    Parameter<uint32_t> P_CFG_UIMODE                                {CFG_UIMODE,                                 "CFG_UIMODE",                                 0x028, 0};
    Parameter<uint16_t> P_CFG_IDX_HEAD                              {CFG_IDX_HEAD,                               "CFG_IDX_HEAD",                               0x02c, 0};
    Parameter<uint16_t> P_CFG_IDX_COLUMN                            {CFG_IDX_COLUMN,                             "CFG_IDX_COLUMN",                             0x02e, 0};
    Parameter<uint16_t> P_CFG_IDX_PLAY                              {CFG_IDX_PLAY,                               "CFG_IDX_PLAY",                               0x030, 0};
    Parameter<uint64_t> P_CFG_PLAY_POS                              {CFG_PLAY_POS,                               "CFG_PLAY_POS",                               0x038, 0};
    Parameter<uint32_t> P_CFG_SAMPLES_PLAYED                        {CFG_SAMPLES_PLAYED,                         "CFG_SAMPLES_PLAYED",                         0x040, 0};
    // type of CFG_MENU_xxx must be uint32_t and default values indidicates index of selection (see ConfigMenu.h)
    Parameter<uint32_t> P_CFG_MENU_IDX_GENERAL_TIME_TO_POWER_OFF    {CFG_MENU_IDX_GENERAL_TIME_TO_POWER_OFF,     "CFG_MENU_IDX_GENERAL_TIME_TO_POWER_OFF",     0x080, 0};
    Parameter<uint32_t> P_CFG_MENU_IDX_GENERAL_TIME_TO_LEAVE_CONFIG {CFG_MENU_IDX_GENERAL_TIME_TO_LEAVE_CONFIG,  "CFG_MENU_IDX_GENERAL_TIME_TO_LEAVE_CONFIG",  0x084, 1};
    Parameter<uint32_t> P_CFG_MENU_IDX_GENERAL_PUSH_BUTTON_LAYOUT   {CFG_MENU_IDX_GENERAL_PUSH_BUTTON_LAYOUT,    "CFG_MENU_IDX_GENERAL_PUSH_BUTTON_LAYOUT",    0x088, 0};
    Parameter<uint32_t> P_CFG_MENU_IDX_GENERAL_HP_BUTTON_LAYOUT     {CFG_MENU_IDX_GENERAL_HP_BUTTON_LAYOUT,      "CFG_MENU_IDX_GENERAL_HP_BUTTON_LAYOUT",      0x08c, 1};
    Parameter<uint32_t> P_CFG_MENU_IDX_DISPLAY_LCD_CONFIG           {CFG_MENU_IDX_DISPLAY_LCD_CONFIG,            "CFG_MENU_IDX_DISPLAY_LCD_CONFIG",            0x090, 0};
    Parameter<uint32_t> P_CFG_MENU_IDX_DISPLAY_ROTATION             {CFG_MENU_IDX_DISPLAY_ROTATION,              "CFG_MENU_IDX_DISPLAY_ROTATION",              0x094, 0};
    Parameter<uint32_t> P_CFG_MENU_IDX_DISPLAY_BACKLIGHT_LOW_LEVEL  {CFG_MENU_IDX_DISPLAY_BACKLIGHT_LOW_LEVEL,   "CFG_MENU_IDX_DISPLAY_BACKLIGHT_LOW_LEVEL",   0x098, 7};
    Parameter<uint32_t> P_CFG_MENU_IDX_DISPLAY_BACKLIGHT_HIGH_LEVEL {CFG_MENU_IDX_DISPLAY_BACKLIGHT_HIGH_LEVEL,  "CFG_MENU_IDX_DISPLAY_BACKLIGHT_HIGH_LEVEL",  0x09c, 12};
    Parameter<uint32_t> P_CFG_MENU_IDX_DISPLAY_TIME_TO_BACKLIGHT_LOW{CFG_MENU_IDX_DISPLAY_TIME_TO_BACKLIGHT_LOW, "CFG_MENU_IDX_DISPLAY_TIME_TO_BACKLIGHT_LOW", 0x0a0, 1};
    Parameter<uint32_t> P_CFG_MENU_IDX_PLAY_TIME_TO_NEXT_PLAY       {CFG_MENU_IDX_PLAY_TIME_TO_NEXT_PLAY,        "CFG_MENU_IDX_PLAY_TIME_TO_NEXT_PLAY",        0x0a4, 2};
    Parameter<uint32_t> P_CFG_MENU_IDX_PLAY_NEXT_PLAY_ALBUM         {CFG_MENU_IDX_PLAY_NEXT_PLAY_ALBUM,          "CFG_MENU_IDX_PLAY_NEXT_PLAY_ALBUM",          0x0a8, 1};
    Parameter<uint32_t> P_CFG_MENU_IDX_PLAY_RANDOM_DIR_DEPTH        {CFG_MENU_IDX_PLAY_RANDOM_DIR_DEPTH,         "CFG_MENU_IDX_PLAY_RANDOM_DIR_DEPTH",         0x0ac, 1};

private:
    ConfigParamClass() = default;
    ~ConfigParamClass() = default;
    ConfigParamClass(const ConfigParamClass&) = delete;
    ConfigParamClass& operator=(const ConfigParamClass&) = delete;
    void loadDefault();
    uint32_t getBootCountFromFlash();
    // accessor by ParamId_t on template T = Patameter<>
    template <typename T>
    void _setValue(const ParamId_t& id, const typename T::valueType& value) {
        auto& param = Params::instance().getParam<T>(id);
        return param.set(value);
    }
    template <typename T>
    decltype(auto) _getValue(const ParamId_t& id) const {
        const auto& param = Params::instance().getParam<T>(id);
        return param.get();
    }
};
}

// alias to ConfigParamNs::ConfigParamClass
using ConfigParam = ConfigParamNs::ConfigParamClass;
