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
#include "ui_control.h"
#include "stack.h"
#include "PlayAudio/audio_codec.h"

const uint8_t *BLANK_LINE = ((uint8_t *) "                    ");

// UIMode class instances
LcdCanvas *UIMode::lcd = nullptr;
button_action_t UIMode::btn_act;

//================================
// Implementation of UIMode class
//================================
void UIMode::linkLcdCanvas(LcdCanvas *lcd_canvas)
{
    lcd = lcd_canvas;
}

UIMode::UIMode(const char *name, ui_mode_enm_t ui_mode_enm, UIVars *vars) : name(name), prevMode(NULL), ui_mode_enm(ui_mode_enm), vars(vars), idle_count(0)
{
}

void UIMode::entry(UIMode *prevMode)
{
    this->prevMode = prevMode;
    idle_count = 0;
    if (ui_mode_enm == vars->init_dest_ui_mode) { // Reached desitination of initial UI mode
        vars->init_dest_ui_mode = InitialMode;
    }
    ui_clear_btn_evt();
}

const char *UIMode::getName()
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
UIInitialMode::UIInitialMode(UIVars *vars) : UIMode("UIInitialMode", InitialMode, vars)
{
    file_menu_open_dir("/");
    if (file_menu_get_size() <= 1) {
        //power_off("Card Read Error!", 1);
        lcd->setMsg("Card Read Error!");
    }
}

UIMode* UIInitialMode::update()
{
    switch (vars->init_dest_ui_mode) {
        case FileViewMode:
        /*
        case PlayMode:
            return getUIMode(FileViewMode);
            break;
        */
        default:
            if (idle_count++ > 100) {
                return getUIMode(FileViewMode);
            }
            break;
    }
    idle_count++;
    return this;
}

void UIInitialMode::entry(UIMode *prevMode)
{
    UIMode::entry(prevMode);
    lcd->switchToInitial();
}

void UIInitialMode::draw()
{
    lcd->drawInitial();
}

//=========================================
// Implementation of UIFileViewMode class
//=========================================
UIFileViewMode::UIFileViewMode(UIVars *vars, stack_t *dir_stack) : UIMode("UIFileViewMode", FileViewMode, vars), dir_stack(dir_stack)
{
    sft_val = new uint16_t[vars->num_list_lines];
    for (int i = 0; i < vars->num_list_lines; i++) {
        sft_val[i] = 0;
    }
}

void UIFileViewMode::listIdxItems()
{
    char str[256];
    for (int i = 0; i < vars->num_list_lines; i++) {
        if (vars->idx_head+i >= file_menu_get_size()) {
            lcd->setListItem(i, ""); // delete
            continue;
        }
        file_menu_get_fname(vars->idx_head+i, str, sizeof(str));
        uint8_t icon = file_menu_is_dir(vars->idx_head+i) ? ICON16x16_FOLDER : ICON16x16_FILE;
        lcd->setListItem(i, str, icon, (i == vars->idx_column));
    }
}

bool UIFileViewMode::isAudioFile()
{
    /*
    uint16_t idx = vars->idx_head + vars->idx_column;
    return (
        file_menu_match_ext(idx, "mp3", 3) ||  file_menu_match_ext(idx, "MP3", 3) ||  
        file_menu_match_ext(idx, "wav", 3) ||  file_menu_match_ext(idx, "WAV", 3) ||  
        file_menu_match_ext(idx, "m4a", 3) ||  file_menu_match_ext(idx, "M4A", 3) ||  
        file_menu_match_ext(idx, "flac", 4) ||  file_menu_match_ext(idx, "FLAC", 4)
    );
    */
    return true;
}

