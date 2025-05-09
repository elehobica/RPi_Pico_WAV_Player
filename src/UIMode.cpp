/*------------------------------------------------------/
/ UIMode
/-------------------------------------------------------/
/ Copyright (c) 2020-2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include "UIMode.h"

#include <cstdio>
#include <cstring>

#include "pico/stdlib.h"

#include "audio_codec.h"
#include "file_menu_FatFs.h"
#include "power_manage.h"
#include "TagRead.h"
#include "tf_card.h"

// ENABLE_REBOOT_AFTER_WAKEUP:
// reboot allows stdio_usb (USB CDC) and Serial terminal to be re-activated
// otherwise, USB cable needs to be reconnected
#define ENABLE_REBOOT_AFTER_WAKEUP

TagRead tag;

// UIMode class instances
button_action_t UIMode::btn_act;
button_unit_t UIMode::btn_unit;
UIVars* UIMode::vars;
std::stack<stack_data_t> UIMode::dir_stack;
UIMode::ExitType UIMode::exitType = UIMode::NoError;
ConfigMenu& UIMode::cfgMenu = ConfigMenu::instance();
ConfigParam& UIMode::cfgParam = ConfigParam::instance();
LcdCanvas* UIMode::lcd = nullptr;  // dynamic instance generation after configureLcd() is needed
std::array<UIMode*, NUM_UI_MODES> UIMode::ui_mode_ary;

//================================
// Implementation of UIMode class
//================================
/*static*/
void UIMode::initialize(UIVars* vars)
{
    UIMode::vars = vars;
    ui_mode_ary[InitialMode]  = (UIMode*) new UIInitialMode();
    ui_mode_ary[ChargeMode]   = (UIMode*) new UIChargeMode();
    ui_mode_ary[OpeningMode]  = (UIMode*) new UIOpeningMode();
    ui_mode_ary[FileViewMode] = (UIMode*) new UIFileViewMode();
    ui_mode_ary[PlayMode]     = (UIMode*) new UIPlayMode();
    ui_mode_ary[ConfigMode]   = (UIMode*) new UIConfigMode();
    ui_mode_ary[PowerOffMode] = (UIMode*) new UIPowerOffMode();
    lcd = &LcdCanvas::instance();
}

UIMode* UIMode::getUIMode(const ui_mode_enm_t& ui_mode_enm)
{
    return ui_mode_ary.at(ui_mode_enm);
}

UIMode::UIMode(const char* name, const ui_mode_enm_t& ui_mode_enm)
    : name(name), ui_mode_enm(ui_mode_enm)
{
}

void UIMode::entry(UIMode* prevMode)
{
    this->prevMode = prevMode;
    idle_count = 0;
    if (ui_mode_enm == vars->init_dest_ui_mode) { // Reached desitination of initial UI mode
        vars->init_dest_ui_mode = InitialMode;
    }
    ui_clear_btn_evt();
}

bool UIMode::isAudioFile(const uint16_t& idx) const
{
    if (file_menu_match_ext(idx, "wav", 3) ||  file_menu_match_ext(idx, "WAV", 3)) {
        set_audio_codec(PlayAudio::AUDIO_CODEC_WAV);
        return true;
    }
    set_audio_codec(PlayAudio::AUDIO_CODEC_NONE);
    return false;
}

const char* UIMode::getName() const
{
    return name;
}

ui_mode_enm_t UIMode::getUIModeEnm() const
{
    return ui_mode_enm;
}

uint16_t UIMode::getIdleCount() const
{
    return idle_count;
}

//=======================================
// Implementation of UIInitialMode class
//=======================================
UIInitialMode::UIInitialMode() : UIMode("UIInitialMode", InitialMode)
{
}

UIMode* UIInitialMode::update()
{
    ui_get_btn_evt(btn_act, btn_unit); // Ignore button event
    // Always transfer to ChargeMode or OpeningMode
    if (pm_usb_power_detected() && !pm_is_caused_reboot()) {
        return getUIMode(ChargeMode);
    } else {
        return getUIMode(OpeningMode);
    }
    idle_count++;
    return this;
}

void UIInitialMode::entry(UIMode* prevMode)
{
    UIMode::entry(prevMode);
    loadFromFlash();
}

void UIInitialMode::draw() const
{
    ui_clear_btn_evt();
}

void UIInitialMode::loadFromFlash() const
{
    // Load Configuration parameters from Flash
    cfgParam.initialize();
    printf("Raspberry Pi Pico Player: %s\r\n", cfgParam.P_CFG_VERSION.get().c_str());
    cfgMenu.scanHookFunc();
}

//=======================================
// Implementation of UIChargeMode class
//=======================================
UIChargeMode::UIChargeMode() : UIMode("UIChargeMode", ChargeMode)
{
}

