/*-----------------------------------------------------------/
/ ConfigMenu.cpp
/------------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/-----------------------------------------------------------*/

#include "ConfigMenu.h"

#include "LcdCanvas.h"
#include "ui_control.h"

//=================================
// Implementation of Hook Functions
//=================================
void hookDispLcdConfig()
{
    ConfigMenu& cfg = ConfigMenu::instance();
    LcdCanvas::configureLcd(ui_get_board_type(), cfg.get(ConfigMenuId::DISPLAY_LCD_CONFIG));
    lcd.setRotation(cfg.get(ConfigMenuId::DISPLAY_ROTATION));
    lcd.switchToListView();
}

void hookDispRotation()
{
    ConfigMenu& cfg = ConfigMenu::instance();
    lcd.setRotation(cfg.get(ConfigMenuId::DISPLAY_ROTATION));
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

ConfigMenu::ConfigMenu()
{
    // generate menuMap by category
    for (auto& [id, item] : menuMap) {
        menuMapByCategory[item.category_id][id] = &item;
    }
}

uint32_t ConfigMenu::get(Item_t& item)
{
    uint32_t sel_idx;
    configParam.read(item.paramID, &sel_idx);
    return item.selection->at(sel_idx).value;
}

uint32_t ConfigMenu::get(const ConfigMenuId& id)
{
    return get(menuMap.at(id));
}

void ConfigMenu::scanHookFunc()
{
    for (const auto& [id, item] : menuMap) {
        if (item.hook_func != nullptr) {
            item.hook_func();
        }
    }
}

int ConfigMenu::getNum()
{
    switch (level) {
        case 0: // Top
            return static_cast<int>(CategoryId_t::__NUM_CATEGORY_ID__);
            break;
        case 1: // Category
            return menuMapByCategory[curCategoryId].size();
            break;
        case 2: // Item
            return curItem->selection->size();
            break;
        default:
            break;
    }
    return 0;
}

const char* ConfigMenu::getStr(int idx)
{
    switch (level) {
        case 0: {  // Top
            return categoryMap.at(static_cast<CategoryId_t>(idx));
            break;
        }
        case 1: {  // Category
            auto it = menuMapByCategory[curCategoryId].begin();
            if (idx < menuMapByCategory[curCategoryId].size()) {
                return std::next(it, idx)->second->name;
            }
            break;
        }
        case 2: // Item
            return curItem->selection->at(idx).name;
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
            curCategoryId = static_cast<CategoryId_t>(idx);
            level++;
            flag = true;
            break;
        case 1: { // Category
            auto it = menuMapByCategory[curCategoryId].begin();
            if (idx < menuMapByCategory[curCategoryId].size()) {
                curItem = std::next(it, idx)->second;
                level++;
                flag = true;
            }
	    break;
        }
        case 2: { // Item
                uint32_t sel_idx = idx;
                configParam.write(curItem->paramID, &sel_idx);
                // Do hook_func() when value is set
                if (curItem->hook_func != nullptr) {
                    curItem->hook_func();
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
    configParam.read(curItem->paramID, &sel_idx);
    return ((int) sel_idx == idx);
}
