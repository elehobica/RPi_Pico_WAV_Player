/*-----------------------------------------------------------/
/ ConfigParam.h
/------------------------------------------------------------/
/ Copyright (c) 2021, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/-----------------------------------------------------------*/

#ifndef __CONFIG_PARAM_H_INCLUDED__
#define __CONFIG_PARAM_H_INCLUDED__

#include <stdint.h>
#include <stdlib.h>

#define configParam				(ConfigParam::ConfigParamClass::instance())

#define	CFG_STACK_HEAD(x)		((x == 4) ? ConfigParam::CFG_STACK_HEAD4 : (x == 3) ? ConfigParam::CFG_STACK_HEAD3 : (x == 2) ? ConfigParam::CFG_STACK_HEAD2 : (x == 1) ? ConfigParam::CFG_STACK_HEAD1 : ConfigParam::CFG_STACK_HEAD0)
#define	CFG_STACK_COLUMN(x)		((x == 4) ? ConfigParam::CFG_STACK_COLUMN4 : (x == 3) ? ConfigParam::CFG_STACK_COLUMN3 : (x == 2) ? ConfigParam::CFG_STACK_COLUMN2 : (x == 1) ? ConfigParam::CFG_STACK_COLUMN1 : ConfigParam::CFG_STACK_COLUMN0)

#define GETBOOL(identifier)		(configParam.getBool(identifier))
#define GETU8(identifier)		(configParam.getU8(identifier))
#define GETU16(identifier)		(configParam.getU16(identifier))
#define GETU32(identifier)		(configParam.getU32(identifier))
#define GETU64(identifier)		(configParam.getU64(identifier))
#define GETI8(identifier)		(configParam.getI8(identifier))
#define GETI16(identifier)		(configParam.getI16(identifier))
#define GETI32(identifier)		(configParam.getI32(identifier))
#define GETI64(identifier)		(configParam.getI64(identifier))
#define GETSTR(identifier)		(configParam.getStr(identifier))

#define GET_CFG_BOOT_COUNT		GETU32(ConfigParam::CFG_BOOT_COUNT)
#define GET_CFG_SEED			GETU32(ConfigParam::CFG_BOOT_COUNT)
#define GET_CFG_VOLUME			GETU8(ConfigParam::CFG_VOLUME)
#define GET_CFG_STACK_COUNT		GETU8(ConfigParam::CFG_STACK_COUNT)
#define GET_CFG_STACK_HEAD(x)	GETU16(CFG_STACK_HEAD(x))
#define GET_CFG_STACK_COLUMN(x) GETU16(CFG_STACK_COLUMN(x))
#define GET_CFG_UIMODE			GETU32(ConfigParam::CFG_UIMODE)
#define GET_CFG_IDX_HEAD		GETU16(ConfigParam::CFG_IDX_HEAD)
#define GET_CFG_IDX_COLUMN		GETU16(ConfigParam::CFG_IDX_COLUMN)
#define GET_CFG_IDX_PLAY		GETU16(ConfigParam::CFG_IDX_PLAY)
#define GET_CFG_PLAY_POS		GETU64(ConfigParam::CFG_PLAY_POS)
#define GET_CFG_SAMPLES_PLAYED	GETU32(ConfigParam::CFG_SAMPLES_PLAYED)

//=================================
// Interface of ConfigMenu class
//=================================
namespace ConfigParam
{
	typedef enum {
		LOAD_DEFAULT_IF_FLASH_IS_BLANK = 0,
		FORCE_LOAD_DEFAULT,
		ALWAYS_LOAD_FROM_FLASH
	} LoadDefaultBehavior_t;

	typedef enum {
		CFG_BOOT_COUNT = 0,
		CFG_SEED,
		CFG_VOLUME,
		CFG_STACK_COUNT,
		CFG_STACK_HEAD0,
		CFG_STACK_COLUMN0,
		CFG_STACK_HEAD1,
		CFG_STACK_COLUMN1,
		CFG_STACK_HEAD2,
		CFG_STACK_COLUMN2,
		CFG_STACK_HEAD3,
		CFG_STACK_COLUMN3,
		CFG_STACK_HEAD4,
		CFG_STACK_COLUMN4,
		CFG_UIMODE,
		CFG_IDX_HEAD,
		CFG_IDX_COLUMN,
		CFG_IDX_PLAY,
		CFG_PLAY_POS,
		CFG_SAMPLES_PLAYED,
		CFG_MENU_IDX_GENERAL_TIME_TO_POWER_OFF,
		CFG_MENU_IDX_GENERAL_TIME_TO_LEAVE_CONFIG,
		CFG_MENU_IDX_DISPLAY_ROTATION,
		CFG_MENU_IDX_DISPLAY_BACKLIGHT_LOW_LEVEL,
		CFG_MENU_IDX_DISPLAY_BACKLIGHT_HIGH_LEVEL,
		CFG_MENU_IDX_DISPLAY_TIME_TO_BACKLIGHT_LOW,
		CFG_MENU_IDX_PLAY_TIME_TO_NEXT_PLAY,
		CFG_MENU_IDX_PLAY_NEXT_PLAY_ALBUM,
		CFG_MENU_IDX_PLAY_RANDOM_DIR_DEPTH,
		NUM_CFG_PARAMS
	} ParamID_t;

