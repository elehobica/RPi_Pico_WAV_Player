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
// Interface of ConfigParam class
//=================================
struct ConfigParam : FlashParamNs::FlashParam {
    static ConfigParam& instance();  // Singleton
    uint32_t getBootCountFromFlash();
    void incBootCount();
    void initialize();

    // Parameter<T>     inst                                         id                                          name                                          addr   default
    FlashParamNs::Parameter<uint32_t> P_CFG_BOOT_COUNT                            {CFG_BOOT_COUNT,                             "CFG_BOOT_COUNT",                             0x000, 10};
    FlashParamNs::Parameter<uint32_t> P_CFG_FORMAT_REV                            {CFG_FORMAT_REV,                             "CFG_FORMAT_REV",                             0x004, 20240513};  // update value when updated to reset user flash
    FlashParamNs::Parameter<uint32_t> P_CFG_SEED                                  {CFG_SEED,                                   "CFG_SEED",                                   0x008, 0};
    FlashParamNs::Parameter<uint8_t>  P_CFG_VOLUME                                {CFG_VOLUME,                                 "CFG_VOLUME",                                 0x00c, 65};
    FlashParamNs::Parameter<uint8_t>  P_CFG_STACK_COUNT                           {CFG_STACK_COUNT,                            "CFG_STACK_COUNT",                            0x00d, 0};
    FlashParamNs::Parameter<uint16_t> P_CFG_STACK_HEAD0                           {CFG_STACK_HEAD0,                            "CFG_STACK_HEAD0",                            0x010, 0};
    FlashParamNs::Parameter<uint16_t> P_CFG_STACK_COLUMN0                         {CFG_STACK_COLUMN0,                          "CFG_STACK_COLUMN0",                          0x012, 0};
    FlashParamNs::Parameter<uint16_t> P_CFG_STACK_HEAD1                           {CFG_STACK_HEAD1,                            "CFG_STACK_HEAD1",                            0x014, 0};
    FlashParamNs::Parameter<uint16_t> P_CFG_STACK_COLUMN1                         {CFG_STACK_COLUMN1,                          "CFG_STACK_COLUMN1",                          0x016, 0};
    FlashParamNs::Parameter<uint16_t> P_CFG_STACK_HEAD2                           {CFG_STACK_HEAD2,                            "CFG_STACK_HEAD2",                            0x018, 0};
    FlashParamNs::Parameter<uint16_t> P_CFG_STACK_COLUMN2                         {CFG_STACK_COLUMN2,                          "CFG_STACK_COLUMN2",                          0x01a, 0};
    FlashParamNs::Parameter<uint16_t> P_CFG_STACK_HEAD3                           {CFG_STACK_HEAD3,                            "CFG_STACK_HEAD3",                            0x01c, 0};
    FlashParamNs::Parameter<uint16_t> P_CFG_STACK_COLUMN3                         {CFG_STACK_COLUMN3,                          "CFG_STACK_COLUMN3",                          0x020, 0};
    FlashParamNs::Parameter<uint16_t> P_CFG_STACK_HEAD4                           {CFG_STACK_HEAD4,                            "CFG_STACK_HEAD4",                            0x022, 0};
    FlashParamNs::Parameter<uint16_t> P_CFG_STACK_COLUMN4                         {CFG_STACK_COLUMN4,                          "CFG_STACK_COLUMN4",                          0x024, 0};
    FlashParamNs::Parameter<uint32_t> P_CFG_UIMODE                                {CFG_UIMODE,                                 "CFG_UIMODE",                                 0x028, 0};
    FlashParamNs::Parameter<uint16_t> P_CFG_IDX_HEAD                              {CFG_IDX_HEAD,                               "CFG_IDX_HEAD",                               0x02c, 0};
    FlashParamNs::Parameter<uint16_t> P_CFG_IDX_COLUMN                            {CFG_IDX_COLUMN,                             "CFG_IDX_COLUMN",                             0x02e, 0};
    FlashParamNs::Parameter<uint16_t> P_CFG_IDX_PLAY                              {CFG_IDX_PLAY,                               "CFG_IDX_PLAY",                               0x030, 0};
    FlashParamNs::Parameter<uint64_t> P_CFG_PLAY_POS                              {CFG_PLAY_POS,                               "CFG_PLAY_POS",                               0x038, 0};
    FlashParamNs::Parameter<uint32_t> P_CFG_SAMPLES_PLAYED                        {CFG_SAMPLES_PLAYED,                         "CFG_SAMPLES_PLAYED",                         0x040, 0};
    // type of CFG_MENU_xxx must be uint32_t and default values indidicates index of selection (see ConfigMenu.h)
    FlashParamNs::Parameter<uint32_t> P_CFG_MENU_IDX_GENERAL_TIME_TO_POWER_OFF    {CFG_MENU_IDX_GENERAL_TIME_TO_POWER_OFF,     "CFG_MENU_IDX_GENERAL_TIME_TO_POWER_OFF",     0x080, 0};
    FlashParamNs::Parameter<uint32_t> P_CFG_MENU_IDX_GENERAL_TIME_TO_LEAVE_CONFIG {CFG_MENU_IDX_GENERAL_TIME_TO_LEAVE_CONFIG,  "CFG_MENU_IDX_GENERAL_TIME_TO_LEAVE_CONFIG",  0x084, 1};
    FlashParamNs::Parameter<uint32_t> P_CFG_MENU_IDX_GENERAL_PUSH_BUTTON_LAYOUT   {CFG_MENU_IDX_GENERAL_PUSH_BUTTON_LAYOUT,    "CFG_MENU_IDX_GENERAL_PUSH_BUTTON_LAYOUT",    0x088, 0};
    FlashParamNs::Parameter<uint32_t> P_CFG_MENU_IDX_GENERAL_HP_BUTTON_LAYOUT     {CFG_MENU_IDX_GENERAL_HP_BUTTON_LAYOUT,      "CFG_MENU_IDX_GENERAL_HP_BUTTON_LAYOUT",      0x08c, 1};
    FlashParamNs::Parameter<uint32_t> P_CFG_MENU_IDX_DISPLAY_LCD_CONFIG           {CFG_MENU_IDX_DISPLAY_LCD_CONFIG,            "CFG_MENU_IDX_DISPLAY_LCD_CONFIG",            0x090, 0};
    FlashParamNs::Parameter<uint32_t> P_CFG_MENU_IDX_DISPLAY_ROTATION             {CFG_MENU_IDX_DISPLAY_ROTATION,              "CFG_MENU_IDX_DISPLAY_ROTATION",              0x094, 0};
    FlashParamNs::Parameter<uint32_t> P_CFG_MENU_IDX_DISPLAY_BACKLIGHT_LOW_LEVEL  {CFG_MENU_IDX_DISPLAY_BACKLIGHT_LOW_LEVEL,   "CFG_MENU_IDX_DISPLAY_BACKLIGHT_LOW_LEVEL",   0x098, 7};
    FlashParamNs::Parameter<uint32_t> P_CFG_MENU_IDX_DISPLAY_BACKLIGHT_HIGH_LEVEL {CFG_MENU_IDX_DISPLAY_BACKLIGHT_HIGH_LEVEL,  "CFG_MENU_IDX_DISPLAY_BACKLIGHT_HIGH_LEVEL",  0x09c, 12};
    FlashParamNs::Parameter<uint32_t> P_CFG_MENU_IDX_DISPLAY_TIME_TO_BACKLIGHT_LOW{CFG_MENU_IDX_DISPLAY_TIME_TO_BACKLIGHT_LOW, "CFG_MENU_IDX_DISPLAY_TIME_TO_BACKLIGHT_LOW", 0x0a0, 1};
    FlashParamNs::Parameter<uint32_t> P_CFG_MENU_IDX_PLAY_TIME_TO_NEXT_PLAY       {CFG_MENU_IDX_PLAY_TIME_TO_NEXT_PLAY,        "CFG_MENU_IDX_PLAY_TIME_TO_NEXT_PLAY",        0x0a4, 2};
    FlashParamNs::Parameter<uint32_t> P_CFG_MENU_IDX_PLAY_NEXT_PLAY_ALBUM         {CFG_MENU_IDX_PLAY_NEXT_PLAY_ALBUM,          "CFG_MENU_IDX_PLAY_NEXT_PLAY_ALBUM",          0x0a8, 1};
    FlashParamNs::Parameter<uint32_t> P_CFG_MENU_IDX_PLAY_RANDOM_DIR_DEPTH        {CFG_MENU_IDX_PLAY_RANDOM_DIR_DEPTH,         "CFG_MENU_IDX_PLAY_RANDOM_DIR_DEPTH",         0x0ac, 1};
};
