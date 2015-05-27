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

# include "reader.h"
# include <istream>
# ifdef HAVE_XMLLITE
#   include "xmllite_errmsg.h"
#   include "finally.h"
#   include "stringconvert.h"
#   include <shlwapi.h>
# else
#   include <libxml/xmlreader.h>
# endif

/**
 * @file xml/reader.h
 *
 * @brief XML reader.
 */

/**
 * @internal
 *
 * @file xml/finally.h
 *
 * @brief Generalized wrapper for stack-based cleanup of dynamically allocated
 *        resources.
 */

/**
 * @mainpage
 *
 * @section intro Introduction
 *
 * <a href="http://www.xmlsoft.org">libxml2</a>'s <a href="http://www.xmlsoft.org/html/libxml-xmlreader.html">xmlTextReader</a>
 * and Microsoft's <a href="https://msdn.microsoft.com/en-us/library/ms752872.aspx">XmlLite</a>
 * both offer interfaces based on the <a href="https://msdn.microsoft.com/en-us/library/system.xml.xmltextreader.aspx">C# XmlTextReader</a>.  xmlrw provides a
 * fa&ccedil;ade that takes advantage of the commonalities between these two
 * interfaces and supplies a modern C++ interface.
 *
 * The C interface provided by xmlTextReader and the COM interface provided by
 * XmlLite can each be verbose and somewhat error-prone, particularly with
 * regard to error handling and resource management.  xmlrw's C++ interface
 * streamlines a good deal of this by relying on call stack-based resource
 * management and C++ exceptions.
 *
 * @section caveats Caveats
 *
 * - xmlrw currently reads and writes only UTF-8.
 * - libxml2 uses UTF-8 strings in its API, while XmlLite uses UTF-16 strings
 *   (like the rest of the Windows API).  xmlrw uses UTF-8 strings, which means
 *   that the strings must be converted when using the XmlLite backend.
 */

/**
 * @namespace xml
 *
 * @brief XML reader and writer.
 */

/**
 * @class xml::parse_error
 *
 * @brief Exception thrown when @c xml::reader encounters a parse error.
 */

/**
 * @brief Construct.
 *
 * @param[in] line the line number where the error occurred
 * @param[in] msg  a message describing the parse error
 */
xml::parse_error::parse_error(size_t line, const std::string & msg):
    std::runtime_error{msg},
    line_{line}
{}

/**
 * @brief The line number where the error occurred.
 *
 * @return the line number where the error occurred
 */
size_t xml::parse_error::line() const throw ()
{
    return this->line_;
}

/**
 * @class xml::reader
 *
 * @brief A wrapper that abstracts commonality between libxml's XmlTextReader
 *        interface and Microsoft's XmlLite.
 *
 * There is a lot in common here between the two underlying APIs; and that's
 * no coincidence, apparently: both APIs are based on the C# XmlReader API.
 */

/**
 * @internal
 *
 * @brief Using the pimpl idiom here avoids exposing libxml2/XmlLite types in
 *        the header.
 */
struct xml::reader::impl {
# ifdef HAVE_XMLLITE
    IStream * input;
    IXmlReader * reader;
# else
    parse_error error;
    xmlTextReaderPtr reader;
# endif

    explicit impl(const std::string & filename);
    explicit impl(std::istream & in);
    impl(const impl &) = delete;
    ~impl() throw ();

    impl & operator=(const impl &) = delete;
};

/**
 * @var xml::parse_error xml::reader::impl::error
 *
 * @internal
 *
 * @brief In the event of an error, the value of this exception is set.
 *
 * This value is set by the
 * <a href="http://www.xmlsoft.org/html/libxml-xmlreader.html#xmlTextReaderErrorFunc">`xmlTextReaderErrorFunc`</a>.
 * When <a href="http://www.xmlsoft.org/html/libxml-xmlreader.html#xmlTextReaderRead">`xmlTextReaderRead`</a>
 * indicates a parsing error, the value is thrown.
 *
 * @sa xml::reader::read
 */

/**
 * @var xmlTextReaderPtr xml::reader::impl::reader
 *
 * @internal
 *
 * @brief The <a href="http://www.xmlsoft.org/html/libxml-xmlreader.html#xmlTextReader">`xmlTextReader`</a>.
 */

# ifndef HAVE_XMLLITE
extern "C" {
    void xml_reader_errorFunc(void * arg, const char * msg,
                              xmlParserSeverities severity,
                              xmlTextReaderLocatorPtr locator);
}
# endif