UIMode* UIChargeMode::update()
{
    if (ui_get_btn_evt(btn_act, btn_unit)) {
        switch (btn_act) {
            case button_action_t::CenterSingle:
                return getUIMode(OpeningMode);
            default:
                break;
        }
        idle_count = 0;
    }
    if (idle_count >= 2 * OneSec) {
        lcd->setMsg("");
        lcd->clear(true);
        pm_enter_dormant_and_wake();
        #ifdef ENABLE_REBOOT_AFTER_WAKEUP
        pm_reboot(); // not go to below
        #endif // ENABLE_REBOOT_AFTER_WAKEUP
        return getUIMode(OpeningMode);
    }
    idle_count++;
    return this;
}

void UIChargeMode::entry(UIMode* prevMode)
{
    UIMode::entry(prevMode);
    lcd->setMsg("Charging", true);
    pm_enable_button_control(true);  // for wake up
    pm_set_power_keep(false);
}

void UIChargeMode::draw() const
{
    lcd->drawPowerOff();
    ui_clear_btn_evt();
}

//=======================================
// Implementation of UIOpeningMode class
//=======================================
UIOpeningMode::UIOpeningMode() : UIMode("UIOpeningMode", OpeningMode)
{
}

void UIOpeningMode::restoreFromFlash() const
{
    // Load Configuration parameters from Flash
    cfgParam.printInfo();
    //cfgMenu.printInfo();

    // Restore from cfgParam to user parameters
    srand(cfgParam.P_CFG_SEED.get());
    PlayAudio::setVolume(cfgParam.P_CFG_VOLUME.get());
    bool err_flg = false;
    for (int i = cfgParam.P_CFG_STACK_COUNT.get() - 1; i >= 0; i--) {
        stack_data_t item;
        const auto& head_param = (i == 4) ? cfgParam.P_CFG_STACK_HEAD4 : (i == 3) ? cfgParam.P_CFG_STACK_HEAD3 : (i == 2) ? cfgParam.P_CFG_STACK_HEAD2 : (i == 1) ? cfgParam.P_CFG_STACK_HEAD1 : cfgParam.P_CFG_STACK_HEAD0;
        const auto& column_param = (i == 4) ? cfgParam.P_CFG_STACK_COLUMN4 : (i == 3) ? cfgParam.P_CFG_STACK_COLUMN3 : (i == 2) ? cfgParam.P_CFG_STACK_COLUMN2 : (i == 1) ? cfgParam.P_CFG_STACK_COLUMN1 : cfgParam.P_CFG_STACK_COLUMN0;
        item.head = head_param.get();
        item.column = column_param.get();
        if (item.head+item.column >= file_menu_get_num()) { err_flg = true; break; } // idx overflow
        file_menu_sort_entry(item.head+item.column, item.head+item.column + 1);
        if (file_menu_is_dir(item.head+item.column) <= 0 || item.head+item.column == 0) { err_flg = true; break; } // Not Directory or Parent Directory
        dir_stack.push(item);
        file_menu_ch_dir(item.head+item.column);
    }

    vars->init_dest_ui_mode = static_cast<ui_mode_enm_t>(cfgParam.P_CFG_UIMODE.get());

    uint16_t idx_head = cfgParam.P_CFG_IDX_HEAD.get();
    uint16_t idx_column = cfgParam.P_CFG_IDX_COLUMN.get();
    if (idx_head+idx_column >= file_menu_get_num()) { err_flg = true; } // idx overflow
    if (err_flg) { // Load Error
        printf("dir_stack Load Error. root directory is set\r\n");
        while (dir_stack.size()) { dir_stack.pop(); }
        file_menu_open_dir("/");
        idx_head = idx_column = 0;
        vars->init_dest_ui_mode = FileViewMode;
    }
    vars->idx_head = idx_head;
    vars->idx_column = idx_column;

    vars->idx_play = 0;
    vars->fpos = 0;
    vars->samples_played = 0;
    if (vars->init_dest_ui_mode == PlayMode) {
        vars->idx_play = cfgParam.P_CFG_IDX_PLAY.get();
        uint64_t play_pos = cfgParam.P_CFG_PLAY_POS.get();
        vars->fpos = (size_t) play_pos;
        vars->samples_played = cfgParam.P_CFG_SAMPLES_PLAYED.get();
    }
}

UIMode* UIOpeningMode::update()
{
    ui_get_btn_evt(btn_act, btn_unit); // Ignore button event
    if (exitType == FatFsError) {
        return getUIMode(PowerOffMode);
    } else if (idle_count++ > 1 * OneSec) { // Always transfer to FileViewMode after 1 sec when no error
        return getUIMode(FileViewMode);
    }
    return this;
}

