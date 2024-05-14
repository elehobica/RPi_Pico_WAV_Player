/*------------------------------------------------------/
/ UIMode
/-------------------------------------------------------/
/ Copyright (c) 2020-2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#pragma once

#include "ConfigMenu.h"
#include "file_menu_FatFs.h"
#include "LcdCanvasIconDef.h"
#include "ui_control.h"

#include <stack>

typedef enum {
    None = 0,
    ImmediatePlay,
    TimeoutPlay
} do_next_play_t;

typedef enum {
    SequentialPlay = 0,
    RandomPlay,
} next_play_type_t;

struct UIVars
{
    uint8_t fs_type = FS_FAT32;
    uint16_t num_list_lines = 1;
    ui_mode_enm_t init_dest_ui_mode = InitialMode;
    ui_mode_enm_t resume_ui_mode = FileViewMode;
    uint16_t idx_head = 0;
    uint16_t idx_column = 0;
    uint16_t idx_play = 0;
    uint16_t num_tracks = 0;
    do_next_play_t do_next_play = None;
    next_play_type_t next_play_type = RandomPlay;
    size_t fpos = 0;
    uint32_t samples_played = 0;
};

//===========================
// Interface of UIMode class
//===========================
class UIMode
{
public:
    static constexpr int UpdateCycleMs = 50; // loop cycle (ms) (= LoopCycleMs value in arduino_main.cpp)
    static void initialize(UIVars* vars);
    static UIMode* getUIMode(const ui_mode_enm_t& ui_mode_enm);
    UIMode(const char* name, const ui_mode_enm_t& ui_mode_enm);
    virtual UIMode* update() = 0;
    virtual void entry(UIMode* prevMode);
    virtual void draw() const = 0;
    static std::array<UIMode*, NUM_UI_MODES> ui_mode_ary;
    const char* getName() const;
    ui_mode_enm_t getUIModeEnm() const;
    uint16_t getIdleCount() const;
protected:
    typedef enum {
        NoError = 0,
        FatFsError,
        LowBatteryVoltage
    } ExitType;
    static constexpr int OneSec = 1000 / UpdateCycleMs; // 1 Sec
    static constexpr int OneMin = 60 * OneSec; // 1 Min
    static button_action_t btn_act;
    static button_unit_t btn_unit;
    static UIVars* vars;
    static std::stack<stack_data_t> dir_stack;
    static ExitType exitType;
    static ConfigMenu& cfgMenu;
    static ConfigParam& cfgParam;
    bool isAudioFile(const uint16_t& idx) const;
    const char* name;
    UIMode* prevMode = nullptr;
    ui_mode_enm_t ui_mode_enm;
    uint16_t idle_count = 0;
};

//===================================
// Interface of UIInitialMode class
//===================================
class UIInitialMode : public UIMode
{
public:
    UIInitialMode();
    UIMode* update();
    void entry(UIMode* prevMode);
    void draw() const;
protected:
    void loadFromFlash() const;
};

//===================================
// Interface of UIChargeMode class
//===================================
class UIChargeMode : public UIMode
{
public:
    UIChargeMode();
    UIMode* update();
    void entry(UIMode* prevMode);
    void draw() const;
};

//===================================
// Interface of UIOpeningMode class
//===================================
class UIOpeningMode : public UIMode
{
public:
    UIOpeningMode();
    UIMode* update();
    void entry(UIMode* prevMode);
    void draw() const;
protected:
    void restoreFromFlash() const;
};

//===================================
// Interface of UIFileViewMode class
//===================================
class UIFileViewMode : public UIMode
{
public:
    UIFileViewMode();
    UIMode* update();
    void entry(UIMode* prevMode);
    void draw() const;
protected:
    uint16_t* sft_val;
    void listIdxItems();
    uint16_t getNumAudioFiles() const;
    void chdir() const;
    UIMode* nextPlay();
    UIMode* sequentialSearch(const bool& repeatFlg);
    UIMode* randomSearch(const uint16_t& depth);
    void findFirstAudioTrack() const;
    void idxInc() const;
    void idxDec() const;
    void idxFastInc() const;
    void idxFastDec() const;
    UIMode* getUIPlayMode();
};

//===============================
// Interface of UIPlayMode class
//===============================
class UIPlayMode : public UIMode
{
public:
    UIPlayMode();
    UIMode* update();
    void entry(UIMode* prevMode);
    void draw() const;
protected:
    size_t tagImageSize = 0;
    bool loadImageFromDir = true;
    void play();
    //audio_codec_enm_t getAudioCodec(MutexFsBaseFile* f);
    void readTag();
};

//===================================
// Interface of UIConfigMode class
//===================================
class UIConfigMode : public UIMode
{
public:
    UIConfigMode();
    UIMode* update();
    void entry(UIMode* prevMode);
    void draw() const;
protected:
    std::stack<stack_data_t> path_stack;
    uint16_t idx_head;
    uint16_t idx_column;
    uint16_t getNum() const;
    const char* getStr(const uint16_t& idx) const;
    IconIndex_t getIconIndex(const uint16_t& idx) const;
    void listIdxItems() const;
    void idxInc();
    void idxDec();
    int select();
};

//===================================
// Interface of UIPowerOffMode class
//===================================
class UIPowerOffMode : public UIMode
{
public:
    UIPowerOffMode();
    UIMode* update();
    void entry(UIMode* prevMode);
    void draw() const;
protected:
    void storeToFlash() const;
};
