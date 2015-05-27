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

# include "writer.h"
# include "finally.h"
# include <ostream>
# ifdef HAVE_XMLLITE
#   include "stringconvert.h"
#   include "xmllite_errmsg.h"
#   include <XmlLite.h>
#   include <shlwapi.h>
# else
#   include <libxml/xmlwriter.h>
# endif

/**
 * @file xml/writer.h
 *
 * @brief XML writer.
 */

/**
 * @class xml::write_error
 *
 * @brief Exception thrown when @c xml::writer encounters an error.
 */

/**
 * @brief Construct.
 *
 * @param[in] msg  a message describing the error
 */
xml::write_error::write_error(const std::string & msg):
    std::runtime_error{msg}
{}

/**
 * @class xml::writer
 *
 * @brief A wrapper that abstracts commonality between libxml's XmlTextWriter
 *        interface and Microsoft's XmlLite.
 */

/**
 * @internal
 *
 * @brief Using the pimpl idiom here avoids exposing libxml2/XmlLite types in
 *        the header.
 */
struct xml::writer::impl {
# ifdef HAVE_XMLLITE
    IStream * output;
    IXmlWriter * writer;
# else
    xmlTextWriterPtr writer;
# endif

    explicit impl(const std::string & filename);
    explicit impl(std::ostream & out);
    impl(const impl &) = delete;
    ~impl() throw ();

    impl & operator=(const impl &) = delete;
};

/**
 * @var xmlTextWriterPtr xml::writer::impl::writer
 *
 * @brief The <a href="http://www.xmlsoft.org/html/libxml-xmlwriter.html#xmlTextWriter">`xmlTextWriter`</a>.
 */

namespace {
# ifdef HAVE_XMLLITE
    // From: <http://msdn.microsoft.com/en-us/library/windows/desktop/dd317756.aspx>
    const UINT utf8_code_page = 65001;
# endif
}

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
xml::writer::impl::impl(const std::string & filename):
# ifdef HAVE_XMLLITE
    output{0},
# endif
    writer{0}
{
# ifdef HAVE_XMLLITE
    HRESULT hr;
    bool succeeded = false;

    hr = SHCreateStreamOnFile(
            detail::utf8_to_utf16(filename).c_str(),
            STGM_CREATE | STGM_WRITE,
            &this->output);
    detail::finally f1([&]{
        if (!succeeded && output != nullptr) { output->Release(); }
	});
    if (FAILED(hr)) {
        if (hr == E_OUTOFMEMORY) {
            throw std::bad_alloc{};
        }
        throw std::runtime_error{"failed to open file \"" + filename + '\"'};
    }

    static IMalloc * const malloc = 0;
    IXmlWriterOutput * xml_output = 0;
    hr = CreateXmlWriterOutputWithEncodingCodePage(this->output,
                                                   malloc,
                                                   utf8_code_page,
                                                   &xml_output);
    if (FAILED(hr)) {
        throw std::runtime_error{"failed to create XML output"};
    }

    hr = CreateXmlReader(__uuidof(IXmlWriter),
                         reinterpret_cast<void **>(&this->writer),
                         0);
    detail::finally f2([&]{
        if (!succeeded && this->writer != nullptr) { this->writer->Release(); }
	});
    if (FAILED(hr)) {
        if (hr == E_OUTOFMEMORY) {
            throw std::bad_alloc{};
        }
        throw std::runtime_error{"failed to create XML writer"};
    }

    hr = this->writer->SetOutput(xml_output);
    if (FAILED(hr)) {
        throw std::runtime_error{"failed to set input for XML writer"};
    }

    succeeded = true;
# else
    static const int compress = 0;
    this->writer = xmlNewTextWriterFilename(filename.c_str(), compress);
    if (!this->writer) {
        throw std::runtime_error{"failed to create XML writer"};
    }
# endif
}

# ifdef HAVE_XMLLITE
namespace
{
    class com_ostream : public ::IStream {
        std::ostream & out_;
        LONG count_;

    public:
        explicit com_ostream(std::ostream & out);
        com_ostream(const com_ostream &) = delete;

