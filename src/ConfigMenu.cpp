/*-----------------------------------------------------------/
/ ConfigMenu.cpp
/------------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/-----------------------------------------------------------*/

#include "LcdCanvas.h"
#include "ConfigMenu.h"

//=================================
// Implementation of Hook Functions
//=================================
void hook_disp_lcd_config()
{
    LcdCanvas::configureLcd(GET_CFG_MENU_DISPLAY_LCD_CONFIG);
    lcd.setRotation(GET_CFG_MENU_DISPLAY_ROTATION);
    lcd.switchToListView();
}

void hook_disp_rotation()
{
    lcd.setRotation(GET_CFG_MENU_DISPLAY_ROTATION);
    lcd.switchToListView();
}

//=================================
// Implementation of ConfigMenu class
//=================================

ConfigMenu& ConfigMenu::instance()
{
    static ConfigMenu _instance; // Singleton
    return _instance;
}

ConfigMenu::ConfigMenu() : level(0)
{
}

ConfigMenu::~ConfigMenu()
{

}

uint32_t ConfigMenu::getValue(int category_idx, int item_idx)
{
    if (category_idx >= sz_category) { return 0; }
    if (item_idx >= category[category_idx].num_items) { return 0; }
    config_menu_item_t *item = &category[category_idx].items[item_idx];
    uint32_t sel_idx;
    configParam.read(item->paramID, &sel_idx);
    return item->selection[sel_idx].value;
}

void ConfigMenu::scanHookFunc()
{
    int adrs = 0;
    for (int i = 0; i < sz_category; i++) {
        for (int j = 0; j < category[i].num_items; j++) {
            config_menu_item_t *item = &category[i].items[j];
            if (item->hook_func != nullptr) {
                item->hook_func();
            }
        }
    }
}

/*
int ConfigMenu::getSelIdx(int category_idx, int item_idx)
{
    if (category_idx >= sz_category) { return 0; }
    if (item_idx >= category[category_idx].num_items) { return 0; }
    config_menu_item_t *item = &category[category_idx].items[item_idx];
    return item->sel_idx;
}

void ConfigMenu::setSelIdx(int category_idx, int item_idx, int sel_idx)
{
    if (category_idx >= sz_category) { return; }
    if (item_idx >= category[category_idx].num_items) { return; }
    if (sel_idx >= category[category_idx].items[item_idx].num_selections) { return; }
    config_menu_item_t *item = &category[category_idx].items[item_idx];
    item->sel_idx = sel_idx;
}

void ConfigMenu::scanSelIdx(void (*yield_func)(ConfigParam::ParamID_t paramID, uint32_t *val), bool do_hook_func)
{
    int adrs = 0;
    for (int i = 0; i < sz_category; i++) {
        for (int j = 0; j < category[i].num_items; j++) {
            config_menu_item_t *item = &category[i].items[j];
            yield_func(item->paramID, &item->sel_idx);
            if (do_hook_func && item->hook_func != nullptr) {
                item->hook_func();
            }
        }
    }

}
*/

int ConfigMenu::getNum()
{
    switch (level) {
        case 0: // Top
            return sz_category;
            break;
        case 1: // Category
            return cur_category->num_items;
            break;
        case 2: // Item
            return cur_item->num_selections;
            break;
        default:
            break;
    }
    return 0;
}

const char *ConfigMenu::getStr(int idx)
{
    if (idx >= getNum()) { return ""; }
    switch (level) {
        case 0: // Top
            return category[idx].name;
            break;
        case 1: // Category
            return cur_category->items[idx].name;
            break;
        case 2: // Item
            return cur_item->selection[idx].name;
            break;
        default:
            break;
    }
    return "";
}

bool ConfigMenu::enter(int idx)
{
    bool flag = false;
    switch (level) {
        case 0: // Top
            cur_category = &category[idx];
            level++;
            flag = true;
            break;
        case 1: // Category
            cur_item = &cur_category->items[idx];
            level++;
            flag = true;
            break;
        case 2: { // Item
                uint32_t sel_idx = idx;
                configParam.write(cur_item->paramID, &sel_idx);
                // Do hook_func() when value is set
                if (cur_item->hook_func != nullptr) {
                    cur_item->hook_func();
                }
            }
            break;
        default:
            break;
    }
    return flag;
}

bool ConfigMenu::leave()
{
    bool flag = false;
    switch (level) {
        case 0: // Top
            break;
        case 1: // Category
            level--;
            flag = true;
            break;
        case 2: // Item
            level--;
            flag = true;
            break;
        default:
            break;
    }
    return flag;
}

bool ConfigMenu::isSelection()
{
    return (level == 2);
}

bool ConfigMenu::selIdxMatched(int idx)
{
    if (!isSelection()) { return false; }
    uint32_t sel_idx;
    configParam.read(cur_item->paramID, &sel_idx);
    return ((int) sel_idx == idx);
}
