/*-----------------------------------------------------------/
/ ConfigMenu.h
/------------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/-----------------------------------------------------------*/

#pragma once

#include "ConfigParam.h"

#include <map>
#include <vector>

enum class ConfigMenuId: uint32_t {
    GENERAL_TIME_TO_POWER_OFF = 0,
    GENERAL_TIME_TO_LEAVE_CONFIG,
    DISPLAY_LCD_CONFIG,
    DISPLAY_ROTATION,
    DISPLAY_BACKLIGHT_LOW_LEVEL,
    DISPLAY_BACKLIGHT_HIGH_LEVEL,
    DISPLAY_TIME_TO_BACKLIGHT_LOW,
    PLAY_TIME_TO_NEXT_PLAY,
    PLAY_NEXT_PLAY_ALBUM,
    PLAY_RANDOM_DIR_DEPTH,
};

//=================================
// Interface of Hook Functions
//=================================
void hookDispLcdConfig();
void hookDispRotation();

//=================================
// Interface of ConfigMenu class
//=================================
class ConfigMenu
{
public:
    enum class CategoryId_t: uint32_t {
        GENERAL = 0,
        DISPLAY,
        PLAY,
        __NUM_CATEGORY_ID__
    };

    typedef enum {
        Stop = 0,
        Sequential,
        SequentialRepeat,
        Repeat,
        Random
    } NextPlayAction_t;

    typedef struct {
        const char* name;
        int         value;
    } ConfigSel_t;

    typedef struct {
        const char*               name;
        CategoryId_t              category_id;
        ConfigParam::ParamId_t    paramID;
        std::vector<ConfigSel_t>* selection;
        void                      (*hook_func)();
    } Item_t;

    static ConfigMenu& instance(); // Singleton

    // For Read Users
    uint32_t get(const ConfigMenuId& id) const;
    void scanHookFunc();

    // Menu Utilities
    int getNum() const;
    const char* getStr(const uint32_t& idx) const;
    bool enter(const uint32_t& idx);
    bool leave();
    bool isSelection() const;
    bool selIdxMatched(const uint32_t& idx) const;

private:
    ConfigMenu();
    ~ConfigMenu() = default;
    ConfigMenu(const ConfigMenu&) = delete;
    ConfigMenu& operator=(const ConfigMenu&) = delete;
    uint32_t get(const Item_t& item) const;

    std::vector<ConfigSel_t> selTime0 = {
        {"3 min", 3*60},
        {"5 min", 5*60},
        {"10 min", 10*60},
    };

    std::vector<ConfigSel_t> selTime1 = {
        {"10 sec", 10},
        {"20 sec", 20},
        {"30 sec", 30},
        {"1 min", 1*60},
        {"3 min", 3*60},
        {"5 min", 5*60},
        {"Forever", -1},
    };
    std::vector<ConfigSel_t> selTime2 = {
        {"15 sec", 15},
        {"30 sec", 30},
        {"1 min", 1*60},
        {"2 min", 2*60},
        {"3 min", 3*60},
    };
    std::vector<ConfigSel_t> selLcdConfig = {
        {"0", 0},
        {"1", 1},
        {"2", 2},
    };
    std::vector<ConfigSel_t> selRotation = {
        {"0 deg", 0},
        {"180 deg", 1},
    };
    std::vector<ConfigSel_t> selBacklightLevel = {
        {"16", 16},
        {"32", 32},
        {"48", 48},
        {"64", 64},
        {"80", 80},
        {"96", 96},
        {"112", 112},
        {"128", 128},
        {"144", 144},
        {"160", 160},
        {"176", 176},
        {"192", 192},
        {"208", 208},
        {"224", 224},
        {"240", 240},
        {"255", 255},
    };
    std::vector<ConfigSel_t> selNextPlayAlbum = {
        {"Stop", Stop},
        {"Sequential", Sequential},
        {"SequentialRepeat", SequentialRepeat},
        {"Repeat", Repeat},
        {"Random", Random},
    };
    std::vector<ConfigSel_t> selRandDirDepth = {
        {"1", 1},
        {"2", 2},
        {"3", 3},
        {"4", 4},
    };