        com_ostream & operator=(const com_ostream &) = delete;

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
    int xml_writer_outputWriteCallback(void * context, const char * buffer, int len);
    int xml_writer_outputCloseCallback(void * context);
}
# endif

/**
 * @internal
 *
 * @brief Construct using an output stream.
 *
 * @param[in,out] out  an output stream.
 *
 * @exception std::runtime_error   if XmlLite/libxml2 setup fails
 * @exception std::bad_alloc       if memory allocation fails
 */
xml::writer::impl::impl(std::ostream & out):
# ifdef HAVE_XMLLITE
    output{new com_ostream{out}},
# endif
    writer{0}
{
    bool succeeded = false;

# ifdef HAVE_XMLLITE
    detail::finally f1([&]{
        if (!succeeded && output != nullptr) { output->Release(); }
    });

    HRESULT hr;

    static IMalloc * const malloc = 0;
    IXmlWriterOutput * xml_output = 0;
    hr = CreateXmlWriterOutputWithEncodingCodePage(this->output,
                                                   malloc,
                                                   utf8_code_page,
                                                   &xml_output);
    if (FAILED(hr)) {
        throw std::runtime_error{"failed to create XML output"};
    }

    hr = CreateXmlWriter(__uuidof(IXmlWriter),
                         reinterpret_cast<void **>(&this->writer),
                         malloc);
    detail::finally f2([&]{
        if (!succeeded && this->writer != nullptr) { this->writer->Release(); }
    });
    if (FAILED(hr)) {
        throw std::runtime_error{"failed to create XML writer"};
    }

    hr = this->writer->SetOutput(xml_output);
    if (FAILED(hr)) {
        throw std::runtime_error{"failed to set output for XML writer"};
    }
# else
    //
    // libxml will clean up the xmlOutputBuffer when cleaning up the
    // associated xmlTextWriter; so as long as we succeed here, we don't need
    // to look after it further.
    //
    const xmlCharEncodingHandlerPtr encoding_handler =
        xmlGetCharEncodingHandler(XML_CHAR_ENCODING_UTF8);
    const xmlOutputBufferPtr out_buf =
        xmlOutputBufferCreateIO(xml_writer_outputWriteCallback,
                                xml_writer_outputCloseCallback,
                                &out,
                                encoding_handler);
    if (!out_buf) {
        throw std::runtime_error{"failed to create XML output buffer"};
    }
    detail::finally f([&]{
        if (!succeeded && out_buf != nullptr) { xmlOutputBufferClose(out_buf); }
    });
    this->writer = xmlNewTextWriter(out_buf);
    if (!this->writer) {
        throw std::runtime_error{"failed to create XML writer"};
    }
# endif

    succeeded = true;
}

/**
 * @fn xml::writer::impl::impl(const impl &)
 *
 * @brief Not copyable.
 */

/**
 * @internal
 *
 * @brief Destroy.
 */
xml::writer::impl::~impl() throw ()
{
# ifdef HAVE_XMLLITE
    this->writer->Release();
    this->output->Release();
# else
    xmlFreeTextWriter(this->writer);
# endif
}

/**
 * @fn xml::writer::impl & xml::writer::impl::operator=(const impl &)
 *
 * @brief Not copyable.
 */

/**
 * @enum xml::writer::standalone
 *
 * @brief The value of the `standalone` attribute in the XML declaration.
 *
 * A document is "standalone" if all entity declarations required by the XML
 * document are contained within the document.
 */

/**
 * @var xml::writer::standalone xml::writer::omit
 *
 * @brief Used to indicate that the `standalone` attribute should be omitted.
 */

/**
 * @var xml::writer::standalone xml::writer::yes
 *
 * @brief Used to indicate that the XML document is "standalone".
 */

/**
 * @var xml::writer::standalone xml::writer::no
 *
 * @brief Used to indicate that the XML document is not "standalone".
 */

/**
 * @brief Construct from a file name.
 *
 * @param[in] filename  the full path to a file.
 *
 * @exception std::runtime_error    if opening @p filename or creating the
 *                                  underlying XML writer fails.
 * @exception std::bad_alloc        if memory allocation fails.
 */
xml::writer::writer(const std::string & filename):
    impl_{new impl{filename}}
{}

/**
 * @brief Construct from an output stream.
 *
 * @param[in,out] out   an output stream.
 *
 * @exception std::runtime_error    if creating the underlying XML writer
 *                                  fails.
 * @exception std::bad_alloc        if memory allocation fails.
 */
xml::writer::writer(std::ostream & out):
    impl_{new impl{out}}
{}

/**
 * @fn xml::writer::writer(const writer &)
 *
 * @brief Not copyable.
 */

/**
 * @brief Move construct.
 */
xml::writer::writer(writer && w) throw ():
    impl_{std::move(w.impl_)}
{}

/**
 * @fn xml::writer & xml::writer::operator=(const writer &)
 *
 * @brief Not copyable.
 */

/**
 * @brief Move assign.
 */
xml::writer & xml::writer::operator=(writer && w) throw ()
{
    this->impl_ = std::move(w.impl_);
    return *this;
}

/**
 * @brief Start the document.
 *
 * @param[in] sa    the value of the `standalone` attribute in the XML
 *                  declaration.
 *
 * @exception xml::write_error  if there is an error starting the document.
 */
void xml::writer::start_document(standalone sa)
{
# ifdef HAVE_XMLLITE
    const HRESULT hr =
        this->impl_->writer->WriteStartDocument(
            static_cast<XmlStandalone>(sa));
    if (FAILED(hr)) {
        if (detail::is_xmllite_writer_error(hr)) {
            detail::throw_write_error(hr);
        }
    }
# else
    static const char * const version = NULL; // default (1.0)
    static const char encoding[] = "utf-8";
    const char * const sa_str = (sa == standalone::yes)
                              ? "yes"
                              : (sa == standalone::no)
                                ? "no"
                                : NULL;
    const int bytes_written = xmlTextWriterStartDocument(this->impl_->writer,
                                                         version,
                                                         encoding,
                                                         sa_str);
    if (bytes_written == -1) {
        throw write_error{"error starting document"};
    }
# endif
}

/**
 * @brief End the document.
 *
 * All open elements are closed and the content is flushed to the output.
 *
 * @exception xml::write_error  if there is an error ending the document.
 */
void xml::writer::end_document()
{
# ifdef HAVE_XMLLITE
# else
    const int bytes_written = xmlTextWriterEndDocument(this->impl_->writer);
    if (bytes_written == -1) {
        throw write_error{"error ending document"};
    }
# endif
}

/**
 * @brief Start an element.
 *
 * @param[in] prefix        namespace prefix.
 * @param[in] local_name    local name.
 * @param[in] namespace_uri namespace URI.
 *
 * @exception xml::write_error  if there is an error starting the element.
 */
void xml::writer::start_element(const std::string & prefix,
                                const std::string & local_name,
                                const std::string & namespace_uri)
{
# ifdef HAVE_XMLLITE
    const HRESULT hr =
        this->impl_->writer->WriteStartElement(
            detail::utf8_to_utf16(prefix).c_str(),
            detail::utf8_to_utf16(local_name).c_str(),
            detail::utf8_to_utf16(namespace_uri).c_str());
    if (FAILED(hr)) {
        if (detail::is_xmllite_writer_error(hr)) {
            detail::throw_write_error(hr);
        }
    }
# else
    const int bytes_written =
        xmlTextWriterStartElementNS(
            this->impl_->writer,
            reinterpret_cast<const xmlChar *>(prefix.c_str()),
            reinterpret_cast<const xmlChar *>(local_name.c_str()),
            reinterpret_cast<const xmlChar *>(namespace_uri.c_str()));
    if (bytes_written == -1) {
        throw write_error{"error starting element"};
    }
# endif
}

/**
 * @brief End an element.
 *
 * @exception xml::write_error  if there is an error ending the element.
 */
void xml::writer::end_element()
{
# ifdef HAVE_XMLLITE
    const HRESULT hr = this->impl_->writer->WriteEndElement();
    if (FAILED(hr)) {
        if (detail::is_xmllite_writer_error(hr)) {
            detail::throw_write_error(hr);
        }
    }
# else
    const int bytes_written = xmlTextWriterEndElement(this->impl_->writer);
    if (bytes_written == -1) {
        throw write_error{"error ending element"};
    }
# endif
}

/**
 * @brief Write an attribute.
 *
 * @param[in] prefix        namespace prefix.
 * @param[in] local_name    local name.
 * @param[in] namespace_uri namespace URI.
 * @param[in] value         attribute value.
 *
 * @exception xml::write_error  if there is an error writing the attribute.
 */
void xml::writer::attribute(const std::string & prefix,
                            const std::string & local_name,
                            const std::string & namespace_uri,
                            const std::string & value)
{
# ifdef HAVE_XMLLITE
    const HRESULT hr =
        this->impl_->writer->WriteAttributeString(
            detail::utf8_to_utf16(prefix).c_str(),
            detail::utf8_to_utf16(local_name).c_str(),
            detail::utf8_to_utf16(namespace_uri).c_str(),
            detail::utf8_to_utf16(value).c_str());
    if (FAILED(hr)) {
        if (detail::is_xmllite_writer_error(hr)) {
            detail::throw_write_error(hr);
        }
    }
# else
    const int bytes_written =
        xmlTextWriterWriteAttributeNS(
            this->impl_->writer,
            reinterpret_cast<const xmlChar *>(prefix.c_str()),
            reinterpret_cast<const xmlChar *>(local_name.c_str()),
            reinterpret_cast<const xmlChar *>(namespace_uri.c_str()),
            reinterpret_cast<const xmlChar *>(value.c_str()));
    if (bytes_written == -1) {
        throw write_error{"error writing attribute"};
    }
# endif
}

/**
 * @brief Write comment.
 *
 * @param[in] text comment text.
 *
 * @exception xml::write_error  if there is an error writing the comment.
 */
void xml::writer::comment(const std::string & text)
{
# ifdef HAVE_XMLLITE
    const HRESULT hr =
        this->impl_->writer->WriteComment(detail::utf8_to_utf16(text).c_str());
    if (FAILED(hr)) {
        if (detail::is_xmllite_writer_error(hr)) {
            detail::throw_write_error(hr);
        }
    }
# else
    const int bytes_written =
        xmlTextWriterWriteComment(
            this->impl_->writer,
            reinterpret_cast<const xmlChar *>(text.c_str()));
    if (bytes_written == -1) {
        throw write_error{"error writing comment"};
    }
# endif
}

# ifdef HAVE_XMLLITE
namespace
{
    com_ostream::com_ostream(std::ostream & out):
        out_{out},
        count_{1}
    {}

