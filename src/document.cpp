#include "pdfium/document.h"
#include "pdfium/page.h"
#include "pdfium/bitmap.h"
#include "pdfium/form.h"
#include "pdfium/attachment.h"
#include "pdfium/signature.h"

#include <vector>

namespace pdfium {

namespace {

std::string Utf16leToUtf8(const void* utf16_data, unsigned long length_bytes) {
    if (!utf16_data || length_bytes < 2) {
        return {};
    }
    unsigned long char_count = length_bytes / 2;
    if (char_count > 0 &&
        static_cast<const unsigned short*>(utf16_data)[char_count - 1] == 0) {
        --char_count;
    }
    if (char_count == 0) {
        return {};
    }

    size_t utf8_len = 0;
    const unsigned short* input = static_cast<const unsigned short*>(utf16_data);
    for (unsigned long i = 0; i < char_count; ++i) {
        unsigned short cp = input[i];
        if (cp < 0x80) {
            utf8_len += 1;
        } else if (cp < 0x800) {
            utf8_len += 2;
        } else if (cp >= 0xD800 && cp <= 0xDFFF) {
            if (cp >= 0xD800 && cp <= 0xDBFF && i + 1 < char_count) {
                unsigned short next = input[i + 1];
                if (next >= 0xDC00 && next <= 0xDFFF) {
                    utf8_len += 4;
                    ++i;
                } else {
                    utf8_len += 3;
                }
            } else {
                utf8_len += 3;
            }
        } else {
            utf8_len += 3;
        }
    }

    std::string result(utf8_len, '\0');
    size_t pos = 0;
    for (unsigned long i = 0; i < char_count; ++i) {
        unsigned short cp = input[i];
        if (cp < 0x80) {
            result[pos++] = static_cast<char>(cp);
        } else if (cp < 0x800) {
            result[pos++] = static_cast<char>(0xC0 | (cp >> 6));
            result[pos++] = static_cast<char>(0x80 | (cp & 0x3F));
        } else if (cp >= 0xD800 && cp <= 0xDBFF && i + 1 < char_count) {
            unsigned short next = input[i + 1];
            if (next >= 0xDC00 && next <= 0xDFFF) {
                unsigned int codepoint =
                    0x10000 + (((cp - 0xD800) << 10) | (next - 0xDC00));
                result[pos++] = static_cast<char>(0xF0 | (codepoint >> 18));
                result[pos++] = static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
                result[pos++] = static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                result[pos++] = static_cast<char>(0x80 | (codepoint & 0x3F));
                ++i;
            } else {
                result[pos++] = static_cast<char>(0xE0 | (cp >> 12));
                result[pos++] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
                result[pos++] = static_cast<char>(0x80 | (cp & 0x3F));
            }
        } else if (cp >= 0xDC00 && cp <= 0xDFFF) {
            result[pos++] = static_cast<char>(0xE0 | (cp >> 12));
            result[pos++] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            result[pos++] = static_cast<char>(0x80 | (cp & 0x3F));
        } else {
            result[pos++] = static_cast<char>(0xE0 | (cp >> 12));
            result[pos++] = static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            result[pos++] = static_cast<char>(0x80 | (cp & 0x3F));
        }
    }
    return result;
}

std::string GetUtf16Text(unsigned long (*pdfium_func)(FPDF_DOCUMENT, FPDF_BYTESTRING, void*, unsigned long),
                          FPDF_DOCUMENT doc, const std::string& tag) {
    unsigned long len = pdfium_func(doc, tag.c_str(), nullptr, 0);
    if (len == 0) {
        return {};
    }
    std::vector<unsigned short> buf(len / 2 + 1, 0);
    unsigned long actual = pdfium_func(doc, tag.c_str(), buf.data(),
                                        static_cast<unsigned long>(buf.size() * 2));
    if (actual == 0) {
        return {};
    }
    return Utf16leToUtf8(buf.data(), actual);
}

std::string GetUtf16TextIndex(unsigned long (*pdfium_func)(FPDF_DOCUMENT, int, void*, unsigned long),
                               FPDF_DOCUMENT doc, int index) {
    unsigned long len = pdfium_func(doc, index, nullptr, 0);
    if (len == 0) {
        return {};
    }
    std::vector<unsigned short> buf(len / 2 + 1, 0);
    unsigned long actual = pdfium_func(doc, index, buf.data(),
                                        static_cast<unsigned long>(buf.size() * 2));
    if (actual == 0) {
        return {};
    }
    return Utf16leToUtf8(buf.data(), actual);
}

} // anonymous namespace

Document::Document(FPDF_DOCUMENT doc) : doc_(doc) {}

Document::~Document() {
    if (doc_) {
        FPDF_CloseDocument(doc_);
    }
}

Document::Document(Document&& other) noexcept : doc_(other.doc_) {
    other.doc_ = nullptr;
}

Document& Document::operator=(Document&& other) noexcept {
    if (this != &other) {
        if (doc_) {
            FPDF_CloseDocument(doc_);
        }
        doc_ = other.doc_;
        other.doc_ = nullptr;
    }
    return *this;
}

std::unique_ptr<Document> Document::LoadFromFile(const std::string& path,
                                                  const std::string& password) {
    FPDF_DOCUMENT doc = FPDF_LoadDocument(path.c_str(), password.c_str());
    if (!doc) {
        return nullptr;
    }
    return std::unique_ptr<Document>(new Document(doc));
}

std::unique_ptr<Document> Document::LoadFromMemory(const void* data, size_t size,
                                                    const std::string& password) {
    FPDF_DOCUMENT doc = FPDF_LoadMemDocument64(data, size, password.c_str());
    if (!doc) {
        return nullptr;
    }
    return std::unique_ptr<Document>(new Document(doc));
}

std::unique_ptr<Document> Document::LoadFromCustom(FPDF_FILEACCESS* file_access,
                                                    const std::string& password) {
    FPDF_DOCUMENT doc = FPDF_LoadCustomDocument(file_access, password.c_str());
    if (!doc) {
        return nullptr;
    }
    return std::unique_ptr<Document>(new Document(doc));
}

std::unique_ptr<Document> Document::Create() {
    FPDF_DOCUMENT doc = FPDF_CreateNewDocument();
    if (!doc) {
        return nullptr;
    }
    return std::unique_ptr<Document>(new Document(doc));
}

int Document::GetPageCount() const {
    return FPDF_GetPageCount(doc_);
}

std::unique_ptr<Page> Document::LoadPage(int index) {
    FPDF_PAGE page = FPDF_LoadPage(doc_, index);
    if (!page) {
        return nullptr;
    }
    return std::unique_ptr<Page>(new Page(page));
}

void Document::DeletePage(int index) {
    FPDFPage_Delete(doc_, index);
}

std::unique_ptr<Page> Document::NewPage(int index, double width, double height) {
    FPDF_PAGE page = FPDFPage_New(doc_, index, width, height);
    if (!page) {
        return nullptr;
    }
    return std::unique_ptr<Page>(new Page(page));
}

int Document::GetFileVersion() const {
    int version = 0;
    if (!FPDF_GetFileVersion(doc_, &version)) {
        return -1;
    }
    return version;
}

unsigned long Document::GetPermissions() const {
    return FPDF_GetDocPermissions(doc_);
}

unsigned long Document::GetUserPermissions() const {
    return FPDF_GetDocUserPermissions(doc_);
}

int Document::GetSecurityHandlerRevision() const {
    return FPDF_GetSecurityHandlerRevision(doc_);
}

std::string Document::GetMetaText(const std::string& tag) const {
    return GetUtf16Text(FPDF_GetMetaText, doc_, tag);
}

std::string Document::GetPageLabel(int page_index) const {
    return GetUtf16TextIndex(FPDF_GetPageLabel, doc_, page_index);
}

bool Document::GetPrintScaling() const {
    return FPDF_VIEWERREF_GetPrintScaling(doc_) != 0;
}

int Document::GetNumCopies() const {
    return FPDF_VIEWERREF_GetNumCopies(doc_);
}

int Document::GetDuplexType() const {
    return static_cast<int>(FPDF_VIEWERREF_GetDuplex(doc_));
}

Bookmark Document::GetFirstChild(const Bookmark* bookmark) const {
    FPDF_BOOKMARK bm = bookmark ? bookmark->Handle() : nullptr;
    return Bookmark(FPDFBookmark_GetFirstChild(doc_, bm), doc_);
}

Bookmark Document::FindBookmark(const std::wstring& title) const {
    return Bookmark(FPDFBookmark_Find(
        doc_, reinterpret_cast<FPDF_WIDESTRING>(title.c_str())), doc_);
}

int Document::CountNamedDests() const {
    return static_cast<int>(FPDF_CountNamedDests(doc_));
}

int Document::GetFormType() const {
    return FPDF_GetFormType(doc_);
}

std::unique_ptr<FormHandle> Document::InitFormFillEnvironment() {
    FPDF_FORMFILLINFO form_info{};
    form_info.version = 1;
    FPDF_FORMHANDLE handle = FPDFDOC_InitFormFillEnvironment(doc_, &form_info);
    if (!handle) {
        return nullptr;
    }
    return std::unique_ptr<FormHandle>(new FormHandle(handle));
}

int Document::GetAttachmentCount() const {
    return FPDFDoc_GetAttachmentCount(doc_);
}

std::unique_ptr<Attachment> Document::GetAttachment(int index) const {
    FPDF_ATTACHMENT attach = FPDFDoc_GetAttachment(doc_, index);
    if (!attach) {
        return nullptr;
    }
    return std::unique_ptr<Attachment>(new Attachment(attach));
}

std::unique_ptr<Attachment> Document::AddAttachment(const std::wstring& name) {
    FPDF_ATTACHMENT attach = FPDFDoc_AddAttachment(
        doc_, reinterpret_cast<FPDF_WIDESTRING>(name.c_str()));
    if (!attach) {
        return nullptr;
    }
    return std::unique_ptr<Attachment>(new Attachment(attach));
}

bool Document::DeleteAttachment(int index) {
    return FPDFDoc_DeleteAttachment(doc_, index) != 0;
}

int Document::GetSignatureCount() const {
    return FPDF_GetSignatureCount(doc_);
}

std::unique_ptr<Signature> Document::GetSignatureObject(int index) const {
    FPDF_SIGNATURE sig = FPDF_GetSignatureObj(doc_, index);
    if (!sig) {
        return nullptr;
    }
    return std::unique_ptr<Signature>(new Signature(sig));
}

int Document::GetJavaScriptActionCount() const {
    return FPDFDoc_GetJavaScriptActionCount(doc_);
}

bool Document::ImportPages(const Document& src_doc, const std::string& page_range,
                            int index) {
    FPDF_BYTESTRING range = page_range.empty() ? nullptr : page_range.c_str();
    return FPDF_ImportPages(doc_, src_doc.doc_, range, index) != 0;
}

bool Document::ImportPagesByIndices(const Document& src_doc,
                                     const std::vector<int>& page_indices,
                                     int index) {
    const int* indices_ptr = page_indices.empty() ? nullptr : page_indices.data();
    return FPDF_ImportPagesByIndex(
               doc_, src_doc.doc_, indices_ptr,
               static_cast<unsigned long>(page_indices.size()), index) != 0;
}

bool Document::IsTagged() const {
    return FPDFCatalog_IsTagged(doc_) != 0;
}

std::string Document::GetLanguage() const {
    unsigned long len = FPDFCatalog_GetLanguage(doc_, nullptr, 0);
    if (len == 0) {
        return {};
    }
    std::vector<unsigned short> buf(len / 2 + 1, 0);
    unsigned long actual = FPDFCatalog_GetLanguage(
        doc_, buf.data(), static_cast<unsigned long>(buf.size() * 2));
    if (actual == 0) {
        return {};
    }
    return Utf16leToUtf8(buf.data(), actual);
}

void Document::SetLanguage(const std::string& language) {
    std::wstring wlang;
    wlang.assign(language.begin(), language.end());
    FPDFCatalog_SetLanguage(
        doc_, reinterpret_cast<FPDF_WIDESTRING>(wlang.c_str()));
}

int Document::GetXFAPacketCount() const {
    return FPDF_GetXFAPacketCount(doc_);
}

std::string Document::GetXFAPacketName(int index) const {
    unsigned long len = FPDF_GetXFAPacketName(doc_, index, nullptr, 0);
    if (len == 0) {
        return {};
    }
    std::vector<char> buf(len, '\0');
    FPDF_GetXFAPacketName(doc_, index, buf.data(),
                           static_cast<unsigned long>(buf.size()));
    return std::string(buf.data());
}

std::vector<uint8_t> Document::GetXFAPacketContent(int index) const {
    unsigned long out_buflen = 0;
    if (!FPDF_GetXFAPacketContent(doc_, index, nullptr, 0, &out_buflen)) {
        return {};
    }
    std::vector<uint8_t> buf(out_buflen);
    if (!FPDF_GetXFAPacketContent(doc_, index, buf.data(),
                                   static_cast<unsigned long>(buf.size()),
                                   &out_buflen)) {
        return {};
    }
    buf.resize(out_buflen);
    return buf;
}

} // namespace pdfium