void UIOpeningMode::entry(UIMode* prevMode)
{
    UIMode::entry(prevMode);
    pm_set_power_keep(true);
    sleep_ms(10);
    pm_enable_button_control(true);

    // Particular microsd card needs interval time when reboot, otherwise fails to mount (Samsung PRO Plus)
    if (pm_is_caused_reboot()) {
        sleep_ms(1000);
    }
    // Mount FAT
    int count = 0;
    FRESULT fr;
    while (true) {
        fr = file_menu_init(&vars->fs_type);
        if (fr == FR_OK || count++ > 10) { break; }
        sleep_ms(10);
    }
    if (fr != FR_OK) { // Mount Fail
        exitType = FatFsError;
        lcd->setMsg("No SD Card Found!", true);
        return;
    }
    const char* fs_type_str[5] = {"NOT_MOUNTED", "FAT12", "FAT16", "FAT32", "EXFAT"};
    printf("SD Card File System = %s\r\n", fs_type_str[vars->fs_type]);

    // Open root directory
    file_menu_open_dir("/");
    if (file_menu_get_num() <= 1) { // Directory read Fail
        exitType = FatFsError;
        lcd->setMsg("SD Card Read Error!", true);
        return;
    }
    exitType = NoError;
    // Opening Logo
    lcd->setImageJpeg("logo.jpg");

    restoreFromFlash();

    audio_codec_init();
    audio_codec_set_dac_enable_func(pm_set_audio_dac_enable);

    lcd->switchToOpening();
    pm_set_audio_dac_enable(true); // I2S DAC Mute Off
}

void UIOpeningMode::draw() const
{
    lcd->drawOpening();
    pm_backlight_update();
    ui_clear_btn_evt();
}

//=========================================
// Implementation of UIFileViewMode class
//=========================================
UIFileViewMode::UIFileViewMode() : UIMode("UIFileViewMode", FileViewMode)
{
    sft_val = new uint16_t[vars->num_list_lines];
    for (int i = 0; i < vars->num_list_lines; i++) {
        sft_val[i] = 0;
    }
}

void UIFileViewMode::listIdxItems()
{
    char str[256];
    memset(str, 0, sizeof(str));
    for (int i = 0; i < vars->num_list_lines; i++) {
        if (vars->idx_head+i >= file_menu_get_num()) {
            lcd->setListItem(i, ""); // delete
            continue;
        }
        file_menu_get_fname(vars->idx_head+i, str, sizeof(str) - 1);
        IconIndex_t iconIndex = file_menu_is_dir(vars->idx_head+i) ? IconIndex_t::FOLDER : IconIndex_t::FILE;
        lcd->setListItem(i, str, iconIndex, (i == vars->idx_column));
    }
}

uint16_t UIFileViewMode::getNumAudioFiles() const
{
    uint16_t num_tracks = 0;
    num_tracks += file_menu_get_ext_num("wav", 3) + file_menu_get_ext_num("WAV", 3);
    return num_tracks;
}

void UIFileViewMode::chdir() const
{
    stack_data_t item;
    if (vars->idx_head+vars->idx_column == 0) { // upper ("..") dirctory
        if (dir_stack.size() > 0) {
            if (vars->fs_type == FS_EXFAT) { // This is workaround for FatFs known bug for ".." in EXFAT
                std::stack<stack_data_t> temp_stack;
                while (dir_stack.size() > 0) {
                    item = dir_stack.top();
                    dir_stack.pop();
                    //printf("pop %d %d %d\r\n", dir_stack.size(), item.head, item.column);
                    temp_stack.push(item);
                }
                file_menu_close_dir();
                file_menu_open_dir("/"); // Root directory
                while (temp_stack.size() > 1) {
                    item = temp_stack.top();
                    temp_stack.pop();
                    //printf("pushA %d %d %d\r\n", dir_stack.size(), item.head, item.column);
                    file_menu_sort_entry(item.head+item.column, item.head+item.column+1);
                    file_menu_ch_dir(item.head+item.column);
                    dir_stack.push(item);
                }
                item = temp_stack.top();
                temp_stack.pop();
                //printf("pushB %d %d %d\r\n", dir_stack.size(), item.head, item.column);
                dir_stack.push(item);
            } else {
                file_menu_ch_dir(vars->idx_head+vars->idx_column);
            }
        } else { // Already in top directory
            file_menu_close_dir();
            file_menu_open_dir("/"); // Root directory
            item.head = 0;
            item.column = 0;
            dir_stack.push(item);
        }
        item = dir_stack.top();
        dir_stack.pop();
        vars->idx_head = item.head;
        vars->idx_column = item.column;
    } else { // normal directory
        item.head = vars->idx_head;
        item.column = vars->idx_column;
        dir_stack.push(item);
        file_menu_ch_dir(vars->idx_head+vars->idx_column);
        vars->idx_head = 0;
        vars->idx_column = 0;
    }
}

UIMode* UIFileViewMode::nextPlay()
{
    switch (cfgMenu.get(ConfigMenuId::PLAY_NEXT_PLAY_ALBUM)) {
        case ConfigMenu::NextPlayAction_t::Sequential:
            return sequentialSearch(false);
            break;
        case ConfigMenu::NextPlayAction_t::SequentialRepeat:
            return sequentialSearch(true);
            break;
        case ConfigMenu::NextPlayAction_t::Repeat:
            vars->idx_head = 0;
            vars->idx_column = 0;
            findFirstAudioTrack();
            return getUIPlayMode();
        case ConfigMenu::NextPlayAction_t::Random:
            return randomSearch(cfgMenu.get(ConfigMenuId::PLAY_RANDOM_DIR_DEPTH));
            break;
        case ConfigMenu::NextPlayAction_t::Stop:
        default:
            return this;
            break;
    }
    return this;
}