/**
 * @internal
 *
 * @brief Construct using a UTF-8 file name.
 *
 * @param[in] filename a UTF-8-encoded file name.
 *
 * @exception std::runtime_error   if:
 *                                  * @p filename cannot be opened; or
 *                                  * XmlLite/libxml2 setup fails
 * @exception std::bad_alloc       if memory allocation fails
 */
xml::reader::impl::impl(const std::string & filename):
# ifdef HAVE_XMLLITE
    input{0},
# else
    error{0, ""},
# endif
    reader{0}
{
# ifdef HAVE_XMLLITE
    HRESULT hr;
    bool succeeded = false;

    hr = SHCreateStreamOnFile(
            reinterpret_cast<LPCWSTR>(detail::utf8_to_utf16(filename).c_str()),
            STGM_READ,
            &this->input);
    detail::finally f1([&]{
        if (!succeeded && input != nullptr) { input->Release(); }
    });
    if (FAILED(hr)) {
        if (hr == E_OUTOFMEMORY) {
            throw std::bad_alloc{};
        }
        throw std::runtime_error{"failed to open file \"" + filename
                                 + '\"'};
    }

    hr = CreateXmlReader(__uuidof(IXmlReader),
                         reinterpret_cast<void **>(&this->reader),
                         0);
    detail::finally f2([&]{
        if (!succeeded && this->reader != nullptr) { this->reader->Release(); }
    });
    if (FAILED(hr)) {
        if (hr == E_OUTOFMEMORY) {
            throw std::bad_alloc{};
        }
        throw std::runtime_error{"failed to create XML reader"};
    }

    hr = this->reader->SetInput(this->input);
    if (FAILED(hr)) {
        throw std::runtime_error{"failed to set input for XML reader"};
    }

    succeeded = true;
# else
    static const char * const encoding = 0;
    static const int options = 0;
    this->reader = xmlReaderForFile(filename.c_str(), encoding, options);
    if (!this->reader) {
        throw std::runtime_error{"failed to create XML reader"};
    }
    xmlTextReaderSetErrorHandler(this->reader, xml_reader_errorFunc, this);
# endif
}

# ifdef HAVE_XMLLITE
namespace
{
    class com_istream : public ::IStream {
        std::istream & in_;
        LONG count_;

    public:
        explicit com_istream(std::istream & in);
        com_istream(const com_istream &) = delete;

        com_istream & operator=(const com_istream &) = delete;

        //
        // IUnknown implementation
        //
        virtual HRESULT __stdcall QueryInterface(const IID & iid, void ** ppv);
        virtual ULONG __stdcall AddRef();
        virtual ULONG __stdcall Release();

        //
        // ISequentialStream implementation
        //
        virtual HRESULT __stdcall Read(void * pv, ULONG cb, ULONG * pcbRead);
        virtual HRESULT __stdcall Write(const void * pv, ULONG cb, ULONG * pcbWritten);

        //
        // IStream implementation
        //
        virtual HRESULT __stdcall Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin,
                                       ULARGE_INTEGER * plibNewPosition);
        virtual HRESULT __stdcall SetSize(ULARGE_INTEGER libNewSize);
        virtual HRESULT __stdcall CopyTo(IStream * pstm, ULARGE_INTEGER cb,
                                         ULARGE_INTEGER * pcbRead,
                                         ULARGE_INTEGER * pcbWritten);
        virtual HRESULT __stdcall Commit(DWORD grfCommitFlags);
        virtual HRESULT __stdcall Revert();
        virtual HRESULT __stdcall LockRegion(ULARGE_INTEGER libOffset,
                                             ULARGE_INTEGER cb,
                                             DWORD dwLockType);
        virtual HRESULT __stdcall UnlockRegion(ULARGE_INTEGER libOffset,
                                               ULARGE_INTEGER cb,
                                               DWORD dwLockType);
        virtual HRESULT __stdcall Stat(STATSTG * pstatstg, DWORD grfStatFlag);
        virtual HRESULT __stdcall Clone(IStream ** ppstm);
    };

}
# else
extern "C" {
    int xml_reader_inputReadCallback(void * context, char * buffer, int len);
    int xml_reader_inputCloseCallback(void * context);
}
# endif

/**
 * @internal
 *
 * @brief Construct using an input stream.
 *
 * @param[in,out] in   an input stream.
 *
 * @exception std::runtime_error   if XmlLite/libxml2 setup fails
 * @exception std::bad_alloc       if memory allocation fails
 */