    std::map<CategoryId_t, const char*> categoryMap = {
        {CategoryId_t::GENERAL, "General"},
        {CategoryId_t::DISPLAY, "Display"},
        {CategoryId_t::PLAY,    "Play"},
    };

    std::map<const ConfigMenuId, Item_t> menuMap = {
        //                                            Name                      category               ConfigParam::ParamId_t                                   selection           hook_func
        {ConfigMenuId::GENERAL_TIME_TO_POWER_OFF,     {"Time to Power Off",     CategoryId_t::GENERAL, ConfigParam::CFG_MENU_IDX_GENERAL_TIME_TO_POWER_OFF,     &selTime0,          nullptr}},
        {ConfigMenuId::GENERAL_TIME_TO_LEAVE_CONFIG,  {"Time to Leave Config",  CategoryId_t::GENERAL, ConfigParam::CFG_MENU_IDX_GENERAL_TIME_TO_LEAVE_CONFIG,  &selTime2,          nullptr}},
        {ConfigMenuId::DISPLAY_LCD_CONFIG,            {"LCD Config",            CategoryId_t::DISPLAY, ConfigParam::CFG_MENU_IDX_DISPLAY_LCD_CONFIG,            &selLcdConfig,      hookDispLcdConfig}},
        {ConfigMenuId::DISPLAY_ROTATION,              {"Rotation",              CategoryId_t::DISPLAY, ConfigParam::CFG_MENU_IDX_DISPLAY_ROTATION,              &selRotation,       hookDispRotation}},
        {ConfigMenuId::DISPLAY_BACKLIGHT_LOW_LEVEL,   {"Backlight Low Level",   CategoryId_t::DISPLAY, ConfigParam::CFG_MENU_IDX_DISPLAY_BACKLIGHT_LOW_LEVEL,   &selBacklightLevel, nullptr}},
        {ConfigMenuId::DISPLAY_BACKLIGHT_HIGH_LEVEL,  {"Backlight High Level",  CategoryId_t::DISPLAY, ConfigParam::CFG_MENU_IDX_DISPLAY_BACKLIGHT_HIGH_LEVEL,  &selBacklightLevel, nullptr}},
        {ConfigMenuId::DISPLAY_TIME_TO_BACKLIGHT_LOW, {"Time to Backlight Low", CategoryId_t::DISPLAY, ConfigParam::CFG_MENU_IDX_DISPLAY_TIME_TO_BACKLIGHT_LOW, &selTime1,          nullptr}},
        {ConfigMenuId::PLAY_TIME_TO_NEXT_PLAY,        {"Time to Next Play",     CategoryId_t::PLAY,    ConfigParam::CFG_MENU_IDX_PLAY_TIME_TO_NEXT_PLAY,        &selTime2,          nullptr}},
        {ConfigMenuId::PLAY_NEXT_PLAY_ALBUM,          {"Next Play Album",       CategoryId_t::PLAY,    ConfigParam::CFG_MENU_IDX_PLAY_NEXT_PLAY_ALBUM,          &selNextPlayAlbum,  nullptr}},
        {ConfigMenuId::PLAY_RANDOM_DIR_DEPTH,         {"Random Dir Depth",      CategoryId_t::PLAY,    ConfigParam::CFG_MENU_IDX_PLAY_RANDOM_DIR_DEPTH,         &selRandDirDepth,   nullptr}},
    };

    std::map<const CategoryId_t, std::map<const ConfigMenuId, Item_t*>> menuMapByCategory;

    int level = 0; // 0: Top, 1: Category, 2: Item
    CategoryId_t curCategoryId;
    Item_t* curItem;
    ConfigParam& cfgParam = ConfigParam::instance();
};
