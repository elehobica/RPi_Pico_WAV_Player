/*-----------------------------------------------------------/
/ file_menu_FatFs: File Menu sorting utility for FatFs v0.90
/------------------------------------------------------------/
/ Copyright (c) 2020, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/-----------------------------------------------------------*/

#include "file_menu_FatFs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tf_card.h"

//#define DEBUG_FILE_MENU
//#define DEBUG_FILE_MENU_LVL2

#define TGT_DIRS    (1<<0)
#define TGT_FILES   (1<<1)
#define FFL_SZ 8

static FATFS fs;
static DIR dir;
static FILINFO fno, fno_temp;
static int target = TGT_DIRS | TGT_FILES; // TGT_DIRS, TGT_FILES
static int16_t f_stat_cnt;
static uint16_t max_entry_cnt;
static uint16_t* entry_list;
static uint32_t* sorted_flg;
static char (*fast_fname_list)[FFL_SZ];
static uint32_t* is_file_flg; // 0: Dir, 1: File
static uint16_t last_order; // order number memo for last file_menu_get_fname() request

//==============================
// idx Internal Funcions
//   provided by readdir 'index'
//==============================

static void idx_entry_swap(uint32_t entry_number1, uint32_t entry_number2)
{
    uint16_t tmp_entry;
    tmp_entry = entry_list[entry_number1];
    entry_list[entry_number1] = entry_list[entry_number2];
    entry_list[entry_number2] = tmp_entry;
}

static int32_t my_strcmp(char* str1 , char* str2)
{
    int32_t result = 0;

    // Compare until strings do not match
    while (result == 0) {
        result = *str1 - *str2;
        if (*str1 == '\0' || *str2 == '\0') break;  //Comparing also ends when reading to the end of the string
        str1++;
        str2++;
    }
    return result;
}

static int32_t my_strncmp(char* str1 , char* str2, int size)
{
    int32_t result = 0;
    int32_t len = 0;

    // Compare until strings do not match
    while (result == 0) {
        result = *str1 - *str2;
        len++;
        if (*str1 == '\0' || *str2 == '\0' || len >= size) break;   //Comparing also ends when reading to the end of the string
        str1++;
        str2++;
    }
    return result;
}

static FRESULT idx_f_stat(uint16_t idx, FILINFO* fno)
{
    FRESULT res = FR_OK;
    int error_count = 0;
    if (idx == 0) {
        strncpy(fno->fname, "..", FF_LFN_BUF);
        fno->fattrib = AM_DIR;
        return res;
    }
    if (f_stat_cnt == 1 || f_stat_cnt > idx) {
        // Rewind directory index
        f_readdir(&dir, 0);
        f_stat_cnt = 1;
    }
    for (;;) {
        f_readdir(&dir, fno);
        if (fno->fname[0] == '\0') {
            printf("Invalid empty name\n\r");
            res = FR_INVALID_NAME;
            if (error_count++ > 10) {
                printf("ERROR: idx_f_stat invalid name\n\r");
                break;
            }
            f_readdir(&dir, 0);
            f_stat_cnt = 1;
            continue;
        }
        if (fno->fname[0] == '.') continue;
        if (fno->fattrib & AM_HID) continue;
        if (!(target & TGT_DIRS)) { // File Only
            if (fno->fattrib & AM_DIR) continue;
        } else if (!(target & TGT_FILES)) { // Dir Only
            if (!(fno->fattrib & AM_DIR)) continue;
        }
        if (f_stat_cnt++ >= idx) break;
    }
    return res;
}

static void set_sorted(uint16_t pos) /* pos is entry_list's position, not index number */
{
    *(sorted_flg + (pos/32)) |= 1<<(pos%32);
}

static int get_sorted(uint16_t pos) /* pos is entry_list's position, not index number */
{
    return ((*(sorted_flg + (pos/32)) & 1<<(pos%32)) != 0);
}

static int get_range_full_sorted(uint16_t start, uint16_t end_1)
{
    int res = 1;
    for (int i = start; i < end_1; i++) {
        res &= get_sorted(i);
    }
    return res;
}