xml::reader::impl::impl(std::istream & in):
# ifdef HAVE_XMLLITE
    input{new com_istream{in}},
# else
    error{0, ""},
# endif
    reader{0}
{
# ifdef HAVE_XMLLITE
    bool succeeded = false;
    detail::finally f1([&]{
        if (!succeeded && input != nullptr) { input->Release(); }
    });

    HRESULT hr;

    static IMalloc * const malloc = 0;
    IXmlReaderInput * xml_input = 0;
    static const WCHAR * const base_uri = 0;
    hr = CreateXmlReaderInputWithEncodingName(this->input,
                                              malloc,
                                              L"utf-8",
                                              FALSE, // Require UTF-8.
                                              base_uri,
                                              &xml_input);
    if (FAILED(hr)) {
        throw std::runtime_error{"failed to create XML input"};
    }

    hr = CreateXmlReader(__uuidof(IXmlReader),
                         reinterpret_cast<void **>(&this->reader),
                         malloc);
    detail::finally f2([&]{
        if (!succeeded && this->reader != nullptr) { this->reader->Release(); }
    });
    if (FAILED(hr)) {
        throw std::runtime_error{"failed to create XML reader"};
    }

    hr = this->reader->SetInput(xml_input);
    if (FAILED(hr)) {
        throw std::runtime_error{"failed to set input for XML reader"};
    }

    succeeded = true;
# else
    static const char * const base_uri = 0;
    static const char * const encoding = 0;
    static const int options = 0;
    this->reader = xmlReaderForIO(xml_reader_inputReadCallback,
                                  xml_reader_inputCloseCallback,
                                  &in,
                                  base_uri,
                                  encoding,
                                  options);
    if (!this->reader) {
        throw std::runtime_error{"failed to create XML reader"};
    }
    xmlTextReaderSetErrorHandler(this->reader,
                                 xml_reader_errorFunc,
                                 &this->error);
# endif
}

/**
 * @fn xml::reader::impl::impl(const impl &)
 *
 * @internal
 *
 * @brief Not copyable.
 */

/**
 * @internal
 *
 * @brief Destroy.
 */
xml::reader::impl::~impl() throw ()
{
# ifdef HAVE_XMLLITE
    this->reader->Release();
    this->input->Release();
# else
    xmlFreeTextReader(this->reader);
# endif
}

/**
 * @fn xml::reader::impl & xml::reader::impl::operator=(const impl &)
 *
 * @internal
 *
 * @brief Not copyable.
 */

/**
 * @enum xml::reader::node_type_id
 *
 * @brief The node type identifier.
 *
 * Due to the common heritage of XmlTextReader and XmlLite, the underlying
 * APIs report consistent values that can be passed through directly to
 * the @c xml::reader interface.
 *
 * @sa http://xmlsoft.org/html/libxml-xmlreader.html#xmlReaderTypes
 * @sa http://msdn.microsoft.com/en-us/library/ms753141.aspx
 */

/**
 * @var xml::reader::node_type_id xml::reader::none_id
 *
 * @brief The &ldquo;null&rdquo; type identifier.
 */

/**
 * @var xml::reader::node_type_id xml::reader::element_id
 *
 * @brief Element identifier.
 */

/**
 * @var xml::reader::node_type_id xml::reader::attribute_id
 *
 * @brief Attribute identifier.
 */

/**
 * @var xml::reader::node_type_id xml::reader::text_id
 *
 * @brief Text identifier.
 */

/**
 * @var xml::reader::node_type_id xml::reader::cdata_id
 *
 * @brief CDATA identifier.
 */

/**
 * @var xml::reader::node_type_id xml::reader::processing_instruction_id
 *
 * @brief Processing instruction identifier.
 */

/**
 * @var xml::reader::node_type_id xml::reader::comment_id
 *
 * @brief Comment identifier.
 */

/**
 * @var xml::reader::node_type_id xml::reader::document_type_id
 *
 * @brief Document type identifier.
 */

/**
 * @var xml::reader::node_type_id xml::reader::whitespace_id
 *
 * @brief Whitespace identifier.
 */

/**
 * @var xml::reader::node_type_id xml::reader::end_element_id
 *
 * @brief Element end tag identifier.
 *
 * Unlike SAX, where you get an &ldquo;end of element&rdquo; callback at
 * the logical element boundary, @c #node_type only returns this value
 * when the current node is a literal end tag.
 */

