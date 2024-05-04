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
#include "ui_control.h"
#include "power_manage.h"
#include "UserFlash.h"
#include "ConfigParam.h"
#include "ConfigMenu.h"
#include "audio_codec.h"
#include "file_menu_FatFs.h"
#include "LcdCanvas.h"
#include "TagRead.h"

// ENABLE_REBOOT_AFTER_WAKEUP:
// reboot allows stdio_usb (USB CDC) and Serial terminal to be re-activated
// otherwise, USB cable needs to be reconnected
#define ENABLE_REBOOT_AFTER_WAKEUP

TagRead tag;

// UIMode class instances
button_action_t UIMode::btn_act;
UIVars* UIMode::vars;
stack_t* UIMode::dir_stack;
UIMode::ExitType UIMode::exitType = UIMode::NoError;

//================================
// Implementation of UIMode class
//================================
/*static*/
void UIMode::initialize(UIVars* vars)
{
    UIMode::vars = vars;
    dir_stack = stack_init();
}

UIMode::UIMode(const char* name, ui_mode_enm_t ui_mode_enm) : name(name), prevMode(nullptr), ui_mode_enm(ui_mode_enm), idle_count(0)
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

bool UIMode::isAudioFile(uint16_t idx)
{
    if (file_menu_match_ext(idx, "wav", 3) ||  file_menu_match_ext(idx, "WAV", 3)) {
        set_audio_codec(PlayAudio::AUDIO_CODEC_WAV);
        return true;
    }
    set_audio_codec(PlayAudio::AUDIO_CODEC_NONE);
    return false;
}

const char* UIMode::getName()
{
    return name;
}

ui_mode_enm_t UIMode::getUIModeEnm()
{
    return ui_mode_enm;
}

uint16_t UIMode::getIdleCount()
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
    ui_get_btn_evt(&btn_act); // Ignore button event
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

void UIInitialMode::draw()
{
    ui_clear_btn_evt();
}

void UIInitialMode::loadFromFlash()
{
    // Load Configuration parameters from Flash
    configParam.initialize();
    configMenu.scanHookFunc();
}

//=======================================
// Implementation of UIChargeMode class
//=======================================
UIChargeMode::UIChargeMode() : UIMode("UIChargeMode", ChargeMode)
{
}