static int get_range_full_unsorted(uint16_t start, uint16_t end_1)
{
    int res = 0;
    for (int i = start; i < end_1; i++) {
        res |= get_sorted(i);
    }
    return !res;
}

static void set_is_file(uint16_t idx)
{
    *(is_file_flg + (idx/32)) |= 1<<(idx%32);
}

static int get_is_file(uint16_t idx)
{
    return ((*(is_file_flg + (idx/32)) & 1<<(idx%32)) != 0);
}

static void idx_qsort_entry_list_by_range(uint16_t r_start, uint16_t r_end_1, uint16_t start, uint16_t end_1)
{
    int result;
    int start_next;
    int end_1_next;
    char* fno_ptr;
    char* fno_temp_ptr;
    if (r_start < start) r_start = start;
    if (r_end_1 > end_1) r_end_1 = end_1;
    if (get_range_full_sorted(r_start, r_end_1)) return;
    if (get_range_full_sorted(start, end_1)) return;
    if (!get_range_full_unsorted(start, end_1)) { // Sorting including previous sorted items broke correct result
        start_next = start;
        while (get_sorted(start_next)) {
            start_next++;
        }
        end_1_next = start_next+1;
        while (!get_sorted(end_1_next) && end_1_next < end_1) {
            end_1_next++;
        }
        #ifdef DEBUG_FILE_MENU
        printf("partial %d %d %d\n\r", start_next, end_1_next, end_1);
        #endif // #ifdef DEBUG_FILE_MENU
        idx_qsort_entry_list_by_range(r_start, r_end_1, start_next, end_1_next);
        if (end_1_next < end_1) {
            idx_qsort_entry_list_by_range(r_start, r_end_1, end_1_next, end_1);
        }
        return;
    }
    #ifdef DEBUG_FILE_MENU_LVL2
    printf("r_start %d r_end_1 %d start %d end_1 %d\n\r", r_start, r_end_1, start, end_1);
    printf("\n\r");
    for (int k = start; k < end_1; k++) {
        idx_f_stat(entry_list[k], &fno);
        printf("before[%d] %d %s\n\r", k, entry_list[k], fno.fname);
    }
    #endif // #ifdef DEBUG_FILE_MENU_LVL2
    if (end_1 - start <= 1) {
        set_sorted(start);
    } else if (end_1 - start <= 2) {
        // try fast_fname_list compare
        result = get_is_file(entry_list[start]) - get_is_file(entry_list[start+1]);
        if (result == 0) {
            result = my_strncmp(fast_fname_list[entry_list[start]], fast_fname_list[entry_list[start+1]], FFL_SZ);
        }
        //printf("fast_fname_list %s %s %d, %d\n\r", fast_fname_list[entry_list[0]], fast_fname_list[entry_list[1]], entry_list[0], entry_list[1]);
        if (result > 0) {
            idx_entry_swap(start, start+1);
        } else if (result < 0) {
            // do nothing
        } else {
            // full name compare
            idx_f_stat(entry_list[start], &fno);
            idx_f_stat(entry_list[start+1], &fno_temp);
            fno_ptr = (strncmp(fno.fname, "The ", 4) == 0) ? &fno.fname[4] : fno.fname;
            fno_temp_ptr = (strncmp(fno_temp.fname, "The ", 4) == 0) ? &fno_temp.fname[4] : fno_temp.fname;
            result = my_strcmp(fno_ptr, fno_temp_ptr);
            if (result >= 0) {
                idx_entry_swap(start, start+1);
            }
        }
        set_sorted(start);
        set_sorted(start+1);
    } else {
        int top = start;
        int bottom = end_1 - 1;
        uint16_t key_idx = entry_list[start+(end_1-start)/2];
        idx_f_stat(key_idx, &fno_temp);
        #ifdef DEBUG_FILE_MENU_LVL2
        printf("key %s\n\r", fno_temp.fname);
        #endif // #ifdef DEBUG_FILE_MENU_LVL2
        while (1) {
            // try fast_fname_list compare
            result = get_is_file(entry_list[top]) - get_is_file(key_idx);
            if (result == 0) {
                result = my_strncmp(fast_fname_list[entry_list[top]], fast_fname_list[key_idx], FFL_SZ);
            }
            if (result < 0) {
                top++;
            } else if (result > 0) {
                idx_entry_swap(top, bottom);
                bottom--;               
            } else {
                // full name compare
                idx_f_stat(entry_list[top], &fno);
                fno_ptr = (strncmp(fno.fname, "The ", 4) == 0) ? &fno.fname[4] : fno.fname;
                fno_temp_ptr = (strncmp(fno_temp.fname, "The ", 4) == 0) ? &fno_temp.fname[4] : fno_temp.fname;
                result = my_strcmp(fno_ptr, fno_temp_ptr);
                if (result < 0) {
                    top++;
                } else {
                    idx_entry_swap(top, bottom);
                    bottom--;
                }
            }
            if (top > bottom) break;
        }
        #ifdef DEBUG_FILE_MENU_LVL2
        for (int k = 0; k < top; k++) {
            idx_f_stat(entry_list[k], &fno);
            printf("top[%d] %d %s\n\r", k, entry_list[k], fno.fname);
        }
        for (int k = top; k < max_entry_cnt; k++) {
            idx_f_stat(entry_list[k], &fno);
            printf("bottom[%d] %d %s\n\r", k, entry_list[k], fno.fname);
        }
        #endif // #ifdef DEBUG_FILE_MENU_LVL2
        if ((r_start < top && r_end_1 > start) && !get_range_full_sorted(start, top)) {
            if (top - start > 1) {
                idx_qsort_entry_list_by_range(r_start, r_end_1, start, top);
            } else {
                set_sorted(start);
            }
        }
        if ((r_start < end_1 && r_end_1 > top) && !get_range_full_sorted(top, end_1)) {
            if (end_1 - top > 1) {
                idx_qsort_entry_list_by_range(r_start, r_end_1, top, end_1);
            } else {
                set_sorted(top);
            }
        }
    }
}

