/*------------------------------------------------------/
/ utf_conv                                              /
/-------------------------------------------------------/
/ Copyright (c) 2020, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/------------------------------------------------------*/

#ifndef _UTF_CONV_H_
#define _UTF_CONV_H_

#include <string>

std::string utf16_to_utf8(std::u16string const& src);
//std::string shiftjis_to_utf8(std::string const& src);

#endif // _UTF_CONV_H_
