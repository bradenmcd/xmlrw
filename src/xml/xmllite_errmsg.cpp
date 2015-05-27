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

# include "xmllite_errmsg.h"
# include "reader.h"
# include "writer.h"

/**
 * @internal
 *
 * @file xml/xmllite_errmsg.h
 *
 * @brief Helper functions to convert COM `HRESULT`s to C++ exceptions.
 */

/**
 * @namespace xml::detail
 *
 * @internal
 *
 * @brief Implementation details.
 */

namespace {

    //
    // These come from:
    //   <https://msdn.microsoft.com/en-us/library/ms753129.aspx>
    //
    const char * reader_errmsg[] = {
        /* 00 */ "",
        /* 01 */ "unexpected end of input",
        /* 02 */ "unrecognized encoding",
        /* 03 */ "unable to switch the encoding",
        /* 04 */ "unrecognized input signature",
        /* 05 */ "",
        /* 06 */ "",
        /* 07 */ "",
        /* 08 */ "",
        /* 09 */ "",
        /* 0A */ "",
        /* 0B */ "",
        /* 0C */ "",
        /* 0D */ "",
        /* 0E */ "",
        /* 0F */ "",
        /* 20 */ "",
        /* 21 */ "whitespace expected",
        /* 22 */ "semicolon expected",
        /* 23 */ "'>' expected",
        /* 24 */ "quote expected",
        /* 25 */ "equal expected",
        /* 26 */ "well-formedness constraint: no '<' in attribute value",
        /* 27 */ "hexadecimal digit expected",
        /* 28 */ "'[' expected",
        /* 29 */ "'(' expected",
        /* 2A */ "illegal XML character",
        /* 2B */ "illegal name character",
        /* 2C */ "incorrect document syntax",
        /* 2D */ "incorrect CDATA section syntax",
        /* 2F */ "incorrect comment syntax",
        /* 30 */ "incorrect conditional section syntax",
        /* 31 */ "incorrect ATTLIST declaration syntax",
        /* 32 */ "incorrect DOCTYPE declaration syntax",
        /* 33 */ "incorrect ELEMENT declaration syntax",
        /* 34 */ "incorrect ENTITY declaration syntax",
        /* 35 */ "incorrect NOTATION declaration syntax",
        /* 36 */ "NDATA expected",
        /* 37 */ "PUBLIC expected",
        /* 38 */ "SYSTEM expected",
        /* 39 */ "name expected",
        /* 3A */ "one root element",
        /* 3B */ "well-formedness constraint: element type match",
        /* 3C */ "well-formedness constraint: unique attribute spec",
        /* 3D */ "text/xmldecl not at the beginning of input",
        /* 3E */ "leading \"xml\"",
        /* 3F */ "incorrect text declaration syntax",
        /* 40 */ "incorrect XML declaration syntax",
        /* 41 */ "incorrect encoding name syntax",
        /* 42 */ "incorrect public identifier syntax",
        /* 43 */ "well-formedness constraint: pes in internal subset",
        /* 44 */ "well-formedness constraint: pes between declarations",
        /* 45 */ "well-formedness constraint: no recursion",
        /* 46 */ "entity content not well formed",
        /* 47 */ "well-formedness constraint: undeclared entity",
        /* 48 */ "well-formedness constraint: parsed entity",
        /* 49 */ "well-formedness constraint: no external entity references",
        /* 4A */ "incorrect processing instruction syntax",
        /* 4B */ "incorrect system identifier syntax",
        /* 4C */ "'?' expected",
        /* 4D */ "no ']]>' in element content",
        /* 4E */ "not all chunks of value have been read",
        /* 4F */ "DTD was found but is prohibited",
        /* 50 */ "xml:space attribute with invalid value",
        /* 51 */ "",
        /* 52 */ "",
        /* 53 */ "",
        /* 54 */ "",
        /* 55 */ "",
        /* 56 */ "",
        /* 57 */ "",
        /* 58 */ "",
        /* 59 */ "",
        /* 5A */ "",
        /* 5B */ "",
        /* 5C */ "",
        /* 5D */ "",
        /* 5E */ "",
        /* 5F */ "",
        /* 60 */ "",
        /* 61 */ "illegal qualified name character",
        /* 62 */ "multiple colons in qualified name",
        /* 63 */ "colon in name",
        /* 64 */ "declared prefix",
        /* 65 */ "undeclared prefix",
        /* 66 */ "nondefault namespace with empty URI",
        /* 67 */ "\"xml\" prefix is reserved and must have the http://www.w3.org/XML/1998/namespace URI",
        /* 68 */ "\"xmlns\" prefix is reserved for use by XML",
        /* 69 */ "xml namespace URI (http://www.w3.org/XML/1998/namespace) must bee assigned only to prefix \"xml\"",
        /* 6A */ "xmlns namespace URI (http://www.w3.org/2000/xmlns/) is reserved and must not be used",
        /* 6B */ "",
        /* 6C */ "",
        /* 6D */ "",
        /* 6E */ "",
        /* 6F */ "",
        /* 70 */ "",
        /* 71 */ "",
        /* 72 */ "",
        /* 73 */ "",
        /* 74 */ "",
        /* 75 */ "",
        /* 76 */ "",
        /* 77 */ "",
        /* 78 */ "",
        /* 79 */ "",
        /* 7A */ "",
        /* 7B */ "",
        /* 7C */ "",
        /* 7D */ "",
        /* 7E */ "",
        /* 7F */ "",
        /* 80 */ "",
        /* 81 */ "element depth exceeds limit in XmlReaderProperty_MaxElementDepth",
        /* 82 */ "entity expansion exceeds limit in XmlReaderProperty_MaxEntityExpansion"
    };

