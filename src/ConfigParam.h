/*-----------------------------------------------------------/
/ ConfigParam.h
/------------------------------------------------------------/
/ Copyright (c) 2024, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/-----------------------------------------------------------*/

#pragma once

#include "FlashParam.h"

typedef enum {
    CFG_REVISION = FlashParamNs::CFG_ID_BASE,
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
    CFG_FLOAT,
    CFG_DOUBLE,
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
// Interface of ConfigParam class
//=================================
struct ConfigParam : FlashParamNs::FlashParam {
    static ConfigParam& instance() {  // Singleton
        static ConfigParam instance;
        return instance;
    }
    // Parameter<T>                      inst                                         id                                          name                                          default  size
    FlashParamNs::Parameter<std::string> P_CFG_REVISION                              {CFG_REVISION,                               "CFG_REVISION",                               "0.9.5", 8};
    FlashParamNs::Parameter<uint32_t>    P_CFG_SEED                                  {CFG_SEED,                                   "CFG_SEED",                                   0};
    FlashParamNs::Parameter<uint8_t>     P_CFG_VOLUME                                {CFG_VOLUME,                                 "CFG_VOLUME",                                 65};
    FlashParamNs::Parameter<uint8_t>     P_CFG_STACK_COUNT                           {CFG_STACK_COUNT,                            "CFG_STACK_COUNT",                            0};
    FlashParamNs::Parameter<uint16_t>    P_CFG_STACK_HEAD0                           {CFG_STACK_HEAD0,                            "CFG_STACK_HEAD0",                            0};
    FlashParamNs::Parameter<uint16_t>    P_CFG_STACK_COLUMN0                         {CFG_STACK_COLUMN0,                          "CFG_STACK_COLUMN0",                          0};
    FlashParamNs::Parameter<uint16_t>    P_CFG_STACK_HEAD1                           {CFG_STACK_HEAD1,                            "CFG_STACK_HEAD1",                            0};
    FlashParamNs::Parameter<uint16_t>    P_CFG_STACK_COLUMN1                         {CFG_STACK_COLUMN1,                          "CFG_STACK_COLUMN1",                          0};
    FlashParamNs::Parameter<uint16_t>    P_CFG_STACK_HEAD2                           {CFG_STACK_HEAD2,                            "CFG_STACK_HEAD2",                            0};
    FlashParamNs::Parameter<uint16_t>    P_CFG_STACK_COLUMN2                         {CFG_STACK_COLUMN2,                          "CFG_STACK_COLUMN2",                          0};
    FlashParamNs::Parameter<uint16_t>    P_CFG_STACK_HEAD3                           {CFG_STACK_HEAD3,                            "CFG_STACK_HEAD3",                            0};
    FlashParamNs::Parameter<uint16_t>    P_CFG_STACK_COLUMN3                         {CFG_STACK_COLUMN3,                          "CFG_STACK_COLUMN3",                          0};
    FlashParamNs::Parameter<uint16_t>    P_CFG_STACK_HEAD4                           {CFG_STACK_HEAD4,                            "CFG_STACK_HEAD4",                            0};
    FlashParamNs::Parameter<uint16_t>    P_CFG_STACK_COLUMN4                         {CFG_STACK_COLUMN4,                          "CFG_STACK_COLUMN4",                          0};
    FlashParamNs::Parameter<uint32_t>    P_CFG_UIMODE                                {CFG_UIMODE,                                 "CFG_UIMODE",                                 0};
    FlashParamNs::Parameter<uint16_t>    P_CFG_IDX_HEAD                              {CFG_IDX_HEAD,                               "CFG_IDX_HEAD",                               0};
    FlashParamNs::Parameter<uint16_t>    P_CFG_IDX_COLUMN                            {CFG_IDX_COLUMN,                             "CFG_IDX_COLUMN",                             0};
    FlashParamNs::Parameter<uint16_t>    P_CFG_IDX_PLAY                              {CFG_IDX_PLAY,                               "CFG_IDX_PLAY",                               0};
    FlashParamNs::Parameter<uint64_t>    P_CFG_PLAY_POS                              {CFG_PLAY_POS,                               "CFG_PLAY_POS",                               0};
    FlashParamNs::Parameter<uint32_t>    P_CFG_SAMPLES_PLAYED                        {CFG_SAMPLES_PLAYED,                         "CFG_SAMPLES_PLAYED",                         0};
    // type of CFG_MENU_xxx must be uint32_t and default values indidicates index of selection (see ConfigMenu.h)
    FlashParamNs::Parameter<uint32_t>    P_CFG_MENU_IDX_GENERAL_TIME_TO_POWER_OFF    {CFG_MENU_IDX_GENERAL_TIME_TO_POWER_OFF,     "CFG_MENU_IDX_GENERAL_TIME_TO_POWER_OFF",     0};
    FlashParamNs::Parameter<uint32_t>    P_CFG_MENU_IDX_GENERAL_TIME_TO_LEAVE_CONFIG {CFG_MENU_IDX_GENERAL_TIME_TO_LEAVE_CONFIG,  "CFG_MENU_IDX_GENERAL_TIME_TO_LEAVE_CONFIG",  1};
    FlashParamNs::Parameter<uint32_t>    P_CFG_MENU_IDX_GENERAL_PUSH_BUTTON_LAYOUT   {CFG_MENU_IDX_GENERAL_PUSH_BUTTON_LAYOUT,    "CFG_MENU_IDX_GENERAL_PUSH_BUTTON_LAYOUT",    0};
    FlashParamNs::Parameter<uint32_t>    P_CFG_MENU_IDX_GENERAL_HP_BUTTON_LAYOUT     {CFG_MENU_IDX_GENERAL_HP_BUTTON_LAYOUT,      "CFG_MENU_IDX_GENERAL_HP_BUTTON_LAYOUT",      1};
    FlashParamNs::Parameter<uint32_t>    P_CFG_MENU_IDX_DISPLAY_LCD_CONFIG           {CFG_MENU_IDX_DISPLAY_LCD_CONFIG,            "CFG_MENU_IDX_DISPLAY_LCD_CONFIG",            0};
    FlashParamNs::Parameter<uint32_t>    P_CFG_MENU_IDX_DISPLAY_ROTATION             {CFG_MENU_IDX_DISPLAY_ROTATION,              "CFG_MENU_IDX_DISPLAY_ROTATION",              0};
    FlashParamNs::Parameter<uint32_t>    P_CFG_MENU_IDX_DISPLAY_BACKLIGHT_LOW_LEVEL  {CFG_MENU_IDX_DISPLAY_BACKLIGHT_LOW_LEVEL,   "CFG_MENU_IDX_DISPLAY_BACKLIGHT_LOW_LEVEL",   7};
    FlashParamNs::Parameter<uint32_t>    P_CFG_MENU_IDX_DISPLAY_BACKLIGHT_HIGH_LEVEL {CFG_MENU_IDX_DISPLAY_BACKLIGHT_HIGH_LEVEL,  "CFG_MENU_IDX_DISPLAY_BACKLIGHT_HIGH_LEVEL",  12};
    FlashParamNs::Parameter<uint32_t>    P_CFG_MENU_IDX_DISPLAY_TIME_TO_BACKLIGHT_LOW{CFG_MENU_IDX_DISPLAY_TIME_TO_BACKLIGHT_LOW, "CFG_MENU_IDX_DISPLAY_TIME_TO_BACKLIGHT_LOW", 1};
    FlashParamNs::Parameter<uint32_t>    P_CFG_MENU_IDX_PLAY_TIME_TO_NEXT_PLAY       {CFG_MENU_IDX_PLAY_TIME_TO_NEXT_PLAY,        "CFG_MENU_IDX_PLAY_TIME_TO_NEXT_PLAY",        2};
    FlashParamNs::Parameter<uint32_t>    P_CFG_MENU_IDX_PLAY_NEXT_PLAY_ALBUM         {CFG_MENU_IDX_PLAY_NEXT_PLAY_ALBUM,          "CFG_MENU_IDX_PLAY_NEXT_PLAY_ALBUM",          1};
    FlashParamNs::Parameter<uint32_t>    P_CFG_MENU_IDX_PLAY_RANDOM_DIR_DEPTH        {CFG_MENU_IDX_PLAY_RANDOM_DIR_DEPTH,         "CFG_MENU_IDX_PLAY_RANDOM_DIR_DEPTH",         1};

    void initialize(bool preserveStoreCount = false) override {
        FlashParamNs::FlashParam::initialize();
        P_CFG_REVISION.setDefault();
    }
};
