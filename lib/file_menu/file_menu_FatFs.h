/*-----------------------------------------------------------/
/ file_menu_FatFs: File Menu sorting utility for FatFs v0.90
/------------------------------------------------------------/
/ Copyright (c) 2020, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/-----------------------------------------------------------*/

#pragma once

#include <stddef.h>

#include "ff.h"

#ifdef __cplusplus
extern "C" {
#endif

FRESULT file_menu_init(uint8_t* fs_type);
FRESULT file_menu_deinit();
FRESULT file_menu_open_dir(const TCHAR* path);
FRESULT file_menu_ch_dir(uint16_t order);
void file_menu_close_dir(void);
uint16_t file_menu_get_num(void);
uint16_t file_menu_get_dir_num(void);
int file_menu_match_ext(uint16_t order, const char* ext, size_t ext_size); // ext: "mp3", "wav" (ext does not include ".")
uint16_t file_menu_get_ext_num(const char* ext, size_t ext_size); // ext: "mp3", "wav" (ext does not include ".")
uint16_t file_menu_get_ext_num_from_max(const char* ext, size_t ext_size, uint16_t max_order); // ext: "mp3", "wav" (ext does not include ".")
void file_menu_full_sort(void);
void file_menu_sort_entry(uint16_t scope_start, uint16_t scope_end_1);
FRESULT file_menu_get_fname(uint16_t order, char* str, uint16_t size);
TCHAR* file_menu_get_fname_ptr(uint16_t order);
int file_menu_is_dir(uint16_t order);
void file_menu_idle(void);

#ifdef __cplusplus
}
#endif
