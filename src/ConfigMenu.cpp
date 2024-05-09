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

uint32_t ConfigMenu::get(const Item_t& item) const
{
    auto sel_idx = cfgParam.getValue<uint32_t>(item.paramID);
    return item.selection->at(sel_idx).value;
}

uint32_t ConfigMenu::get(const ConfigMenuId& id) const
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

int ConfigMenu::getNum() const
{
    switch (level) {
        case 0: // Top
            return static_cast<int>(CategoryId_t::__NUM_CATEGORY_ID__);
            break;
        case 1: // Category
            return menuMapByCategory.at(curCategoryId).size();
            break;
        case 2: // Item
            return curItem->selection->size();
            break;
        default:
            break;
    }
    return 0;
}

const char* ConfigMenu::getStr(const uint32_t& idx) const
{
    switch (level) {
        case 0: {  // Top
            return categoryMap.at(static_cast<CategoryId_t>(idx));
            break;
        }
        case 1: {  // Category
            auto it = menuMapByCategory.at(curCategoryId).begin();
            if (idx < menuMapByCategory.at(curCategoryId).size()) {
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

bool ConfigMenu::enter(const uint32_t& idx)
{
    bool flag = false;
    switch (level) {
        case 0: // Top
            curCategoryId = static_cast<CategoryId_t>(idx);
            level++;
            flag = true;
            break;
        case 1: { // Category
            auto it = menuMapByCategory.at(curCategoryId).begin();
            if (idx < menuMapByCategory.at(curCategoryId).size()) {
                curItem = std::next(it, idx)->second;
                level++;
                flag = true;
            }
	    break;
        }
        case 2: { // Item
                cfgParam.setValue<uint32_t>(curItem->paramID, idx);
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

bool ConfigMenu::isSelection() const
{
    return (level == 2);
}

bool ConfigMenu::selIdxMatched(const uint32_t& idx) const
{
    if (!isSelection()) { return false; }
    uint32_t sel_idx = cfgParam.getValue<uint32_t>(curItem->paramID);
    return ((int) sel_idx == idx);
}