/**
 * @var xml::reader::node_type_id xml::reader::xml_declaration_id
 *
 * @brief XML declaration identifier.
 */

/**
 * @brief Construct from a file name.
 *
 * @param[in] filename  the full path to a file.
 *
 * @exception std::runtime_error    if opening @p filename or creating the
 *                                  underlying XML reader fails.
 * @exception std::bad_alloc        if memory allocation fails.
 */
xml::reader::reader(const std::string & filename):
    impl_{new impl{filename}}
{}

/**
 * @brief Construct from an input stream.
 *
 * @param[in,out] in   an input stream.
 *
 * @exception std::runtime_error    if creating the underlying XML reader
 *                                  fails.
 * @exception std::bad_alloc        if memory allocation fails.
 */
xml::reader::reader(std::istream & in):
    impl_{new impl{in}}
{}

/**
 * @fn xml::reader::reader(const reader &)
 *
 * @brief Not copyable.
 */

/**
 * @brief Move constuct.
 */
xml::reader::reader(reader && r) throw ():
    impl_{std::move(r.impl_)}
{}

/**
 * @fn xml::reader & xml::reader::operator=(const reader &)
 *
 * @brief Not copyable.
 */

/**
 * @brief Move assign.
 */
xml::reader & xml::reader::operator=(reader && r) throw ()
{
    this->impl_ = std::move(r.impl_);
    return *this;
}

/**
 * @brief Advance to the next node in the stream.
 *
 * @retval true if the node was read successfully
 * @retval false if there are no more nodes to read
 *
 * @exception xml::parse_error  if there is an error in the input.
 */
bool xml::reader::read()
{
# ifdef HAVE_XMLLITE
    HRESULT hr = this->impl_->reader->Read(0);
    if (FAILED(hr)) {
        if (detail::is_xmllite_reader_error(hr)) {
            detail::throw_parse_error(*this->impl_->reader, hr);
        }
    }
    return hr == S_OK;
# else
    const int result = xmlTextReaderRead(this->impl_->reader);
    if (result < 0) { throw this->impl_->error; }
    return result;
# endif
}

/**
 * @brief The line number of the current parsing position.
 *
 * @return the line number of the current parsing position.
 */
size_t xml::reader::line() const throw ()
{
# ifdef HAVE_XMLLITE
    UINT line_number = 0;
    this->impl_->reader->GetLineNumber(&line_number);
    return line_number;
# else
    return xmlTextReaderGetParserLineNumber(this->impl_->reader);
# endif
}

/**
 * @brief The column number of the current parsing position.
 *
 * @return the column number of the current parsing position.
 */
size_t xml::reader::col() const throw ()
{
# ifdef HAVE_XMLLITE
    UINT column_number = 0;
    this->impl_->reader->GetLinePosition(&column_number);
    return column_number;
# else
    return xmlTextReaderGetParserColumnNumber(this->impl_->reader);
# endif
}

/**
 * @brief The type of the current node.
 *
 * @return the type of the current node.
 */
xml::reader::node_type_id xml::reader::node_type() const throw ()
{
# ifdef HAVE_XMLLITE
    XmlNodeType type;
    this->impl_->reader->GetNodeType(&type);
    return static_cast<node_type_id>(type);
# else
    return static_cast<node_type_id>(
        xmlTextReaderNodeType(this->impl_->reader));
# endif
}

/**
 * @brief Whether the current element is empty.
 *
 * @retval true if the current element is empty.
 * @retval false if the current element is not empty.
 */
bool xml::reader::empty_element() const throw ()
{
# ifdef HAVE_XMLLITE
    return this->impl_->reader->IsEmptyElement() == TRUE;
# else
    //
    // xmlTextReaderIsEmptyElement may return -1 if the function is not
    // applicable to the current context.  XmlLite just returns 'false' in
    // that case.
    //
    const int result = xmlTextReaderIsEmptyElement(this->impl_->reader);
    return result == 1;
# endif
}

/**
 * @brief The local (i.e., unqualified) name of the node.
 *
 * @return the local name of the node.
 *
 * @exception std::runtime_error    if there is an error getting the name.
 * @exception std::bad_alloc        if memory allocation fails.
 */
