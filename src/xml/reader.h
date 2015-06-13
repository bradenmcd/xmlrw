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

# ifndef XML_READER_H
#   define XML_READER_H

#   include <iosfwd>
#   include <memory>
#   include <string>
#   include <stdexcept>

namespace xml
{
    class parse_error : public std::runtime_error {
        size_t line_;

    public:
        parse_error(size_t line, const std::string & msg);

        size_t line() const throw ();
    };


    class reader {
        struct impl;
        std::unique_ptr<impl> impl_;

    public:
        //
        // Conveniently, these values are consistent between libxml and
        // XmlLite.
        //
        enum node_type_id {
            none_id                   = 0,
            element_id                = 1,
            attribute_id              = 2,
            text_id                   = 3,
            cdata_id                  = 4,
            processing_instruction_id = 7,
            comment_id                = 8,
            document_type_id          = 10,
            whitespace_id             = 13,
            end_element_id            = 15,
            xml_declaration_id        = 17
        };

        explicit reader(const std::string & filename);
        explicit reader(std::istream & in);
        reader(const reader &) = delete;
        reader(reader &&) throw ();

        reader & operator=(const reader &) = delete;
        reader & operator=(reader &&) throw ();

        bool read();
        size_t line() const throw ();
        size_t col() const throw ();
        node_type_id node_type() const throw ();
        bool empty_element() const throw ();
        const std::string local_name() const;
        const std::string qualified_name() const;
        const std::string value() const;
        bool move_to_first_attribute();
        bool move_to_next_attribute();
    };
}

# endif // XML_READER_H
