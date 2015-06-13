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

# ifndef XML_STRINGCONVERT_H
#   define XML_STRINGCONVERT_H

#   include <string>

namespace xml {
    namespace detail {

        //
        // The lack of symmetry in this API is just pragmatic; as is the use of
        // wchar_t rather than char16_t.  These functions only get called on
        // Windows; and a char16_t* cannot be automaticaly converted to a
        // wchar_t*.
        //
        std::wstring utf8_to_utf16(const std::string & str);
        std::string utf16_to_utf8(const wchar_t * first, const wchar_t * last);
    }
}

# endif // ifndef XML_STRINGCONVERT_H