static uint16_t idx_get_size(int target)
{
    int16_t cnt = 1;
    // Rewind directory index
    f_readdir(&dir, 0);
    // Directory search completed with null character
    for (;;) {
        f_readdir(&dir, &fno);
        if (fno.fname[0] == '\0') break;
        if (fno.fname[0] == '.') continue;
        if (fno.fattrib & AM_HID) continue;
        if (!(target & TGT_DIRS)) { // File Only
            if (fno.fattrib & AM_DIR) continue;
        } else if (!(target & TGT_FILES)) { // Dir Only
            if (!(fno.fattrib & AM_DIR)) continue;
        }
        cnt++;
    }
    // Returns the number of entries read
    return cnt;
}

static void idx_sort_new(void)
{
    int i, k;
    max_entry_cnt = idx_get_size(target);
    entry_list = (uint16_t*) malloc(sizeof(uint16_t) * max_entry_cnt);
    if (entry_list == NULL) printf("malloc entry_list failed\n\r");
    for (i = 0; i < max_entry_cnt; i++) entry_list[i] = i;
    sorted_flg = (uint32_t*) malloc(sizeof(uint32_t) * (max_entry_cnt+31)/32);
    if (sorted_flg == NULL) printf("malloc sorted_flg failed\n\r");
    memset(sorted_flg, 0, sizeof(uint32_t) * (max_entry_cnt+31)/32);
    is_file_flg = (uint32_t*) malloc(sizeof(uint32_t) * (max_entry_cnt+31)/32);
    if (is_file_flg == NULL) printf("malloc is_file_flg failed\n\r");
    memset(is_file_flg, 0, sizeof(uint32_t) * (max_entry_cnt+31)/32);
    fast_fname_list = (char (*)[FFL_SZ]) malloc(sizeof(char[FFL_SZ]) * max_entry_cnt);
    if (fast_fname_list == NULL) printf("malloc fast_fname_list failed\n\r");
    for (i = 0; i < max_entry_cnt; i++) {
        idx_f_stat(i, &fno);
        if (!(fno.fattrib & AM_DIR)) set_is_file(i);
        if (strncmp(fno.fname, "The ", 4) == 0) {
            for (k = 0; k < FFL_SZ; k++) {
                fast_fname_list[i][k] = fno.fname[k+4];
            }
        } else {
            for (k = 0; k < FFL_SZ; k++) {
                fast_fname_list[i][k] = fno.fname[k];
            }
        }
        #ifdef DEBUG_FILE_MENU_LVL2
        char temp_str[5] = "    ";
        strncpy(temp_str, fast_fname_list[i], 4);
        printf("fast_fname_list[%d] = %4s, is_file = %d\r\n", i, temp_str, get_is_file(i));
        #endif // #ifdef DEBUG_FILE_MENU_LVL2
    }
}