UIMode* UIChargeMode::update()
{
    if (ui_get_btn_evt(&btn_act)) {
        switch (btn_act) {
            case ButtonCenterSingle:
                return getUIMode(OpeningMode);
            default:
                break;
        }
        idle_count = 0;
    }
    if (idle_count >= 2*OneSec) {
        lcd.setMsg("");
        lcd.clear(true);
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
    lcd.setMsg("Charging", true);
    pm_enable_button_control(true);  // for wake up
    pm_set_power_keep(false);
}

void UIChargeMode::draw()
{
    lcd.drawPowerOff();
    ui_clear_btn_evt();
}

//=======================================
// Implementation of UIOpeningMode class
//=======================================
UIOpeningMode::UIOpeningMode() : UIMode("UIOpeningMode", OpeningMode)
{
}

void UIOpeningMode::restoreFromFlash()
{
    // Load Configuration parameters from Flash
    userFlash.printInfo();
    configParam.printInfo();
    configParam.incBootCount();

    // Restore from configParam to user parameters
    srand(GET_CFG_SEED);
    PlayAudio::setVolume(GET_CFG_VOLUME);
    bool err_flg = false;
    for (int i = GET_CFG_STACK_COUNT - 1; i >= 0; i--) {
        stack_data_t item;
        item.head = GET_CFG_STACK_HEAD(i);
        item.column = GET_CFG_STACK_COLUMN(i);
        if (item.head+item.column >= file_menu_get_num()) { err_flg = true; break; } // idx overflow
        file_menu_sort_entry(item.head+item.column, item.head+item.column + 1);
        if (file_menu_is_dir(item.head+item.column) <= 0 || item.head+item.column == 0) { err_flg = true; break; } // Not Directory or Parent Directory
        stack_push(dir_stack, &item);
        file_menu_ch_dir(item.head+item.column);
    }

    vars->init_dest_ui_mode = static_cast<ui_mode_enm_t>(GET_CFG_UIMODE);

    uint16_t idx_head = GET_CFG_IDX_HEAD;
    uint16_t idx_column = GET_CFG_IDX_COLUMN;
    if (idx_head+idx_column >= file_menu_get_num()) { err_flg = true; } // idx overflow
    if (err_flg) { // Load Error
        printf("dir_stack Load Error. root directory is set\n");
        stack_delete(dir_stack);
        dir_stack = stack_init();
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
        vars->idx_play = GET_CFG_IDX_PLAY;
        uint64_t play_pos = GET_CFG_PLAY_POS;
        vars->fpos = (size_t) play_pos;
        vars->samples_played = GET_CFG_SAMPLES_PLAYED;
    }
}

UIMode* UIOpeningMode::update()
{
    ui_get_btn_evt(&btn_act); // Ignore button event
    if (exitType == FatFsError) {
        return getUIMode(PowerOffMode);
    } else if (idle_count++ > 1*OneSec) { // Always transfer to FileViewMode after 1 sec when no error
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
        lcd.setMsg("No SD Card Found!", true);
        return;
    }
    const char* fs_type_str[5] = {"NOT_MOUNTED", "FAT12", "FAT16", "FAT32", "EXFAT"};
    printf("SD Card File System = %s\n", fs_type_str[vars->fs_type]);

    // Open root directory
    file_menu_open_dir("/");
    if (file_menu_get_num() <= 1) { // Directory read Fail
        exitType = FatFsError;
        lcd.setMsg("SD Card Read Error!", true);
        return;
    }
    exitType = NoError;
    // Opening Logo
    lcd.setImageJpeg("logo.jpg");

    restoreFromFlash();

    audio_codec_init();
    audio_codec_set_dac_enable_func(pm_set_audio_dac_enable);

    lcd.switchToOpening();
    pm_set_audio_dac_enable(true); // I2S DAC Mute Off
}

void UIOpeningMode::draw()
{
    lcd.drawOpening();
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
            lcd.setListItem(i, ""); // delete
            continue;
        }
        file_menu_get_fname(vars->idx_head+i, str, sizeof(str) - 1);
        IconIndex_t iconIndex = file_menu_is_dir(vars->idx_head+i) ? IconIndex_t::FOLDER : IconIndex_t::FILE;
        lcd.setListItem(i, str, iconIndex, (i == vars->idx_column));
    }
}

uint16_t UIFileViewMode::getNumAudioFiles()
{
    uint16_t num_tracks = 0;
    num_tracks += file_menu_get_ext_num("wav", 3) + file_menu_get_ext_num("WAV", 3);
    return num_tracks;
}

void UIFileViewMode::chdir()
{
    stack_data_t item;
    if (vars->idx_head+vars->idx_column == 0) { // upper ("..") dirctory
        if (stack_get_count(dir_stack) > 0) {
            if (vars->fs_type == FS_EXFAT) { // This is workaround for FatFs known bug for ".." in EXFAT
                stack_t* temp_stack = stack_init();
                while (stack_get_count(dir_stack) > 0) {
                    stack_pop(dir_stack, &item);
                    //printf("pop %d %d %d\n", stack_get_count(dir_stack), item.head, item.column);
                    stack_push(temp_stack, &item);
                }
                file_menu_close_dir();
                file_menu_open_dir("/"); // Root directory
                while (stack_get_count(temp_stack) > 1) {
                    stack_pop(temp_stack, &item);
                    //printf("pushA %d %d %d\n", stack_get_count(dir_stack), item.head, item.column);
                    file_menu_sort_entry(item.head+item.column, item.head+item.column+1);
                    file_menu_ch_dir(item.head+item.column);
                    stack_push(dir_stack, &item);
                }
                stack_pop(temp_stack, &item);
                //printf("pushB %d %d %d\n", stack_get_count(dir_stack), item.head, item.column);
                stack_push(dir_stack, &item);
                stack_delete(temp_stack);
            } else {
                file_menu_ch_dir(vars->idx_head+vars->idx_column);
            }
        } else { // Already in top directory
            file_menu_close_dir();
            file_menu_open_dir("/"); // Root directory
            item.head = 0;
            item.column = 0;
            stack_push(dir_stack, &item);
        }
        stack_pop(dir_stack, &item);
        vars->idx_head = item.head;
        vars->idx_column = item.column;
    } else { // normal directory
        item.head = vars->idx_head;
        item.column = vars->idx_column;
        stack_push(dir_stack, &item);
        file_menu_ch_dir(vars->idx_head+vars->idx_column);
        vars->idx_head = 0;
        vars->idx_column = 0;
    }
}

