/*------------------------------------------------------/
/ utf_conv                                              /
/-------------------------------------------------------/
/ Copyright (c) 2020, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#include "utf_conv.h"

#include <cassert>
#include <codecvt>
#include <locale>

extern "C"{
    int __exidx_start(){ return -1;}
    int __exidx_end(){ return -1; }
}

std::string utf16_to_utf8(std::u16string const& src)
{
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    return converter.to_bytes(src);
}

/*
#include <cstdlib>
std::string shiftjis_to_utf8(std::string const& src)
{
    // Shift_JIS to UTF-16
    wchar_t wstr[256];
    setlocale(LC_ALL, "JPN");
    mbstowcs(wstr, src.c_str(), 256);
    std::u16string u16str(&wstr[0], &wstr[255]);
    // UTF-16 to UTF-8
    return utf16_to_utf8(u16str);
}
*/