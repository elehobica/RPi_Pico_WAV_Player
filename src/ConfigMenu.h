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
        ConfigParam::ParamID_t    paramID;
        std::vector<ConfigSel_t>* selection;
        void                      (*hook_func)();
    } Item_t;

    static ConfigMenu& instance(); // Singleton

    // For Read Users
    uint32_t get(const ConfigMenuId& id);
    void scanHookFunc();

    // Menu Utilities
    int getNum();
    const char* getStr(int idx);
    bool enter(int idx);
    bool leave();
    bool isSelection();
    bool selIdxMatched(int idx);

private:
    ConfigMenu();
    ~ConfigMenu() = default;
    ConfigMenu(const ConfigMenu&) = delete;
    ConfigMenu& operator=(const ConfigMenu&) = delete;
    uint32_t get(Item_t& item);

    std::vector<ConfigSel_t> selTime0 = {
        ConfigSel_t{"3 min", 3*60},
        ConfigSel_t{"5 min", 5*60},
        ConfigSel_t{"10 min", 10*60},
    };

    std::vector<ConfigSel_t> selTime1 = {
        ConfigSel_t{"10 sec", 10},
        ConfigSel_t{"20 sec", 20},
        ConfigSel_t{"30 sec", 30},
        ConfigSel_t{"1 min", 1*60},
        ConfigSel_t{"3 min", 3*60},
        ConfigSel_t{"5 min", 5*60},
        ConfigSel_t{"Forever", -1},
    };
    std::vector<ConfigSel_t> selTime2 = {
        ConfigSel_t{"15 sec", 15},
        ConfigSel_t{"30 sec", 30},
        ConfigSel_t{"1 min", 1*60},
        ConfigSel_t{"2 min", 2*60},
        ConfigSel_t{"3 min", 3*60},
    };
    std::vector<ConfigSel_t> selLcdConfig = {
        ConfigSel_t{"0", 0},
        ConfigSel_t{"1", 1},
        ConfigSel_t{"2", 2},
    };
    std::vector<ConfigSel_t> selRotation = {
        ConfigSel_t{"0 deg", 0},
        ConfigSel_t{"180 deg", 1},
    };
    std::vector<ConfigSel_t> selBacklightLevel = {
        ConfigSel_t{"16", 16},
        ConfigSel_t{"32", 32},
        ConfigSel_t{"48", 48},
        ConfigSel_t{"64", 64},
        ConfigSel_t{"80", 80},
        ConfigSel_t{"96", 96},
        ConfigSel_t{"112", 112},
        ConfigSel_t{"128", 128},
        ConfigSel_t{"144", 144},
        ConfigSel_t{"160", 160},
        ConfigSel_t{"176", 176},
        ConfigSel_t{"192", 192},
        ConfigSel_t{"208", 208},
        ConfigSel_t{"224", 224},
        ConfigSel_t{"240", 240},
        ConfigSel_t{"255", 255},
    };
    std::vector<ConfigSel_t> selNextPlayAlbum = {
        ConfigSel_t{"Stop", Stop},
        ConfigSel_t{"Sequential", Sequential},
        ConfigSel_t{"SequentialRepeat", SequentialRepeat},
        ConfigSel_t{"Repeat", Repeat},
        ConfigSel_t{"Random", Random},
    };
    std::vector<ConfigSel_t> selRandDirDepth = {
        ConfigSel_t{"1", 1},
        ConfigSel_t{"2", 2},
        ConfigSel_t{"3", 3},
        ConfigSel_t{"4", 4},
    };

    std::map<CategoryId_t, const char*> categoryMap = {
        {CategoryId_t::GENERAL, "General"},
        {CategoryId_t::DISPLAY, "Display"},
        {CategoryId_t::PLAY,    "Play"},
    };

    std::map<const ConfigMenuId, Item_t> menuMap = {
        //                                            Name                      category               ConfigParam::paramID_t                                   selection           hook_func
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
};
