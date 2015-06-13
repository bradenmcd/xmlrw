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

# ifndef XML_WRITER_H
#   define XML_WRITER_H

#   include <cstdint>
#   include <memory>
#   include <stdexcept>
#   include <string>

namespace xml
{
    class write_error : public std::runtime_error {
    public:
        explicit write_error(const std::string & msg);
    };

    class writer {
        struct impl;
        std::unique_ptr<impl> impl_;

    public:
        enum class standalone : std::uint8_t {
            omit = 0,
            yes = 1,
            no = 2
        };

        explicit writer(const std::string & filename);
        explicit writer(std::ostream & out);
        writer(const writer &) = delete;
        writer(writer &&) throw ();

        writer & operator=(const writer &) = delete;
        writer & operator=(writer &&) throw ();

        void start_document(standalone = standalone::omit);
        void end_document();
        void start_element(const std::string & prefix,
                           const std::string & local_name,
                           const std::string & namespace_uri);
        void end_element();
        void attribute(const std::string & prefix,
                       const std::string & local_name,
                       const std::string & namespace_uri,
                       const std::string & value);
        void comment(const std::string & text);
    };
}

# endif