UIMode* UIFileViewMode::sequentialSearch(const bool& repeatFlg)
{
    int stack_count;
    uint16_t last_dir_idx;

    printf("Sequential Search\r\n");
    stack_count = dir_stack.size();
    if (stack_count < 1) { return this; }
    {
        vars->idx_head = 0;
        vars->idx_column = 0;
        chdir(); // cd ..;
    }
    vars->idx_head += vars->idx_column;
    vars->idx_column = 0;
    last_dir_idx = vars->idx_head;
    while (1) {
        {
            if (file_menu_get_dir_num() == 0) { break; }
            while (1) {
                vars->idx_head++;
                if (repeatFlg) {
                    // [Option 1] loop back to first album (don't take ".." directory)
                    if (vars->idx_head >= file_menu_get_num()) { vars->idx_head = 1; }
                } else {
                    // [Option 2] stop at the bottom of albums
                    if (vars->idx_head >= file_menu_get_num()) {
                        // go back to last dir
                        vars->idx_head = last_dir_idx;
                        chdir();
                        return this;
                    }
                }
                file_menu_sort_entry(vars->idx_head, vars->idx_head+1);
                if (file_menu_is_dir(vars->idx_head) > 0) { break; }
            }
            chdir();
        }
        // Check if Next Target Dir has Audio track files
        if (stack_count == dir_stack.size() && getNumAudioFiles() > 0) {
            findFirstAudioTrack();
            break;
        }
        // Otherwise, chdir to stack_count-depth and retry again
        printf("Retry Sequential Search\r\n");
        while (stack_count - 1 != dir_stack.size()) {
            vars->idx_head = 0;
            vars->idx_column = 0;
            chdir(); // cd ..;
        }
    }
    return getUIPlayMode();
}

UIMode* UIFileViewMode::randomSearch(const uint16_t& depth)
{
    int i;
    int stack_count;

    printf("Random Search\r\n");
    stack_count = dir_stack.size();
    if (stack_count < depth) { return this; }
    for (i = 0; i < depth; i++) {
        vars->idx_head = 0;
        vars->idx_column = 0;
        chdir(); // cd ..;
    }
    while (1) {
        for (i = 0; i < depth; i++) {
            if (file_menu_get_dir_num() == 0) { break; }
            while (1) {
                vars->idx_head = rand() % file_menu_get_num();
                file_menu_sort_entry(vars->idx_head, vars->idx_head+1);
                if (file_menu_is_dir(vars->idx_head) > 0) { break; }
            }
            vars->idx_column = 0;
            chdir();
        }
        // Check if Next Target Dir has Audio track files
        if (stack_count == dir_stack.size() && getNumAudioFiles() > 0) {
            findFirstAudioTrack();
            break;
        }
        // Otherwise, chdir to stack_count-depth and retry again
        printf("Retry Random Search\r\n");
        while (stack_count - depth != dir_stack.size()) {
            vars->idx_head = 0;
            vars->idx_column = 0;
            chdir(); // cd ..;
        }
    }
    return getUIPlayMode();
}

void UIFileViewMode::findFirstAudioTrack() const
{
    vars->idx_play = 0;
    while (vars->idx_play < file_menu_get_num()) {
        if (isAudioFile(vars->idx_play)) { break; }
        vars->idx_play++;
    }
}

void UIFileViewMode::idxInc() const
{
    if (vars->idx_head >= file_menu_get_num() - vars->num_list_lines && vars->idx_column == vars->num_list_lines-1) { return; }
    if (vars->idx_head + vars->idx_column + 1 >= file_menu_get_num()) { return; }
    vars->idx_column++;
    if (vars->idx_column >= vars->num_list_lines) {
        if (vars->idx_head + vars->num_list_lines >= file_menu_get_num() - vars->num_list_lines) {
            vars->idx_column = vars->num_list_lines-1;
            vars->idx_head++;
        } else {
            vars->idx_column = 0;
            vars->idx_head += vars->num_list_lines;
        }
    }
}

void UIFileViewMode::idxDec() const
{
    if (vars->idx_head == 0 && vars->idx_column == 0) { return; }
    if (vars->idx_column == 0) {
        if (vars->idx_head < vars->num_list_lines) {
            vars->idx_column = 0;
            vars->idx_head--;
        } else {
            vars->idx_column = vars->num_list_lines-1;
            vars->idx_head -= vars->num_list_lines;
        }
    } else {
        vars->idx_column--;
    }
}

void UIFileViewMode::idxFastInc() const
{
    if (vars->idx_head >= file_menu_get_num() - vars->num_list_lines && vars->idx_column == vars->num_list_lines-1) { return; }
    if (vars->idx_head + vars->idx_column + 1 >= file_menu_get_num()) { return; }
    if (file_menu_get_num() < vars->num_list_lines) {
        idxInc();
    } else if (vars->idx_head + vars->num_list_lines >= file_menu_get_num() - vars->num_list_lines) {
        vars->idx_head = file_menu_get_num() - vars->num_list_lines;
        idxInc();
    } else {
        vars->idx_head += vars->num_list_lines;
    }
}