	typedef enum {
		CFG_NONE_T,
		CFG_BOOL_T,
		CFG_STRING_T,
		CFG_UINT8_T,
		CFG_UINT16_T,
		CFG_UINT32_T,
		CFG_UINT64_T,
		CFG_INT8_T,
		CFG_INT16_T,
		CFG_INT32_T,
		CFG_INT64_T
	} ParamType_t;

	typedef struct ParamItem {
		const ParamID_t id;
		const char *name;
		const ParamType_t paramType;
		const size_t size;
		const char *defaultValue;
		const uint32_t flashAddr;
		void *ptr;
	} ParamItem_t;

	class ConfigParamClass
	{
	public:
		static ConfigParamClass& instance(); // Singleton
		void printInfo();
		void initialize(LoadDefaultBehavior_t loadDefaultBehavior = LOAD_DEFAULT_IF_FLASH_IS_BLANK);
		void finalize();
		void incBootCount();
		void read(ParamID_t id, void *ptr);
		void write(ParamID_t id, const void *ptr);
		bool getBool(ParamID_t id);
		uint8_t getU8(ParamID_t id);
		uint16_t getU16(ParamID_t id);
		uint32_t getU32(ParamID_t id);
		uint64_t getU64(ParamID_t id);
		int8_t getI8(ParamID_t id);
		int16_t getI16(ParamID_t id);
		int32_t getI32(ParamID_t id);
		int64_t getI64(ParamID_t id);
		char *getStr(ParamID_t id);
		void setU8(ParamID_t id, uint8_t val);
		void setU16(ParamID_t id, uint16_t val);
		void setU32(ParamID_t id, uint32_t val);
		void setU64(ParamID_t id, uint64_t val);
		void setI8(ParamID_t id, int8_t val);
		void setI16(ParamID_t id, int16_t val);
		void setI32(ParamID_t id, int32_t val);
		void setI64(ParamID_t id, int64_t val);
		void setStr(ParamID_t id, char *str);
	private:
		ParamItem_t configParamItems[NUM_CFG_PARAMS] = {
		//	id						name					type			size	default		ofs		ptr
			{CFG_BOOT_COUNT,		"CFG_BOOT_COUNT",		CFG_UINT32_T,	4, 		"0",		0x000,	nullptr},
			{CFG_SEED,				"CFG_SEED",				CFG_UINT32_T,	4,		"0",		0x004,	nullptr},
			{CFG_VOLUME,			"CFG_VOLUME",			CFG_UINT8_T,	1,		"65",		0x008,	nullptr},
			{CFG_STACK_COUNT,		"CFG_STACK_COUNT", 		CFG_UINT8_T,	1,		"0",		0x009,	nullptr},
			{CFG_STACK_HEAD0,		"CFG_STACK_HEAD0",		CFG_UINT16_T,	2,		"0",		0x010,	nullptr},
			{CFG_STACK_COLUMN0,		"CFG_STACK_COLUMN0",	CFG_UINT16_T,	2,		"0",		0x012,	nullptr},
			{CFG_STACK_HEAD1,		"CFG_STACK_HEAD1",		CFG_UINT16_T,	2,		"0",		0x014,	nullptr},
			{CFG_STACK_COLUMN1,		"CFG_STACK_COLUMN1",	CFG_UINT16_T,	2,		"0",		0x016,	nullptr},
			{CFG_STACK_HEAD2,		"CFG_STACK_HEAD2",		CFG_UINT16_T,	2,		"0",		0x018,	nullptr},
			{CFG_STACK_COLUMN2,		"CFG_STACK_COLUMN2",	CFG_UINT16_T,	2,		"0",		0x01a,	nullptr},
			{CFG_STACK_HEAD3,		"CFG_STACK_HEAD3",		CFG_UINT16_T,	2,		"0",		0x01c,	nullptr},
			{CFG_STACK_COLUMN3,		"CFG_STACK_COLUMN3",	CFG_UINT16_T,	2,		"0",		0x020,	nullptr},
			{CFG_STACK_HEAD4,		"CFG_STACK_HEAD4",		CFG_UINT16_T,	2,		"0",		0x022,	nullptr},
			{CFG_STACK_COLUMN4,		"CFG_STACK_COLUMN4",	CFG_UINT16_T,	2,		"0",		0x024,	nullptr},
			{CFG_UIMODE,			"CFG_UIMODE",			CFG_UINT32_T,	4,		"0",		0x028,	nullptr},
			{CFG_IDX_HEAD,			"CFG_IDX_HEAD",			CFG_UINT16_T,	2,		"0",		0x02c,	nullptr},
			{CFG_IDX_COLUMN,		"CFG_IDX_COLUMN",		CFG_UINT16_T,	2,		"0",		0x02e,	nullptr},
			{CFG_IDX_PLAY,			"CFG_IDX_PLAY",			CFG_UINT16_T,	2,		"0",		0x030,	nullptr},
			{CFG_PLAY_POS,			"CFG_PLAY_POS",			CFG_UINT64_T,	8,		"0",		0x038,	nullptr},
			{CFG_SAMPLES_PLAYED,	"CFG_SAMPLES_PLAYED",	CFG_UINT32_T,	4,		"0",		0x040,	nullptr},
			// type of CFG_MENU must be "CFG_UINT32_T" and default values indicates index of selection (see ConfigMenu.h)
			{CFG_MENU_IDX_GENERAL_TIME_TO_POWER_OFF,		"CFG_MENU_IDX_GENERAL_TIME_TO_POWER_OFF",		CFG_UINT32_T,	4,	"0",	0x080,	nullptr},
			{CFG_MENU_IDX_GENERAL_TIME_TO_LEAVE_CONFIG,		"CFG_MENU_IDX_GENERAL_TIME_TO_LEAVE_CONFIG",	CFG_UINT32_T,	4,	"1",	0x084,	nullptr},
			{CFG_MENU_IDX_DISPLAY_ROTATION,          		"CFG_MENU_IDX_DISPLAY_ROTATION",				CFG_UINT32_T,	4,	"0",	0x088,	nullptr},
			{CFG_MENU_IDX_DISPLAY_BACKLIGHT_LOW_LEVEL,		"CFG_MENU_IDX_DISPLAY_BACKLIGHT_LOW_LEVEL",		CFG_UINT32_T,	4,	"7",	0x08c,	nullptr},
			{CFG_MENU_IDX_DISPLAY_BACKLIGHT_HIGH_LEVEL,		"CFG_MENU_IDX_DISPLAY_BACKLIGHT_HIGH_LEVEL",	CFG_UINT32_T,	4,	"15",	0x090,	nullptr},
			{CFG_MENU_IDX_DISPLAY_TIME_TO_BACKLIGHT_LOW,	"CFG_MENU_IDX_DISPLAY_TIME_TO_BACKLIGHT_LOW",	CFG_UINT32_T,	4,	"1",	0x094,	nullptr},
			{CFG_MENU_IDX_PLAY_TIME_TO_NEXT_PLAY,    		"CFG_MENU_IDX_PLAY_TIME_TO_NEXT_PLAY",			CFG_UINT32_T,	4,	"2",	0x098,	nullptr},
			{CFG_MENU_IDX_PLAY_NEXT_PLAY_ALBUM,      		"CFG_MENU_IDX_PLAY_NEXT_PLAY_ALBUM",			CFG_UINT32_T,	4,	"1",	0x09c,	nullptr},
			{CFG_MENU_IDX_PLAY_RANDOM_DIR_DEPTH,     		"CFG_MENU_IDX_PLAY_RANDOM_DIR_DEPTH",			CFG_UINT32_T,	4,	"1",	0x0a0,	nullptr},
		};

		ConfigParamClass();
		~ConfigParamClass();
		ConfigParamClass(const ConfigParamClass&) = delete;
		ConfigParamClass& operator=(const ConfigParamClass&) = delete;
		void loadDefault();
		uint32_t getBootCountFromFlash();
	};
}

#endif // __CONFIG_PARAM_H_INCLUDED__