    HRESULT com_ostream::QueryInterface(const IID & iid, void ** ppv)
    {
        if (!ppv) {
            return E_INVALIDARG;
        }
        if (   iid == __uuidof(IUnknown)
            || iid == __uuidof(IStream)
            || iid == __uuidof(ISequentialStream)) {
            *ppv = static_cast<IStream *>(this);
            this->AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    ULONG com_ostream::AddRef()
    {
        return ::InterlockedIncrement(&this->count_);
    }

    ULONG com_ostream::Release()
    {
        if (::InterlockedDecrement(&this->count_) == 0) {
            delete this;
            return 0;
        }
        return this->count_;
    }

    HRESULT com_ostream::Read(void * /* pv */,
                              ULONG /* cb */,
                              ULONG * /* pcbRead */)
    {
        return E_NOTIMPL;
    }

    HRESULT com_ostream::Write(const void * const pv,
                               const ULONG cb,
                               ULONG * const pcbWritten)
    {
        if (!pv) {
            return STG_E_INVALIDPOINTER;
        }
        try {
            const std::ostream::streampos init_pos = this->out_.tellp();
            this->out_.write(static_cast<const std::ostream::char_type *>(pv), cb);
            if (pcbWritten) {
                *pcbWritten = static_cast<ULONG>(this->out_.tellp() - init_pos);
            }
            if (!this->out_) {
                return STG_E_CANTSAVE;
            }
        } catch (const std::ios_base::failure &) {
            return STG_E_CANTSAVE;
        } catch (const std::bad_alloc &) {
            return E_OUTOFMEMORY;
        } catch (const std::exception &) {
            return E_FAIL;
        } catch (...) {
            return E_UNEXPECTED;
        }
        return S_OK;
    }

    HRESULT com_ostream::Seek(LARGE_INTEGER /* dlibMove */,
                              DWORD /* dwOrigin */,
                              ULARGE_INTEGER * /* plibNewPosition */)
    {
        return E_NOTIMPL;
    }

    HRESULT com_ostream::SetSize(ULARGE_INTEGER /* libNewSize */)
    {
        return E_NOTIMPL;
    }

    HRESULT com_ostream::CopyTo(IStream * /* pstm */,
                                ULARGE_INTEGER /* cb */,
                                ULARGE_INTEGER * /* pcbRead */,
                                ULARGE_INTEGER * /* pcbWritten */)
    {
        return E_NOTIMPL;
    }

    HRESULT com_ostream::Commit(DWORD /* grfCommitFlags */)
    {
        return E_NOTIMPL;
    }

    HRESULT com_ostream::Revert()
    {
        return E_NOTIMPL;
    }

    HRESULT com_ostream::LockRegion(ULARGE_INTEGER /* libOffset */,
                                    ULARGE_INTEGER /* cb */,
                                    DWORD /* dwLockType */)
    {
        return E_NOTIMPL;
    }

    HRESULT com_ostream::UnlockRegion(ULARGE_INTEGER /* libOffset */,
                                      ULARGE_INTEGER /* cb */,
                                      DWORD /* dwLockType */)
    {
        return E_NOTIMPL;
    }

    HRESULT com_ostream::Stat(STATSTG * /* pstatstg */,
                              DWORD /* grfStatFlag */)
    {
        return E_NOTIMPL;
    }

    HRESULT com_ostream::Clone(IStream ** /* ppstm */)
    {
        return E_NOTIMPL;
    }
}
# else
int xml_writer_outputWriteCallback(void * context, const char * buffer, int len)
{
    std::ostream & out = *static_cast<std::ostream *>(context);
    const std::ostream::streampos init_pos = out.tellp();
    out.write(buffer, len);
    return out
        ? out.tellp() - init_pos
        : -1;
}

int xml_writer_outputCloseCallback(void * context)
{
    // Don't need to do anything to a std::ostream here.
    return 0;
}
# endif // HAVE_XMLLITE
