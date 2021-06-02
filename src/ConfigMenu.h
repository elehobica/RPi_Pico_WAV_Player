/*-----------------------------------------------------------/
/ ConfigMenu.h
/------------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/-----------------------------------------------------------*/

#ifndef __CONFIG_MENU_H_INCLUDED__
#define __CONFIG_MENU_H_INCLUDED__

#include <stdint.h>

#define configMenu				ConfigMenu::instance()

#define USERCFG_GEN_TM_PWROFF	(configMenu.getValue(0, 0))
#define USERCFG_GEN_TM_CONFIG	(configMenu.getValue(0, 1))
#define USERCFG_DISP_ROTATION	(configMenu.getValue(1, 0))
#define USERCFG_DISP_BLL		(configMenu.getValue(1, 1))
#define USERCFG_DISP_BLH		(configMenu.getValue(1, 2))
#define USERCFG_DISP_TM_BLL		(configMenu.getValue(1, 3))
#define USERCFG_PLAY_TM_NXT_PLY	(configMenu.getValue(2, 0))
#define USERCFG_PLAY_NEXT_ALBUM	(configMenu.getValue(2, 1))
#define USERCFG_PLAY_RAND_DEPTH	(configMenu.getValue(2, 2))

//=================================
// Interface of Hook Functions
//=================================
void hook_disp_rotation();

//=================================
// Interface of ConfigMenu class
//=================================
class ConfigMenu
{
public:
	typedef enum {
		Stop = 0,
		Sequential,
		SequentialRepeat,
		Repeat,
		Random
	} next_play_action_t;

	typedef struct {
		const char	*name;
		int			value;
	} config_sel_t;

	typedef struct {
		const char		*name;
		int				flash_addr;
		config_sel_t	*selection;
		int				sel_idx;
		int				size;
		void 			(*hook_func)();
	} config_item_t;

	typedef struct {
		const char		*name;
		config_item_t	*items;
		uint16_t		size;
	} config_category_t;

	static ConfigMenu& instance(); // Singleton
	// For Read Users
	int getValue(int category_idx, int item_idx);
	// For Flash Load/Store
	void scanSelIdx(void (*yield_func)(int adrs, int *val), bool do_hook_func = false);
	//int getSelIdx(int category_idx, int item_idx);
	//void setSelIdx(int category_idx, int item_idx, int sel_idx);

	// Menu Utilities
	int getNum();
	const char *getStr(int idx);
	bool enter(int idx);
	bool leave();
	bool isSelection();
	bool selIdxMatched(int idx);

private:
	ConfigMenu();
	~ConfigMenu();
	ConfigMenu(const ConfigMenu&) = delete;
	ConfigMenu& operator=(const ConfigMenu&) = delete;
	config_sel_t sel_time0[3] = {
		{"3 min", 3*60},
		{"5 min", 5*60},
		{"10 min", 10*60}
	};
	config_sel_t sel_time1[7] = {
		{"10 sec", 10},
		{"20 sec", 20},
		{"30 sec", 30},
		{"1 min", 1*60},
		{"3 min", 3*60},
		{"5 min", 5*60},
		{"Forever", -1}
	};
	config_sel_t sel_time2[5] = {
		{"15 sec", 15},
		{"30 sec", 30},
		{"1 min", 1*60},
		{"2 min", 2*60},
		{"3 min", 3*60}
	};
	config_sel_t sel_rotation[2] = {
		{"0 deg", 0},
		{"180 deg", 2}
	};
	config_sel_t sel_backlight_level[16] = {
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
		{"256", 256}
	};
	config_sel_t sel_next_play_album[5] = {
		{"Stop", Stop},
		{"Sequential", Sequential},
		{"SequentialRepeat", SequentialRepeat},
		{"Repeat", Repeat},
		{"Random", Random}
	};
	config_sel_t sel_rand_dir_depth[4] = {
		{"1", 1},
		{"2", 2},
		{"3", 3},
		{"4", 4}
	};
	#define sz_sel(x)	(sizeof(x)/sizeof(config_sel_t))
			
	config_item_t items_general[2] = {
	//	Name						Flash		selection					sel_idx	 	size							hook_func
		{"Time to Power Off",		0,			sel_time0,					0,			sz_sel(sel_time0),				nullptr},
		{"Time to Leave Config",	0,			sel_time2,					1,			sz_sel(sel_time2),				nullptr}
	};
	config_item_t items_display[4] = {
	//	Name						Flash		selection					sel_idx		size
		{"Rotation",				0,			sel_rotation,				0,			sz_sel(sel_rotation),			hook_disp_rotation},
		{"Backlight Low Level",		0,			sel_backlight_level,		7,			sz_sel(sel_backlight_level),	nullptr},
		{"Backlight High Level",	0,			sel_backlight_level,		15,			sz_sel(sel_backlight_level),	nullptr},
		{"Time to Backlight Low",	0,			sel_time1,					1,			sz_sel(sel_time1),				nullptr}
	};
	config_item_t items_play[3] = {
	//	Name						Flash		selection					sel_idx		size
		{"Time to Next Play",		0,			sel_time2, 					2,			sz_sel(sel_time2),				nullptr},
		{"Next Play Album",			0,			sel_next_play_album, 		1,			sz_sel(sel_next_play_album),	nullptr},
		{"Random Dir Depth",		0,			sel_rand_dir_depth,			1,			sz_sel(sel_rand_dir_depth),		nullptr}
	};
	#define sz_item(x)	(sizeof(x)/sizeof(config_item_t))

	config_category_t category[3] = {
	// 	name 			items				size
		{"General", 	items_general, 		sz_item(items_general)},
		{"Display",		items_display, 		sz_item(items_display)},
		{"Play", 		items_play, 		sz_item(items_play)}
	};
	const int sz_category = sizeof(category)/sizeof(config_category_t);

	int level; // 0: Top, 1: Category, 2: Item
    config_category_t *cur_category;
    config_item_t *cur_item;
};

#endif // __CONFIG_MENU_H_INCLUDED__