static void idx_sort_delete(void)
{
    free(entry_list);
    free(sorted_flg);
    free(fast_fname_list);
    free(is_file_flg);
}

//==============================
// File Menu Public Funcions
//   provided by sorted 'order'
//==============================
// Mount FAT
FRESULT file_menu_init(uint8_t* fs_type)
{
    FRESULT fr;
    pico_fatfs_spi_config_t config = {
        spi0,
        CLK_SLOW_DEFAULT,
        40 * MHZ,
        PIN_SPI0_MISO_DEFAULT,
        PIN_SPI0_CS_DEFAULT,
        PIN_SPI0_SCK_DEFAULT,
        PIN_SPI0_MOSI_DEFAULT,
        true  // use internal pullup
    };

    pico_fatfs_set_config(&config);
    fr = f_mount(&fs, "", 1); // fr: 0: mount successful, 1: mount failed
    if (fr == FR_OK) {
        *fs_type = fs.fs_type;
    }
    return fr;
}

// For implicit sort all entries
void file_menu_idle(void)
{
    static int up_down = 0;
    uint16_t r_start = 0;
    uint16_t r_end_1 = 0;
    if (get_range_full_sorted(0, max_entry_cnt)) return;
    for (;;) {
        if (up_down & 0x1) {
            r_start = last_order + 1;
            while (get_range_full_sorted(last_order, r_start) && r_start < max_entry_cnt) {
                r_start++;
            }
            r_start--;
            r_end_1 = r_start + 1;
            while (get_range_full_unsorted(r_start, r_end_1) && r_end_1 <= max_entry_cnt && r_end_1 - r_start <= 5) {
                r_end_1++;
            }
            r_end_1--;
            up_down++;
        } else {
            if (last_order > 0) {
                r_end_1 = last_order - 1;
                while (get_range_full_sorted(r_end_1, last_order+1) && r_end_1 != 0) {
                    r_end_1--;
                }
                r_end_1++;
                r_start = r_end_1 - 1;
                while (get_range_full_unsorted(r_start, r_end_1) && r_start != 0 && r_end_1 - r_start <= 5) {
                    r_start--;
                }               
            }
            up_down++;
        }
        break;
    }
    
    #ifdef DEBUG_FILE_MENU
    printf("implicit sort %d %d\n\r", r_start, r_end_1);
    #endif // #ifdef DEBUG_FILE_MENU
    idx_qsort_entry_list_by_range(r_start, r_end_1, 0, max_entry_cnt);
}

void file_menu_sort_entry(uint16_t scope_start, uint16_t scope_end_1)
{
    uint16_t wing;
    uint16_t wing_start, wing_end_1;
    if (scope_start >= scope_end_1) return;
    if (scope_start > max_entry_cnt - 1) scope_start = max_entry_cnt - 1;
    if (scope_end_1 > max_entry_cnt) scope_end_1 = max_entry_cnt;
    wing = (scope_end_1 - scope_start)*2;
    wing_start = (scope_start > wing) ? scope_start - wing : 0;
    wing = (scope_end_1 - scope_start)*4 - (scope_start - wing_start);
    wing_end_1 = (scope_end_1 + wing < max_entry_cnt) ? scope_end_1 + wing : max_entry_cnt;
    //printf("scope_start %d %d %d %d\n\r", scope_start, scope_end_1, wing_start, wing_end_1);
    if (!get_range_full_sorted(scope_start, scope_end_1)) {
        idx_qsort_entry_list_by_range(wing_start, wing_end_1, 0, max_entry_cnt);
    }
}