UIMode* UIFileViewMode::nextPlay()
{
    switch (GET_CFG_MENU_PLAY_NEXT_PLAY_ALBUM) {
        case ConfigMenu::next_play_action_t::Sequential:
            return sequentialSearch(false);
            break;
        case ConfigMenu::next_play_action_t::SequentialRepeat:
            return sequentialSearch(true);
            break;
        case ConfigMenu::next_play_action_t::Repeat:
            vars->idx_head = 0;
            vars->idx_column = 0;
            findFirstAudioTrack();
            return getUIPlayMode();
        case ConfigMenu::next_play_action_t::Random:
            return randomSearch(GET_CFG_MENU_PLAY_RANDOM_DIR_DEPTH);
            break;
        case ConfigMenu::next_play_action_t::Stop:
        default:
            return this;
            break;
    }
    return this;
}

UIMode* UIFileViewMode::sequentialSearch(bool repeatFlg)
{
    int stack_count;
    uint16_t last_dir_idx;

    printf("Sequential Search\n");
    stack_count = stack_get_count(dir_stack);
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
        if (stack_count == stack_get_count(dir_stack) && getNumAudioFiles() > 0) {
            findFirstAudioTrack();
            break;
        }
        // Otherwise, chdir to stack_count-depth and retry again
        printf("Retry Sequential Search\n");
        while (stack_count - 1 != stack_get_count(dir_stack)) {
            vars->idx_head = 0;
            vars->idx_column = 0;
            chdir(); // cd ..;
        }
    }
    return getUIPlayMode();
}

UIMode* UIFileViewMode::randomSearch(uint16_t depth)
{
    int i;
    int stack_count;

    printf("Random Search\n");
    stack_count = stack_get_count(dir_stack);
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
        if (stack_count == stack_get_count(dir_stack) && getNumAudioFiles() > 0) {
            findFirstAudioTrack();
            break;
        }
        // Otherwise, chdir to stack_count-depth and retry again
        printf("Retry Random Search\n");
        while (stack_count - depth != stack_get_count(dir_stack)) {
            vars->idx_head = 0;
            vars->idx_column = 0;
            chdir(); // cd ..;
        }
    }
    return getUIPlayMode();
}

void UIFileViewMode::findFirstAudioTrack()
{
    vars->idx_play = 0;
    while (vars->idx_play < file_menu_get_num()) {
        if (isAudioFile(vars->idx_play)) { break; }
        vars->idx_play++;
    }
}

void UIFileViewMode::idxInc(void)
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

void UIFileViewMode::idxDec(void)
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

void UIFileViewMode::idxFastInc(void)
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