void UIFileViewMode::idxFastDec() const
{
    if (vars->idx_head == 0 && vars->idx_column == 0) { return; }
    if (vars->idx_head < vars->num_list_lines) {
        vars->idx_head = 0;
        idxDec();
    } else {
        vars->idx_head -= vars->num_list_lines;
    }
}

UIMode* UIFileViewMode::getUIPlayMode()
{
    if (vars->idx_play == 0) {
        vars->idx_play = vars->idx_head + vars->idx_column;
    }
    //file_menu_full_sort();
    vars->num_tracks = getNumAudioFiles();
    return getUIMode(PlayMode);
}

UIMode* UIFileViewMode::update()
{
    vars->resume_ui_mode = ui_mode_enm;
    switch (vars->init_dest_ui_mode) {
        case PlayMode:
            if (isAudioFile(vars->idx_play)) {
                return getUIPlayMode();
            }
            break;
        default:
            break;
    }
    if (ui_get_btn_evt(btn_act, btn_unit)) {
        vars->do_next_play = None;
        auto& btn_layout = (btn_unit == button_unit_t::PushButtons) ? cfgParam.P_CFG_MENU_IDX_GENERAL_PUSH_BUTTON_LAYOUT : cfgParam.P_CFG_MENU_IDX_GENERAL_HP_BUTTON_LAYOUT;
        switch (btn_act) {
            case button_action_t::CenterSingle:
                if (file_menu_is_dir(vars->idx_head+vars->idx_column) > 0) { // Target is Directory
                    chdir();
                    listIdxItems();
                } else { // Target is File
                    if (isAudioFile(vars->idx_head + vars->idx_column)) {
                        return getUIPlayMode();
                    }
                }
                break;
            case button_action_t::CenterDouble:
                // upper ("..") dirctory
                vars->idx_head = 0;
                vars->idx_column = 0;
                chdir();
                listIdxItems();
                break;
            case button_action_t::CenterTriple:
                return randomSearch(cfgMenu.get(ConfigMenuId::PLAY_RANDOM_DIR_DEPTH));
                break;
            case button_action_t::CenterLong:
                return getUIMode(ConfigMode);
                break;
            case button_action_t::CenterLongLong:
                break;
            case button_action_t::PlusSingle:
                if (btn_layout.get() == static_cast<uint32_t>(button_layout_t::Horizontal)) {
                    idxInc();
                } else {
                    idxDec();
                }
                listIdxItems();
                break;
            case button_action_t::PlusLong:
                if (btn_layout.get() == static_cast<uint32_t>(button_layout_t::Horizontal)) {
                    idxFastInc();
                } else {
                    idxFastDec();
                }
                listIdxItems();
                break;
            case button_action_t::MinusSingle:
                if (btn_layout.get() == static_cast<uint32_t>(button_layout_t::Horizontal)) {
                    idxDec();
                } else {
                    idxInc();
                }
                listIdxItems();
                break;
            case button_action_t::MinusLong:
                if (btn_layout.get() == static_cast<uint32_t>(button_layout_t::Horizontal)) {
                    idxFastDec();
                } else {
                    idxFastInc();
                }
                listIdxItems();
                break;
            default:
                break;
        }
        idle_count = 0;
    }
    switch (vars->do_next_play) {
        case ImmediatePlay:
            vars->do_next_play = None;
            return randomSearch(cfgMenu.get(ConfigMenuId::PLAY_RANDOM_DIR_DEPTH));
            break;
        case TimeoutPlay:
            if (idle_count > cfgMenu.get(ConfigMenuId::PLAY_TIME_TO_NEXT_PLAY) * OneSec) {
                vars->do_next_play = None;
                return nextPlay();
            }
            break;
        default:
            break;
    }
    if (pm_get_low_battery()) {
        lcd->setMsg("Low Battery!", true);
        exitType = LowBatteryVoltage;
        return getUIMode(PowerOffMode);
    } else if (idle_count > cfgMenu.get(ConfigMenuId::GENERAL_TIME_TO_POWER_OFF) * OneSec) {
        lcd->setMsg("Bye");
        return getUIMode(PowerOffMode);
    } else if (idle_count > 5 * OneSec) {
        file_menu_idle(); // for background sort
    }
    lcd->setBatteryVoltage(pm_get_battery_voltage());
    idle_count++;
    return this;
}

void UIFileViewMode::entry(UIMode* prevMode)
{
    UIMode::entry(prevMode);
    listIdxItems();
    lcd->switchToListView();
}

void UIFileViewMode::draw() const
{
    lcd->drawListView();
    pm_backlight_update();
    ui_clear_btn_evt();
}

//====================================
// Implementation of UIPlayMode class
//====================================
UIPlayMode::UIPlayMode() : UIMode("UIPlayMode", PlayMode)
{
}