const std::string xml::reader::local_name() const
{
# ifdef HAVE_XMLLITE
    const WCHAR * name;
    UINT length;
    HRESULT hr = this->impl_->reader->GetLocalName(&name, &length);
    if (FAILED(hr)) {
        throw std::runtime_error{"failed to get element name"};
    }
    return detail::utf16_to_utf8(name, name + length);
# else
    const xmlChar * name = xmlTextReaderConstLocalName(this->impl_->reader);
    if (name == nullptr) {
        throw std::runtime_error{"failed to get element name"};
    }
    return std::string{name, name + xmlStrlen(name)};
# endif
}

/**
 * @brief The qualified name of the node.
 *
 * @return the qualified name of the node.
 *
 * @exception std::bad_alloc    if memory allocation fails.
 */
const std::string xml::reader::qualified_name() const
{
# ifdef HAVE_XMLLITE
    const WCHAR * name;
    UINT length;
    HRESULT hr = this->impl_->reader->GetQualifiedName(&name, &length);
    if (FAILED(hr)) {
        throw std::runtime_error{"failed to get element name"};
    }
    return detail::utf16_to_utf8(name, name + length);
# else
    const xmlChar * name = xmlTextReaderConstName(this->impl_->reader);
    if (name == nullptr) {
        throw std::runtime_error{"failed to get element name"};
    }
    return std::string{name, name + xmlStrlen(name)};
# endif
}

/**
 * @brief The text value of the node, if any.
 *
 * @return the text value of the node.
 *
 * @exception std::runtime_error    if there is an error getting the value.
 * @exception std::bad_alloc        if memory allocation fails.
 */
const std::string xml::reader::value() const
{
# ifdef HAVE_XMLLITE
    const WCHAR * val = 0;
    UINT length = 0;
    HRESULT hr = this->impl_->reader->GetValue(&val, &length);
    if (FAILED(hr)) {
        throw std::runtime_error{"failed to get a value"};
    }
    return detail::utf16_to_utf8(val, val + length);
# else
    const xmlChar * val = xmlTextReaderConstValue(this->impl_->reader);
    if (val == nullptr) {
        throw std::runtime_error{"failed to get a value"};
    }
    return std::string{val, val + xmlStrlen(val)};
# endif
}

/**
 * @brief Move to the first attribute associated with the current node.
 *
 * @retval true on success
 * @retval false if the current node has no attributes
 *
 * @exception xml::parse_error  if there is an error in the input.
 */
bool xml::reader::move_to_first_attribute()
{
# ifdef HAVE_XMLLITE
    HRESULT hr = this->impl_->reader->MoveToFirstAttribute();
    if (FAILED(hr)) {
        if (detail::is_xmllite_reader_error(hr)) {
            detail::throw_parse_error(*this->impl_->reader, hr);
        }
    }
    return hr == S_OK;
# else
    const int result = xmlTextReaderMoveToFirstAttribute(this->impl_->reader);
    if (result < 0) { throw this->impl_->error; }
    return result;
# endif
}

/**
 * @brief Move to the next attribute associated with the current node.
 *
 * @retval true on success
 * @retval false if the current node has no more attributes
 *
 * @exception xml::parse_error  if there is an error in the input.
 */
bool xml::reader::move_to_next_attribute()
{
# ifdef HAVE_XMLLITE
    HRESULT hr = this->impl_->reader->MoveToNextAttribute();
    if (FAILED(hr)) {
        if (detail::is_xmllite_reader_error(hr)) {
            detail::throw_parse_error(*this->impl_->reader, hr);
        }
    }
    return hr == S_OK;
# else
    const int result = xmlTextReaderMoveToNextAttribute(this->impl_->reader);
    if (result < 0) { throw this->impl_->error; }
    return result;
# endif
}

# ifdef HAVE_XMLLITE
namespace
{
    com_istream::com_istream(std::istream & in):
        in_{in},
        count_{1}
    {}