void UIFileViewMode::idxFastDec(void)
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
    if (ui_get_btn_evt(&btn_act)) {
        vars->do_next_play = None;
        switch (btn_act) {
            case ButtonCenterSingle:
                if (file_menu_is_dir(vars->idx_head+vars->idx_column) > 0) { // Target is Directory
                    chdir();
                    listIdxItems();
                } else { // Target is File
                    if (isAudioFile(vars->idx_head + vars->idx_column)) {
                        return getUIPlayMode();
                    }
                }
                break;
            case ButtonCenterDouble:
                // upper ("..") dirctory
                vars->idx_head = 0;
                vars->idx_column = 0;
                chdir();
                listIdxItems();
                break;
            case ButtonCenterTriple:
                return randomSearch(GET_CFG_MENU_PLAY_RANDOM_DIR_DEPTH);
                break;
            case ButtonCenterLong:
                return getUIMode(ConfigMode);
                break;
            case ButtonCenterLongLong:
                break;
            case ButtonPlusSingle:
                idxDec();
                listIdxItems();
                break;
            case ButtonPlusLong:
                idxFastDec();
                listIdxItems();
                break;
            case ButtonMinusSingle:
                idxInc();
                listIdxItems();
                break;
            case ButtonMinusLong:
                idxFastInc();
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
            return randomSearch(GET_CFG_MENU_PLAY_RANDOM_DIR_DEPTH);
            break;
        case TimeoutPlay:
            if (idle_count > GET_CFG_MENU_PLAY_TIME_TO_NEXT_PLAY*OneSec) {
                vars->do_next_play = None;
                return nextPlay();
            }
            break;
        default:
            break;
    }
    if (pm_get_low_battery()) {
        lcd.setMsg("Low Battery!", true);
        exitType = LowBatteryVoltage;
        return getUIMode(PowerOffMode);
    } else if (idle_count > GET_CFG_MENU_GENERAL_TIME_TO_POWER_OFF*OneSec) {
        lcd.setMsg("Bye");
        return getUIMode(PowerOffMode);
    } else if (idle_count > 5*OneSec) {
        file_menu_idle(); // for background sort
    }
    lcd.setBatteryVoltage(pm_get_battery_voltage());
    idle_count++;
    return this;
}

void UIFileViewMode::entry(UIMode* prevMode)
{
    UIMode::entry(prevMode);
    listIdxItems();
    lcd.switchToListView();
}