void file_menu_full_sort(void)
{
    file_menu_sort_entry(0, max_entry_cnt);
}

TCHAR* file_menu_get_fname_ptr(uint16_t order)
{
    FRESULT fr = FR_INVALID_PARAMETER;     /* FatFs return code */
    file_menu_sort_entry(order, order+5);
    if (order < max_entry_cnt) {
        fr = idx_f_stat(entry_list[order], &fno);
        last_order = order;

    }
    if (fr == FR_OK) {
        return fno.fname;
    } else {
        return "";
    }
}

FRESULT file_menu_get_fname(uint16_t order, char* str, uint16_t size)
{
    FRESULT fr = FR_INVALID_PARAMETER;     /* FatFs return code */
    file_menu_sort_entry(order, order+5);
    if (order < max_entry_cnt) {
        fr = idx_f_stat(entry_list[order], &fno);
        strncpy(str, fno.fname, size);
        last_order = order;
    }
    return fr;
}

int file_menu_is_dir(uint16_t order)
{
    if (order < max_entry_cnt) {
        return !get_is_file(entry_list[order]);
    } else {
        return -1;
    }
}

uint16_t file_menu_get_num(void)
{
    return max_entry_cnt;
}

uint16_t file_menu_get_dir_num(void)
{
    uint16_t count = 0;
    for (int i = 1; i < max_entry_cnt; i++) {
        if (file_menu_is_dir(i) > 0) { count++; }
    }
    return count;
}

int file_menu_match_ext(uint16_t order, const char* ext, size_t ext_size)
{
    char name[FF_MAX_LFN];
    file_menu_get_fname(order, name, sizeof(name));
    char* ext_pos = strrchr(name, '.');
    if (ext_pos) {
        if (strncmp(ext_pos+1, ext, ext_size) == 0) { return 1; }
    }
    return 0;
}

uint16_t file_menu_get_ext_num(const char* ext, size_t ext_size)
{
    return file_menu_get_ext_num_from_max(ext, ext_size, max_entry_cnt);
}

uint16_t file_menu_get_ext_num_from_max(const char* ext, size_t ext_size, uint16_t max_order)
{
    uint16_t count = 0;
    for (int i = 1; i < max_order; i++) {
        if (file_menu_match_ext(i, ext, ext_size)) { count++; }
        /*
        file_menu_get_fname(i, name, sizeof(name));
        char* ext_pos = strrchr(name, '.');
        if (ext_pos) {
            if (strncmp(ext_pos+1, ext, ext_size) == 0) { count++; }
        }
        */
    }
    return count;
}

FRESULT file_menu_open_dir(const TCHAR* path)
{
    FRESULT fr = FR_INVALID_PARAMETER;     /* FatFs return code */
    //fr = f_opendir(&dir, path);
    f_chdir(path);
    fr = f_opendir(&dir, ".");
    f_stat_cnt = 1;
    last_order = 0;
    if (fr == FR_OK) {
        idx_sort_new();
    }
    return fr;
}

FRESULT file_menu_ch_dir(uint16_t order)
{
    FRESULT fr = FR_INVALID_PARAMETER;     /* FatFs return code */
    if (order < max_entry_cnt) {
        fr = idx_f_stat(entry_list[order], &fno);
        f_closedir(&dir);
        //printf("chdir %s\n\r", fno.fname);
        f_chdir(fno.fname);
        fr = f_opendir(&dir, ".");
        idx_sort_delete();
    }
    f_stat_cnt = 1;
    last_order = 0;
    //fr = f_opendir(&dir, path);
    //f_chdir(path);
    //fr = f_opendir(&dir, ".");
    if (fr == FR_OK) {
        idx_sort_new();
    }
    return fr;
}

void file_menu_close_dir(void)
{
    /*
    for (int i = 0; i < max_entry_cnt; i++) {
        char temp_str[5] = "    ";
        strncpy(temp_str, fast_fname_list[i], 4);
        printf("fast_fname_list[%d] = %4s\r\n", i, temp_str);
    }
    */
    idx_sort_delete();
    f_closedir(&dir);
}