    const char * writer_errmsg[] = {
        /* 00 */ "",
        /* 01 */ "specified string is not whitespace",
        /* 02 */ "namespace prefix is already declared with a different namespace",
        /* 03 */ "it is not allowed to declare a namespace prefix with empty URI",
        /* 04 */ "duplicate attribute",
        /* 05 */ "can not redefine the xmlns prefix",
        /* 06 */ "xml prefix must have the http://www.w3.org/XML/1998/namespace URI",
        /* 07 */ "xml namespace URI (http://www.w3.org/XML/1998/namespace) must be assigned only to prefix \"xml\"",
        /* 08 */ "xmlns namespace URI (http://www.w3.org/2000/xmlns/) is reserved and must not be used",
        /* 09 */ "namespace is not declared",
        /* 0A */ "invalid value of xml:space attribute (allowed values are \"default\" and \"preserve\")",
        /* 0B */ "performing the requested action would result in invalid XML document",
        /* 0C */ "input contains invalid or incomplete surrogate pair"
    };

    const char * misc_errmsg[] = {
        "character in character entity is not a decimal digit as was expected",
        "character in character entity is not a hexadecimal digit as was expected",
        "character entity has invalid Unicode value"
    };

    template <typename T, size_t Size>
    size_t size(T (&)[Size])
    {
        return Size;
    }

    const HRESULT xmllite_misc_hresult_base = 0xC00CE01D;

    const HRESULT xmllite_reader_hresult_base = 0xC00CEE00;

    const HRESULT xmllite_writer_hresult_base = 0xC00CEF00;

    size_t misc_errmsg_index(HRESULT hr) throw ()
    {
        return hr - xmllite_misc_hresult_base;
    }

    size_t reader_errmsg_index(HRESULT hr) throw ()
    {
        return hr - xmllite_reader_hresult_base;
    }

    size_t writer_errmsg_index(HRESULT hr) throw ()
    {
        return hr - xmllite_writer_hresult_base;
    }

    bool is_xmllite_misc_error(HRESULT hr) throw ()
    {
        return ((hr >> 8) == (xmllite_misc_hresult_base >> 8))
            && (misc_errmsg_index(hr) < size(misc_errmsg));
    }
}

/**
 * @internal
 *
 * @brief Check whether an `HRESULT` is an XmlLite reader error.
 *
 * @param[in] hr    a COM `HRESULT`.
 *
 * @retval true     if `hr` is an XmlLite reader error.
 * @retval false    if `hr` is not an XmlLite reader error.
 */
bool xml::detail::is_xmllite_reader_error(HRESULT hr) throw ()
{
    return (((hr >> 8) == (xmllite_reader_hresult_base >> 8))
            && (reader_errmsg_index(hr) < size(reader_errmsg)))
        || is_xmllite_misc_error(hr);
}

/**
 * @internal
 *
 * @brief Check whether an `HRESULT` is an XmlLite writer error.
 *
 * @param[in] hr    a COM `HRESULT`.
 *
 * @retval true     if `hr` is an XmlLite writer error.
 * @retval false    if `hr` is not an XmlLite writer error.
 */
bool xml::detail::is_xmllite_writer_error(HRESULT hr) throw ()
{
    return (((hr >> 8) == (xmllite_writer_hresult_base >> 8))
            && (writer_errmsg_index(hr) < size(writer_errmsg)))
        || is_xmllite_misc_error(hr);
}

/**
 * @internal
 *
 * @brief Throw a `xml::parse_error`.
 *
 * @param[in] reader    an XmlLite reader.
 * @param[in] hr        a COM `HRESULT`.
 *
 * @exception xml::parse_error  includes a descriptive message based on the
 *                              value of `hr`.
 */
void xml::detail::throw_parse_error(IXmlReader & reader, HRESULT hr)
{
    UINT line_number = 0;
    reader.GetLineNumber(&line_number);
    throw parse_error{line_number, reader_errmsg[reader_errmsg_index(hr)]};
}

/**
 * @internal
 *
 * @brief Throw a `xml::write_error`.
 *
 * @param[in] hr    a COM `HRESULT`.
 *
 * @exception xml::write_error  includes a descriptive message based on the
 *                              value of `hr`.
 */
void xml::detail::throw_write_error(HRESULT hr)
{
    throw write_error{writer_errmsg[writer_errmsg_index(hr)]};
}