UIMode* UIPlayMode::update()
{
    vars->resume_ui_mode = ui_mode_enm;
    PlayAudio* codec = get_audio_codec();
    if (ui_get_btn_evt(btn_act, btn_unit)) {
        switch (btn_act) {
            case button_action_t::CenterSingle:
                codec->pause(!codec->isPaused());
                break;
            case button_action_t::CenterDouble:
                vars->idx_play = 0;
                codec->stop();
                vars->do_next_play = None;
                return getUIMode(FileViewMode);
                break;
            case button_action_t::CenterTriple:
                vars->idx_play = 0;
                codec->stop();
                vars->do_next_play = ImmediatePlay;
                return getUIMode(FileViewMode);
                break;
            case button_action_t::CenterLong:
                return getUIMode(ConfigMode);
                break;
            case button_action_t::CenterLongLong:
                break;
            case button_action_t::PlusSingle:
            case button_action_t::PlusLong:
                PlayAudio::volumeUp();
                break;
            case button_action_t::MinusSingle:
            case button_action_t::MinusLong:
                PlayAudio::volumeDown();
                break;
            default:
                break;
        }
        idle_count = 0;
    }
    if (pm_get_low_battery()) {
        codec->getCurrentPosition(&vars->fpos, &vars->samples_played);
        codec->stop();
        lcd->setMsg("Low Battery!", true);
        exitType = LowBatteryVoltage;
        return getUIMode(PowerOffMode);
    } else if (codec->isPaused() && idle_count > cfgMenu.get(ConfigMenuId::GENERAL_TIME_TO_POWER_OFF) * OneSec) {
        codec->getCurrentPosition(&vars->fpos, &vars->samples_played);
        codec->stop();
        lcd->setMsg("Bye");
        return getUIMode(PowerOffMode);
    } else if (!codec->isPlaying()) {
        idle_count = 0;
        while (++vars->idx_play < file_menu_get_num()) {
            if (isAudioFile(vars->idx_play)) {
                play();
                return this;
            }
        }
        vars->idx_play = 0;
        codec->stop();
        vars->do_next_play = TimeoutPlay;
        return getUIMode(FileViewMode);
    }
    lcd->setVolume(PlayAudio::getVolume());
    lcd->setPlayTime(codec->elapsedMillis()/1000, codec->totalMillis()/1000, codec->isPaused());
    float levelL, levelR;
    codec->getLevel(&levelL, &levelR);
    lcd->setAudioLevel(levelL, levelR);
    lcd->setBatteryVoltage(pm_get_battery_voltage());
    idle_count++;
    return this;
}

void UIPlayMode::readTag()
{
    char str[256];
    
    // Read TAG
    memset(str, 0, sizeof(str));
    file_menu_get_fname(vars->idx_play, str, sizeof(str) - 1);
    tag.loadFile(str);

    // copy TAG text
    if (tag.getUTF8Track(str, sizeof(str) - 1)) {
        std::string s(str);
        uint16_t track;
        if (s.size() > 0 && std::isdigit(s.at(0))) {  // accept both "12" and  "12/20" as 12
            track = static_cast<uint8_t>(std::stoi(s));  // stoi stops conversion if non-number appeared
        } else {
            track = 0;
        }
        sprintf(str, "%d/%d", track, vars->num_tracks);
    } else {
        uint16_t track = file_menu_get_ext_num_from_max("wav", 3, vars->idx_play + 1) + file_menu_get_ext_num_from_max("WAV", 3, vars->idx_play +1);
        sprintf(str, "%d/%d", track, vars->num_tracks);
    }
    lcd->setTrack(str);
    if (tag.getUTF8Title(str, sizeof(str) - 1)) {
        lcd->setTitle(str);
    } else { // display filename if no TAG
        file_menu_get_fname(vars->idx_play, str, sizeof(str) - 1);
        lcd->setTitle(str);
        /*
        file_menu_get_fname_UTF16(vars->idx_play, (char16_t*) str, sizeof(str)/2);
        lcd->setTitle(utf16_to_utf8((const char16_t*) str).c_str(), utf8);
        */
    }
    if (tag.getUTF8Album(str, sizeof(str) - 1)) lcd->setAlbum(str); else lcd->setAlbum("");
    if (tag.getUTF8Artist(str, sizeof(str) - 1)) lcd->setArtist(str); else lcd->setArtist("");
    //if (tag.getUTF8Year(str, sizeof(str) - 1)) lcd->setYear(str); else lcd->setYear("");

    {  // load image from TAG
        mime_t mime;
        ptype_t ptype;
        size_t pos;
        size_t size;
        bool isUnsynced;
        if (tag.getPicturePos(0, mime, ptype, pos, size, isUnsynced)) {
            //printf("found coverart mime: %d, ptype: %d, pos: %d, size: %d, isUnsynced: %d\r\n", mime, ptype, (int) pos, size, (int) isUnsynced);
            if (!isUnsynced && mime == jpeg && size != tagImageSize) {  // Note: judge by size to check if the image is identical to previous
                file_menu_get_fname(vars->idx_play, str, sizeof(str) - 1);
                lcd->setImageJpeg(str, pos, size);
                tagImageSize = size;
                loadImageFromDir = false;
            }
        }
    }

    if (loadImageFromDir) {  // load image from local directory
        uint16_t idx = 0;
        bool loaded = false;
        while (idx < file_menu_get_num()) {
            if (file_menu_match_ext(idx, "jpg", 3) || file_menu_match_ext(idx, "JPG", 3) ||
                file_menu_match_ext(idx, "jpeg", 4) || file_menu_match_ext(idx, "JPEG", 4)) {
                file_menu_get_fname(idx, str, sizeof(str) - 1);
                lcd->setImageJpeg(str);
                loaded = true;
                break;
            }
            idx++;
        }
        if (!loaded) { lcd->resetImage(); }
    }
    return;
}