void UIFileViewMode::draw()
{
    lcd.drawListView();
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
    if (ui_get_btn_evt(&btn_act)) {
        switch (btn_act) {
            case ButtonCenterSingle:
                codec->pause(!codec->isPaused());
                break;
            case ButtonCenterDouble:
                vars->idx_play = 0;
                codec->stop();
                vars->do_next_play = None;
                return getUIMode(FileViewMode);
                break;
            case ButtonCenterTriple:
                vars->idx_play = 0;
                codec->stop();
                vars->do_next_play = ImmediatePlay;
                return getUIMode(FileViewMode);
                break;
            case ButtonCenterLong:
                return getUIMode(ConfigMode);
                break;
            case ButtonCenterLongLong:
                break;
            case ButtonPlusSingle:
            case ButtonPlusLong:
                PlayAudio::volumeUp();
                break;
            case ButtonMinusSingle:
            case ButtonMinusLong:
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
        lcd.setMsg("Low Battery!", true);
        exitType = LowBatteryVoltage;
        return getUIMode(PowerOffMode);
    } else if (codec->isPaused() && idle_count > GET_CFG_MENU_GENERAL_TIME_TO_POWER_OFF*OneSec) {
        codec->getCurrentPosition(&vars->fpos, &vars->samples_played);
        codec->stop();
        lcd.setMsg("Bye");
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
    lcd.setVolume(PlayAudio::getVolume());
    lcd.setPlayTime(codec->elapsedMillis()/1000, codec->totalMillis()/1000, codec->isPaused());
    float levelL, levelR;
    codec->getLevel(&levelL, &levelR);
    lcd.setAudioLevel(levelL, levelR);
    lcd.setBatteryVoltage(pm_get_battery_voltage());
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
    lcd.setTrack(str);
    if (tag.getUTF8Title(str, sizeof(str) - 1)) {
        lcd.setTitle(str);
    } else { // display filename if no TAG
        file_menu_get_fname(vars->idx_play, str, sizeof(str) - 1);
        lcd.setTitle(str);
        /*
        file_menu_get_fname_UTF16(vars->idx_play, (char16_t*) str, sizeof(str)/2);
        lcd.setTitle(utf16_to_utf8((const char16_t*) str).c_str(), utf8);
        */
    }
    if (tag.getUTF8Album(str, sizeof(str) - 1)) lcd.setAlbum(str); else lcd.setAlbum("");
    if (tag.getUTF8Artist(str, sizeof(str) - 1)) lcd.setArtist(str); else lcd.setArtist("");
    //if (tag.getUTF8Year(str, sizeof(str) - 1)) lcd.setYear(str); else lcd.setYear("");

    {  // load image from TAG
        mime_t mime;
        ptype_t ptype;
        uint64_t pos;
        size_t size;
        bool isUnsynced;
        if (tag.getPicturePos(0, mime, ptype, pos, size, isUnsynced)) {
            //printf("found coverart mime: %d, ptype: %d, pos: %d, size: %d, isUnsynced: %d\n", mime, ptype, (int) pos, size, (int) isUnsynced);
            if (!isUnsynced && mime == jpeg && size != tagImageSize) {  // Note: judge by size to check if the image is identical to previous
                file_menu_get_fname(vars->idx_play, str, sizeof(str) - 1);
                lcd.setImageJpeg(str, pos, size);
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
                lcd.setImageJpeg(str);
                loaded = true;
                break;
            }
            idx++;
        }
        if (!loaded) { lcd.resetImage(); }
    }
    return;
}

void UIPlayMode::play()
{
    char str[FF_MAX_LFN];
    memset(str, 0, sizeof(str));
    file_menu_get_fname(vars->idx_play, str, sizeof(str) - 1);
    printf("%s\n", str);
    PlayAudio* codec = get_audio_codec();
    readTag();
    loadImageFromDir = false;
    codec->play(str, vars->fpos, vars->samples_played);
    lcd.setBitRes(codec->getBitsPerSample());
    lcd.setSampleFreq(codec->getSampFreq());
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
    lcd.switchToPlay();
}

void UIPlayMode::draw()
{
    lcd.drawPlay();
    pm_backlight_update();
    ui_clear_btn_evt();
}

//=======================================
// Implementation of UIConfigMode class
//=======================================
UIConfigMode::UIConfigMode() : UIMode("UIConfigMode", ConfigMode),
    path_stack(nullptr), idx_head(0), idx_column(0)
{
    path_stack = stack_init();
}

uint16_t UIConfigMode::getNum()
{
    return (uint16_t) configMenu.getNum() + 1;
}

const char* UIConfigMode::getStr(uint16_t idx)
{
    if (idx == 0) { return "[Back]"; }
    return configMenu.getStr((int) idx-1);
}

IconIndex_t UIConfigMode::getIconIndex(uint16_t idx)
{
    IconIndex_t iconIndex = IconIndex_t::UNDEF;
    if (idx == 0) {
        iconIndex = IconIndex_t::LEFTARROW;
    } else if (!configMenu.isSelection()) {
        iconIndex = IconIndex_t::GEAR;
    } else if (configMenu.selIdxMatched(idx-1)) {
        iconIndex = IconIndex_t::CHECKED;
    }
    return iconIndex;
}

void UIConfigMode::listIdxItems()
{
    for (int i = 0; i < vars->num_list_lines; i++) {
        if (idx_head+i >= getNum()) {
            lcd.setListItem(i, ""); // delete
            continue;
        }
        lcd.setListItem(i, getStr(idx_head+i), getIconIndex(idx_head+i), (i == idx_column));
    }
}

void UIConfigMode::idxInc(void)
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

void UIConfigMode::idxDec(void)
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
        if (configMenu.leave()) {
            stack_pop(path_stack, &stack_data);
            idx_head = stack_data.head;
            idx_column = stack_data.column;
        } else {
            return 0; // exit from Config
        }
    } else {
        if (configMenu.enter(idx_head + idx_column - 1)) {
            stack_data.head = idx_head;
            stack_data.column = idx_column;
            stack_push(path_stack, &stack_data);
            idx_head = 0;
            idx_column = 0;
        }
    }
    return 1;
}

