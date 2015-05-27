// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 78 -*-

//
// The MIT License (MIT)
//
// Copyright (c) 2015 Braden McDaniel
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

# include "stringconvert.h"
# include <codecvt>
# include <locale>

/**
 * @internal
 *
 * @file xml/stringconvert.h
 *
 * @brief String encoding conversion functions.
 */

/**
 * @internal
 *
 * @brief Convert a UTF-8 string to a UTF-16 string.
 *
 * Ordinarily it would make more sense to us a `std::u16string` to hold a UTF-16
 * string; however, the Windows APIs are all `wchar_t`-based (and this function
 * is only used with the XmlLite backend).
 *
 * @param[in] str   a UTF-8-encoded string.
 *
 * @return a UTF-16-encoded string.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
std::wstring xml::detail::utf8_to_utf16(const std::string & str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conv;
    return conv.from_bytes(str);
}

/**
 * @internal
 *
 * @brief Convert a UTF-16 string to a UTF-8 string.
 *
 * Given how this function is used when interfacing with the Windows API,
 * accepting a `wchar_t` range is more pragmatic than using a string object.
 *
 * @param[in] first a pointer to the first element of a UTF-16-encoded string.
 * @param[in] last  a pointer to one past the last element of a UTF-16-encoded
 *                  string.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
std::string xml::detail::utf16_to_utf8(const wchar_t * first,
                                       const wchar_t * last)
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conv;
    return conv.to_bytes(first, last);
}