void UIPlayMode::play()
{
    char str[FF_MAX_LFN];
    memset(str, 0, sizeof(str));
    file_menu_get_fname(vars->idx_play, str, sizeof(str) - 1);
    printf("%s\r\n", str);
    PlayAudio* codec = get_audio_codec();
    readTag();
    loadImageFromDir = false;
    codec->play(str, vars->fpos, vars->samples_played);
    lcd->setBitRes(codec->getBitsPerSample());
    lcd->setSampleFreq(codec->getSampFreq());
    vars->fpos = 0;
    vars->samples_played = 0;
}

void UIPlayMode::entry(UIMode* prevMode)
{
    UIMode::entry(prevMode);
    if (prevMode->getUIModeEnm() != ConfigMode) {
        tagImageSize = 0;
        loadImageFromDir = true;
        play();
    }
    lcd->switchToPlay();
}

void UIPlayMode::draw() const
{
    lcd->drawPlay();
    pm_backlight_update();
    ui_clear_btn_evt();
}

//=======================================
// Implementation of UIConfigMode class
//=======================================
UIConfigMode::UIConfigMode() : UIMode("UIConfigMode", ConfigMode),
    idx_head(0), idx_column(0)
{
}

uint16_t UIConfigMode::getNum() const
{
    return (uint16_t) cfgMenu.getNum() + 1;
}

const char* UIConfigMode::getStr(const uint16_t& idx) const
{
    if (idx == 0) { return "[Back]"; }
    return cfgMenu.getStr((int) idx-1);
}

IconIndex_t UIConfigMode::getIconIndex(const uint16_t& idx) const
{
    IconIndex_t iconIndex = IconIndex_t::UNDEF;
    if (idx == 0) {
        iconIndex = IconIndex_t::LEFTARROW;
    } else if (!cfgMenu.isSelection()) {
        iconIndex = IconIndex_t::GEAR;
    } else if (cfgMenu.selIdxMatched(idx-1)) {
        iconIndex = IconIndex_t::CHECKED;
    }
    return iconIndex;
}

void UIConfigMode::listIdxItems() const
{
    for (int i = 0; i < vars->num_list_lines; i++) {
        if (idx_head+i >= getNum()) {
            lcd->setListItem(i, ""); // delete
            continue;
        }
        lcd->setListItem(i, getStr(idx_head+i), getIconIndex(idx_head+i), (i == idx_column));
    }
}

void UIConfigMode::idxInc()
{
    if (idx_head >= getNum() - vars->num_list_lines && idx_column == vars->num_list_lines-1) { return; }
    if (idx_head + idx_column + 1 >= getNum()) { return; }
    idx_column++;
    if (idx_column >= vars->num_list_lines) {
        if (idx_head + vars->num_list_lines >= getNum() - vars->num_list_lines) {
            idx_column = vars->num_list_lines-1;
            idx_head++;
        } else {
            idx_column = 0;
            idx_head += vars->num_list_lines;
        }
    }
}

void UIConfigMode::idxDec()
{
    if (idx_head == 0 && idx_column == 0) { return; }
    if (idx_column == 0) {
        if (idx_head < vars->num_list_lines) {
            idx_column = 0;
            idx_head--;
        } else {
            idx_column = vars->num_list_lines-1;
            idx_head -= vars->num_list_lines;
        }
    } else {
        idx_column--;
    }
}

int UIConfigMode::select()
{
    stack_data_t stack_data;
    if (idx_head + idx_column == 0) {
        if (cfgMenu.leave()) {
            stack_data = path_stack.top();
            path_stack.pop();
            idx_head = stack_data.head;
            idx_column = stack_data.column;
        } else {
            return 0; // exit from Config
        }
    } else {
        if (cfgMenu.enter(idx_head + idx_column - 1)) {
            stack_data.head = idx_head;
            stack_data.column = idx_column;
            path_stack.push(stack_data);
            idx_head = 0;
            idx_column = 0;
        }
    }
    return 1;
}