UIMode* UIConfigMode::update()
{
    PlayAudio* codec = get_audio_codec();
    if (ui_get_btn_evt(&btn_act)) {
        vars->do_next_play = None;
        switch (btn_act) {
            case ButtonCenterSingle:
                if (select()) {
                    listIdxItems();
                } else {
                    return prevMode;
                }
                break;
            case ButtonCenterDouble:
                return prevMode;
                break;
            case ButtonCenterTriple:
                break;
            case ButtonCenterLong:
                break;
            case ButtonCenterLongLong:
                codec->getCurrentPosition(&vars->fpos, &vars->samples_played);
                codec->stop();
                lcd.setMsg("Bye");
                return getUIMode(PowerOffMode);
                break;
            case ButtonPlusSingle:
            case ButtonPlusLong:
                idxDec();
                listIdxItems();
                break;
            case ButtonMinusSingle:
            case ButtonMinusLong:
                idxInc();
                listIdxItems();
                break;
            default:
                break;
        }
        idle_count = 0;
    }
    if (idle_count > GET_CFG_MENU_GENERAL_TIME_TO_LEAVE_CONFIG*OneSec) {
        return prevMode;
    }
    idle_count++;
    return this;
}

void UIConfigMode::entry(UIMode* prevMode)
{
    UIMode::entry(prevMode);
    listIdxItems();
    lcd.switchToListView();
}

void UIConfigMode::draw()
{
    lcd.drawListView();
    pm_backlight_update();
    ui_clear_btn_evt();
}

//=======================================
// Implementation of UIPowerOffMode class
//=======================================
UIPowerOffMode::UIPowerOffMode() : UIMode("UIPowerOffMode", PowerOffMode)
{
}

void UIPowerOffMode::storeToFlash()
{
    // Save user parameters to configParam
    uint32_t seed = to_ms_since_boot(get_absolute_time());
    configParam.setU32(ConfigParam::CFG_SEED, seed);
    uint8_t volume = PlayAudio::getVolume();
    configParam.setU8(ConfigParam::CFG_VOLUME, volume);
    uint8_t stack_count = stack_get_count(dir_stack);
    configParam.setU8(ConfigParam::CFG_STACK_COUNT, stack_count);
    for (int i = 0; i < stack_count; i++) {
        stack_data_t item;
        stack_pop(dir_stack, &item);
        configParam.setU16(CFG_STACK_HEAD(i), item.head);
        configParam.setU16(CFG_STACK_COLUMN(i), item.column);
    }

    configParam.setU32(ConfigParam::CFG_UIMODE, vars->resume_ui_mode);

    configParam.setU16(ConfigParam::CFG_IDX_HEAD, vars->idx_head);
    configParam.setU16(ConfigParam::CFG_IDX_COLUMN, vars->idx_column);
    configParam.setU16(ConfigParam::CFG_IDX_PLAY, vars->idx_play);

    configParam.setU64(ConfigParam::CFG_PLAY_POS, (uint64_t) vars->fpos);
    configParam.setU32(ConfigParam::CFG_SAMPLES_PLAYED, vars->samples_played);


    // Store Configuration parameters to Flash
    configParam.finalize();
}

UIMode* UIPowerOffMode::update()
{
    ui_get_btn_evt(&btn_act); // Ignore button event
    if ((idle_count > 1*OneSec && exitType == NoError) || idle_count > 4*OneSec) {
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
        audio_codec_deinit();
    }

    lcd.switchToPowerOff();
}

void UIPowerOffMode::draw()
{
    lcd.drawPowerOff();
    ui_clear_btn_evt();
}