uint16_t UIFileViewMode::getNumAudioFiles()
{
    uint16_t num_tracks = 0;
    /*
    num_tracks += file_menu_get_ext_num("mp3", 3) + file_menu_get_ext_num("MP3", 3);
    num_tracks += file_menu_get_ext_num("wav", 3) + file_menu_get_ext_num("WAV", 3);
    num_tracks += file_menu_get_ext_num("m4a", 3) + file_menu_get_ext_num("M4A", 3);
    num_tracks += file_menu_get_ext_num("flac", 4) + file_menu_get_ext_num("FLAC", 4);
    */
    return num_tracks;
}

void UIFileViewMode::chdir()
{
    stack_data_t item;
    if (vars->idx_head+vars->idx_column == 0) { // upper ("..") dirctory
        if (stack_get_count(dir_stack) > 0) {
            if (vars->fs_type == FS_EXFAT) { // This is workaround for FatFs known bug for ".." in EXFAT
                stack_t *temp_stack = stack_init();
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

#if 0
UIMode *UIFileViewMode::nextPlay()
{
    switch (USERCFG_PLAY_NEXT_ALBUM) {
        case UserConfig::next_play_action_t::Sequential:
            return sequentialSearch(false);
            break;
        case UserConfig::next_play_action_t::SequentialRepeat:
            return sequentialSearch(true);
            break;
        case UserConfig::next_play_action_t::Repeat:
            vars->idx_head = 0;
            vars->idx_column = 0;
            return getUIPlayMode();
        case UserConfig::next_play_action_t::Random:
            return randomSearch(USERCFG_PLAY_RAND_DEPTH);
            break;
        case UserConfig::next_play_action_t::Stop:
        default:
            return this;
            break;
    }
    return this;
}

UIMode *UIFileViewMode::sequentialSearch(bool repeatFlg)
{
    int stack_count;
    uint16_t last_dir_idx;

    printf("Sequential Search\n");
    vars->idx_play = 0;
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
                    if (vars->idx_head >= file_menu_get_size()) { vars->idx_head = 1; }
                } else {
                    // [Option 2] stop at the bottom of albums
                    if (vars->idx_head >= file_menu_get_size()) {
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
        if (stack_count == stack_get_count(dir_stack) && getNumAudioFiles() > 0) { break; }
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

UIMode *UIFileViewMode::randomSearch(uint16_t depth)
{
    int i;
    int stack_count;

    printf("Random Search\n");
    vars->idx_play = 0;
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
                vars->idx_head = random(1, file_menu_get_size());
                file_menu_sort_entry(vars->idx_head, vars->idx_head+1);
                if (file_menu_is_dir(vars->idx_head) > 0) { break; }
            }
            vars->idx_column = 0;
            chdir();
        }
        // Check if Next Target Dir has Audio track files
        if (stack_count == stack_get_count(dir_stack) && getNumAudioFiles() > 0) { break; }
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
#endif

void UIFileViewMode::idxInc(void)
{
    if (vars->idx_head >= file_menu_get_size() - vars->num_list_lines && vars->idx_column == vars->num_list_lines-1) { return; }
    if (vars->idx_head + vars->idx_column + 1 >= file_menu_get_size()) { return; }
    vars->idx_column++;
    if (vars->idx_column >= vars->num_list_lines) {
        if (vars->idx_head + vars->num_list_lines >= file_menu_get_size() - vars->num_list_lines) {
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
    if (vars->idx_head >= file_menu_get_size() - vars->num_list_lines && vars->idx_column == vars->num_list_lines-1) { return; }
    if (vars->idx_head + vars->idx_column + 1 >= file_menu_get_size()) { return; }
    if (file_menu_get_size() < vars->num_list_lines) {
        idxInc();
    } else if (vars->idx_head + vars->num_list_lines >= file_menu_get_size() - vars->num_list_lines) {
        vars->idx_head = file_menu_get_size() - vars->num_list_lines;
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

#if 0
UIMode *UIFileViewMode::getUIPlayMode()
{
    if (vars->idx_play == 0) {
        vars->idx_play = vars->idx_head + vars->idx_column;
    }
    vars->num_tracks = getNumAudioFiles();
    return getUIMode(PlayMode);
}
#endif

UIMode* UIFileViewMode::update()
{
    vars->resume_ui_mode = ui_mode_enm;
    /*
    switch (vars->init_dest_ui_mode) {
        case PlayMode:
            return getUIPlayMode();
            break;
        default:
            break;
    }
    */
    if (ui_get_btn_evt(&btn_act)) {
        vars->do_next_play = None;
        switch (btn_act) {
            case ButtonCenterSingle:
                if (file_menu_is_dir(vars->idx_head+vars->idx_column) > 0) { // Target is Directory
                    chdir();
                    listIdxItems();
                } else { // Target is File
                    char str[256];
                    file_menu_full_sort();
                    file_menu_get_fname(vars->idx_head+vars->idx_column, str, sizeof(str));
                    printf("%s\n", str);
                    PlayAudio *playAudio = getAudioCodec(AUDIO_CODEC_WAV);
                    playAudio->play(str);
                    /*
                    if (isAudioFile()) {
                        return getUIPlayMode();
                    }
                    */
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
                //return randomSearch(USERCFG_PLAY_RAND_DEPTH);
                break;
            case ButtonCenterLong:
                //return getUIMode(ConfigMode);
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
    /*
    switch (vars->do_next_play) {
        case ImmediatePlay:
            vars->do_next_play = None;
            return randomSearch(USERCFG_PLAY_RAND_DEPTH);
            break;
        case TimeoutPlay:
            if (idle_count > USERCFG_PLAY_TM_NXT_PLY*OneSec) {
                vars->do_next_play = None;
                return nextPlay();
            }
            break;
        default:
            break;
    }
    */
    /*
    if (idle_count > USERCFG_GEN_TM_PWROFF*OneSec) {
        return getUIMode(PowerOffMode);
    } else if (idle_count > 5*OneSec) {
        file_menu_idle(); // for background sort
    }
    lcd->setBatteryVoltage(vars->bat_mv);
    */
    idle_count++;
    return this;
}

void UIFileViewMode::entry(UIMode *prevMode)
{
    UIMode::entry(prevMode);
    listIdxItems();
    lcd->switchToListView();
}

void UIFileViewMode::draw()
{
    lcd->drawListView();
    ui_clear_btn_evt();
}

#if 0
//====================================
// Implementation of UIPlayMode class
//====================================
UIPlayMode::UIPlayMode(UIVars *vars) : UIMode("UIPlayMode", PlayMode, vars)
{
}

UIMode* UIPlayMode::update()
{
    vars->resume_ui_mode = ui_mode_enm;
    AudioCodec *codec = audio_get_codec();
    if (getBtnEvt()) {
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
                audio_volume_up();
                break;
            case ButtonMinusSingle:
            case ButtonMinusLong:
                audio_volume_down();
                break;
            default:
                break;
        }
        idle_count = 0;
    }
    if (codec->isPaused() && idle_count > USERCFG_GEN_TM_PWROFF*OneSec) {
        return getUIMode(PowerOffMode);
    } else if (!codec->isPlaying() || (codec->positionMillis() + 500 > codec->lengthMillis())) {
        MutexFsBaseFile file;
        vars->idx_play++;
        idle_count = 0;
        audio_codec_enm_t next_audio_codec_enm = getAudioCodec(&file);
        if (next_audio_codec_enm != CodecNone) {
            readTag();
            while (codec->isPlaying()) { /*delay(1);*/ } // minimize gap between tracks
            audio_set_codec(next_audio_codec_enm);
            audio_play(&file);
            lcd->switchToPlay();
        } else {
            while (codec->isPlaying()) { delay(1); }
            codec->stop();
            vars->do_next_play = TimeoutPlay;
            return getUIMode(FileViewMode);
        }
    }
    lcd->setVolume(audio_get_volume());
    lcd->setBitRate(codec->bitRate());
    lcd->setPlayTime(codec->positionMillis()/1000, codec->lengthMillis()/1000, codec->isPaused());
    lcd->setBatteryVoltage(vars->bat_mv);
    idle_count++;
    return this;
}

audio_codec_enm_t UIPlayMode::getAudioCodec(MutexFsBaseFile *f)
{
    audio_codec_enm_t audio_codec_enm = CodecNone;
    bool flg = false;
    int ofs = 0;
    char str[256];

    while (vars->idx_play + ofs < file_menu_get_size()) {
        file_menu_get_obj(vars->idx_play + ofs, f);
        f->getName(str, sizeof(str));
        char* ext_pos = strrchr(str, '.');
        if (ext_pos) {
            if (strncmp(ext_pos, ".mp3", 4) == 0 || strncmp(ext_pos, ".MP3", 4) == 0) {
                audio_codec_enm = CodecMp3;
                flg = true;
                break;
            } else if (strncmp(ext_pos, ".wav", 4) == 0 || strncmp(ext_pos, ".WAV", 4) == 0) {
                audio_codec_enm = CodecWav;
                flg = true;
                break;
            } else if (strncmp(ext_pos, ".m4a", 4) == 0 || strncmp(ext_pos, ".M4A", 4) == 0) {
                audio_codec_enm = CodecAac;
                flg = true;
                break;
            } else if (strncmp(ext_pos, ".flac", 5) == 0 || strncmp(ext_pos, ".FLAC", 5) == 0) {
                audio_codec_enm = CodecFlac;
                flg = true;
                break;
            }
        }
        ofs++;
    }
    if (flg) {
        file_menu_get_fname_UTF16(vars->idx_play + ofs, (char16_t *) str, sizeof(str)/2);
        Serial.println(utf16_to_utf8((const char16_t *) str).c_str());
        vars->idx_play += ofs;
    } else {
        vars->idx_play = 0;
    }
    return audio_codec_enm;
}

void UIPlayMode::readTag()
{
    char str[256];
    mime_t mime;
    ptype_t ptype;
    uint64_t img_pos;
    size_t size;
    bool is_unsync;
    int img_cnt = 0;
    
    // Read TAG
    tag.loadFile(vars->idx_play);

    // copy TAG text
    if (tag.getUTF8Track(str, sizeof(str))) {
        uint16_t track = atoi(str);
        sprintf(str, "%d / %d", track, vars->num_tracks);
    } else {
        sprintf(str, "%d / %d", vars->idx_play, vars->num_tracks);
    }
    lcd->setTrack(str);
    if (tag.getUTF8Title(str, sizeof(str))) {
        lcd->setTitle(str, utf8);
    } else { // display filename if no TAG
        file_menu_get_fname_UTF16(vars->idx_play, (char16_t *) str, sizeof(str)/2);
        lcd->setTitle(utf16_to_utf8((const char16_t *) str).c_str(), utf8);
    }
    if (tag.getUTF8Album(str, sizeof(str))) lcd->setAlbum(str, utf8); else lcd->setAlbum("");
    if (tag.getUTF8Artist(str, sizeof(str))) lcd->setArtist(str, utf8); else lcd->setArtist("");
    if (tag.getUTF8Year(str, sizeof(str))) lcd->setYear(str, utf8); else lcd->setYear("");

    // copy TAG image
    lcd->deleteAlbumArt();
    for (int i = 0; i < tag.getPictureCount(); i++) {
        if (tag.getPicturePos(i, &mime, &ptype, &img_pos, &size, &is_unsync)) {
            if (mime == jpeg) { lcd->addAlbumArtJpeg(vars->idx_play, img_pos, size, is_unsync); img_cnt++; }
            else if (mime == png) { lcd->addAlbumArtPng(vars->idx_play, img_pos, size, is_unsync); img_cnt++; }
        }
    }
    // if no AlbumArt in TAG, use JPEG or PNG in current folder
    if (img_cnt == 0) {
        uint16_t idx = 0;
        while (idx < file_menu_get_size()) {
            MutexFsBaseFile f;
            file_menu_get_obj(idx, &f);
            if (file_menu_match_ext(idx, "jpg", 3) || file_menu_match_ext(idx, "JPG", 3) || 
                file_menu_match_ext(idx, "jpeg", 4) || file_menu_match_ext(idx, "JPEG", 4)) {
                lcd->addAlbumArtJpeg(idx, 0, f.fileSize());
                img_cnt++;
            } else if (file_menu_match_ext(idx, "png", 3) || file_menu_match_ext(idx, "PNG", 3)) {
                lcd->addAlbumArtPng(idx, 0, f.fileSize());
                img_cnt++;
            }
            idx++;
        }
    }
}

void UIPlayMode::entry(UIMode *prevMode)
{
    MutexFsBaseFile file;
    UIMode::entry(prevMode);
    if (prevMode->getUIModeEnm() != ConfigMode) {
        audio_set_codec(getAudioCodec(&file));
        readTag();
        audio_play(&file);
    }
    lcd->switchToPlay();
}

void UIPlayMode::draw()
{
    lcd->drawPlay();
    ui_clear_btn_evt();
}

//=======================================
// Implementation of UIConfigMode class
//=======================================
UIConfigMode::UIConfigMode(UIVars *vars) : UIMode("UIConfigMode", ConfigMode, vars),
    path_stack(NULL), idx_head(0), idx_column(0)
{
    path_stack = stack_init();
}

uint16_t UIConfigMode::getNum()
{
    return (uint16_t) userConfig.getNum() + 1;
}

const char *UIConfigMode::getStr(uint16_t idx)
{
    if (idx == 0) { return "[Back]"; }
    return userConfig.getStr((int) idx-1);
}

const uint8_t *UIConfigMode::getIcon(uint16_t idx)
{
    const uint8_t *icon = NULL;
    if (idx == 0) {
        icon = ICON16x16_LEFTARROW;
    } else if (!userConfig.isSelection()) {
        icon = ICON16x16_GEAR;
    } else if (userConfig.selIdxMatched(idx-1)) {
        icon = ICON16x16_CHECKED;
    }
    return icon;
}

void UIConfigMode::listIdxItems()
{
    for (int i = 0; i < vars->num_list_lines; i++) {
        if (idx_head+i >= getNum()) {
            lcd->setListItem(i, ""); // delete
            continue;
        }
        lcd->setListItem(i, getStr(idx_head+i), getIcon(idx_head+i), (i == idx_column));
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
        if (userConfig.leave()) {
            stack_pop(path_stack, &stack_data);
            idx_head = stack_data.head;
            idx_column = stack_data.column;
        } else {
            return 0; // exit from Config
        }
    } else {
        if (userConfig.enter(idx_head + idx_column - 1)) {
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
    if (getBtnEvt()) {
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
    if (idle_count > USERCFG_GEN_TM_CONFIG*OneSec) {
        return prevMode;
    }
    idle_count++;
    return this;
}

void UIConfigMode::entry(UIMode *prevMode)
{
    UIMode::entry(prevMode);
    listIdxItems();
    lcd->switchToListView();
}

void UIConfigMode::draw()
{
    lcd->drawListView();
    ui_clear_btn_evt();
}

//=======================================
// Implementation of UIPowerOffMode class
//=======================================
UIPowerOffMode::UIPowerOffMode(UIVars *vars) : UIMode("UIPowerOffMode", PowerOffMode, vars)
{
}

UIMode* UIPowerOffMode::update()
{
    return this;
}

void UIPowerOffMode::entry(UIMode *prevMode)
{
    UIMode::entry(prevMode);
    lcd->switchToPowerOff(vars->power_off_msg);
    lcd->drawPowerOff();
    ui_terminate(vars->resume_ui_mode);
}

void UIPowerOffMode::draw()
{
}
#endif