UIMode* UIConfigMode::update()
{
    PlayAudio* codec = get_audio_codec();
    if (ui_get_btn_evt(btn_act, btn_unit)) {
        vars->do_next_play = None;
        auto& btn_layout = (btn_unit == button_unit_t::PushButtons) ? cfgParam.P_CFG_MENU_IDX_GENERAL_PUSH_BUTTON_LAYOUT : cfgParam.P_CFG_MENU_IDX_GENERAL_HP_BUTTON_LAYOUT;
        switch (btn_act) {
            case button_action_t::CenterSingle:
                if (select()) {
                    listIdxItems();
                } else {
                    return prevMode;
                }
                break;
            case button_action_t::CenterDouble:
                return prevMode;
                break;
            case button_action_t::CenterTriple:
                break;
            case button_action_t::CenterLong:
                break;
            case button_action_t::CenterLongLong:
                codec->getCurrentPosition(&vars->fpos, &vars->samples_played);
                codec->stop();
                lcd->setMsg("Bye");
                return getUIMode(PowerOffMode);
                break;
            case button_action_t::PlusSingle:
            case button_action_t::PlusLong:
                if (btn_layout.get() == static_cast<uint32_t>(button_layout_t::Horizontal)) {
                    idxInc();
                } else {
                    idxDec();
                }
                listIdxItems();
                break;
            case button_action_t::MinusSingle:
            case button_action_t::MinusLong:
                if (btn_layout.get() == static_cast<uint32_t>(button_layout_t::Horizontal)) {
                    idxDec();
                } else {
                    idxInc();
                }
                listIdxItems();
                break;
            default:
                break;
        }
        idle_count = 0;
    }
    if (idle_count > cfgMenu.get(ConfigMenuId::GENERAL_TIME_TO_LEAVE_CONFIG) * OneSec) {
        return prevMode;
    }
    idle_count++;
    return this;
}

void UIConfigMode::entry(UIMode* prevMode)
{
    UIMode::entry(prevMode);
    listIdxItems();
    lcd->switchToListView();
}

void UIConfigMode::draw() const
{
    lcd->drawListView();
    pm_backlight_update();
    ui_clear_btn_evt();
}

//=======================================
// Implementation of UIPowerOffMode class
//=======================================
UIPowerOffMode::UIPowerOffMode() : UIMode("UIPowerOffMode", PowerOffMode)
{
}

void UIPowerOffMode::storeToFlash() const
{
    // Save user parameters to cfgParam
    uint32_t seed = to_ms_since_boot(get_absolute_time());
    cfgParam.P_CFG_SEED.set(seed);

    uint8_t volume = PlayAudio::getVolume();
    cfgParam.P_CFG_VOLUME.set(volume);
    uint8_t stack_count = dir_stack.size();
    cfgParam.P_CFG_STACK_COUNT.set(stack_count);
    for (int i = 0; i < stack_count; i++) {
        stack_data_t item;
        item = dir_stack.top();
        dir_stack.pop();
        auto& head_param = (i == 4) ? cfgParam.P_CFG_STACK_HEAD4 : (i == 3) ? cfgParam.P_CFG_STACK_HEAD3 : (i == 2) ? cfgParam.P_CFG_STACK_HEAD2 : (i == 1) ? cfgParam.P_CFG_STACK_HEAD1 : cfgParam.P_CFG_STACK_HEAD0;
        auto& column_param = (i == 4) ? cfgParam.P_CFG_STACK_COLUMN4 : (i == 3) ? cfgParam.P_CFG_STACK_COLUMN3 : (i == 2) ? cfgParam.P_CFG_STACK_COLUMN2 : (i == 1) ? cfgParam.P_CFG_STACK_COLUMN1 : cfgParam.P_CFG_STACK_COLUMN0;
        head_param.set(item.head);
        column_param.set(item.column);
    }

    cfgParam.P_CFG_UIMODE.set(vars->resume_ui_mode);

    cfgParam.P_CFG_IDX_HEAD.set(vars->idx_head);
    cfgParam.P_CFG_IDX_COLUMN.set(vars->idx_column);
    cfgParam.P_CFG_IDX_PLAY.set(vars->idx_play);

    cfgParam.P_CFG_PLAY_POS.set(static_cast<uint64_t>(vars->fpos));
    cfgParam.P_CFG_SAMPLES_PLAYED.set(vars->samples_played);

    // Store Configuration parameters to Flash
    cfgParam.finalize();
}

UIMode* UIPowerOffMode::update()
{
    ui_get_btn_evt(btn_act, btn_unit); // Ignore button event
    if ((idle_count > 1 * OneSec && exitType == NoError) || idle_count > 4 * OneSec) {
        pm_set_power_keep(false); // Power Off unless being charged
        if (pm_usb_power_detected()) {
            return getUIMode(ChargeMode);
        }
    }
    idle_count++;
    return this;
}

void UIPowerOffMode::entry(UIMode* prevMode)
{
    UIMode::entry(prevMode);
    pm_set_audio_dac_enable(false); // I2S DAC Mute On
    if (exitType != FatFsError) {
        storeToFlash();
        file_menu_close_dir();
        file_menu_deinit();
        audio_codec_deinit();
    } else {
        pico_fatfs_reboot_spi();
    }

    lcd->switchToPowerOff();
}

void UIPowerOffMode::draw() const
{
    lcd->drawPowerOff();
    ui_clear_btn_evt();
}