    HRESULT com_istream::QueryInterface(const IID & iid,
                                        void ** ppv)
    {
        if (!ppv) {
            return E_INVALIDARG;
        }
        if (iid == __uuidof(IUnknown)
            || iid == __uuidof(IStream)
            || iid == __uuidof(ISequentialStream)) {
            *ppv = static_cast<IStream *>(this);
            this->AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    ULONG com_istream::AddRef()
    {
        return ::InterlockedIncrement(&this->count_);
    }

    ULONG com_istream::Release()
    {
        if (::InterlockedDecrement(&this->count_) == 0) {
            delete this;
            return 0;
        }
        return this->count_;
    }

    HRESULT com_istream::Read(void * const pv,
                              const ULONG cb,
                              ULONG * const pcbRead)
    {
        if (!pv || !pcbRead) {
            return STG_E_INVALIDPOINTER;
        }

        try {
            this->in_.read(static_cast<char *>(pv), cb);
            *pcbRead = static_cast<ULONG>(this->in_.gcount());
        } catch (const std::ios_base::failure &) {
            return STG_E_ACCESSDENIED;
        } catch (const std::bad_alloc &) {
            return E_OUTOFMEMORY;
        } catch (const std::exception &) {
            return E_FAIL;
        } catch (...) {
            return E_UNEXPECTED;
        }

        return (*pcbRead < cb)
            ? S_FALSE
            : S_OK;
    }

    HRESULT com_istream::Write(const void * /* pv */,
                               ULONG /* cb */,
                               ULONG * /* pcbWritten */)
    {
        return E_NOTIMPL;
    }

    HRESULT com_istream::Seek(const LARGE_INTEGER dlibMove,
                              const DWORD dwOrigin,
                              ULARGE_INTEGER * const plibNewPosition)
    {
        try {
            using std::istream;

            switch (dwOrigin) {
            case STREAM_SEEK_SET:
                this->in_.seekg(static_cast<istream::pos_type>(dlibMove.QuadPart));
                break;
            case STREAM_SEEK_CUR:
                this->in_.seekg(static_cast<istream::off_type>(dlibMove.QuadPart),
                                std::ios_base::cur);
                break;
            case STREAM_SEEK_END:
                this->in_.seekg(static_cast<istream::off_type>(dlibMove.QuadPart),
                                std::ios_base::end);
                break;
            default:
                return STG_E_INVALIDFUNCTION;
            }

            if (!this->in_) {
                return STG_E_INVALIDFUNCTION;
            }

            if (!plibNewPosition) {
                return STG_E_INVALIDPOINTER;
            }
            plibNewPosition->QuadPart = this->in_.tellg();
        } catch (const std::ios_base::failure &) {
            return STG_E_ACCESSDENIED;
        } catch (const std::bad_alloc &) {
            return E_OUTOFMEMORY;
        } catch (const std::exception &) {
            return E_FAIL;
        } catch (...) {
            return E_UNEXPECTED;
        }

        return S_OK;
    }

    HRESULT com_istream::SetSize(ULARGE_INTEGER /* libNewSize */)
    {
        return E_NOTIMPL;
    }

    HRESULT com_istream::CopyTo(IStream * /* pstm */,
                                ULARGE_INTEGER /* cb */,
                                ULARGE_INTEGER * /* pcbRead */,
                                ULARGE_INTEGER * /* pcbWritten */)
    {
        return E_NOTIMPL;
    }

    HRESULT com_istream::Commit(DWORD /* grfCommitFlags */)
    {
        return E_NOTIMPL;
    }

    HRESULT com_istream::Revert()
    {
        return E_NOTIMPL;
    }

    HRESULT com_istream::LockRegion(ULARGE_INTEGER /* libOffset */,
                                    ULARGE_INTEGER /* cb */,
                                    DWORD /* dwLockType */)
    {
        return E_NOTIMPL;
    }

    HRESULT com_istream::UnlockRegion(ULARGE_INTEGER /* libOffset */,
                                      ULARGE_INTEGER /* cb */,
                                      DWORD /* dwLockType */)
    {
        return E_NOTIMPL;
    }

    HRESULT com_istream::Stat(STATSTG * /* pstatstg */,
                              DWORD /* grfStatFlag */)
    {
        return E_NOTIMPL;
    }

    HRESULT com_istream::Clone(IStream ** /* ppstm */)
    {
        return E_NOTIMPL;
    }
}
# else
void xml_reader_errorFunc(void * arg, const char * msg,
                          xmlParserSeverities severity,
                          xmlTextReaderLocatorPtr locator)
{
    xml::parse_error & error =
        *static_cast<xml::parse_error *>(arg);
    error =
        xml::parse_error(xmlTextReaderLocatorLineNumber(locator), msg);
}

int xml_reader_inputReadCallback(void * const context,
                                 char * const buffer,
                                 const int len)
{
    std::istream & in = *static_cast<std::istream *>(context);
    in.read(buffer, len);
    return in
        ? static_cast<int>(in.gcount())
        : -1;
}

int xml_reader_inputCloseCallback(void * /* context */)
{
    // Don't need to do anything to a std::istream here.
    return 0;
}
# endif // HAVE_XMLLITE
