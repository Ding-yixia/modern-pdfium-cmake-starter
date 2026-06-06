#include "pdfium/attachment.h"
#include "pdfium/document.h"
#include <fpdfview.h>
#include <fpdf_attachment.h>
#include <vector>

namespace pdfium {

// ---------------------------------------------------------------------------
// Attachment
// ---------------------------------------------------------------------------

Attachment::Attachment(FPDF_ATTACHMENT attach) : attach_(attach) {}

Attachment::~Attachment() {
    // FPDF_ATTACHMENT is owned by the document; no close/free needed.
}

Attachment::Attachment(Attachment&& other) noexcept : attach_(other.attach_) {
    other.attach_ = nullptr;
}

Attachment& Attachment::operator=(Attachment&& other) noexcept {
    if (this != &other) {
        attach_ = other.attach_;
        other.attach_ = nullptr;
    }
    return *this;
}

std::wstring Attachment::GetName() const {
    unsigned long len = FPDFAttachment_GetName(attach_, nullptr, 0);
    if (len <= 0) {
        return {};
    }
    std::vector<unsigned short> buf(len / 2 + 1, 0);
    FPDFAttachment_GetName(attach_, buf.data(),
                            static_cast<unsigned long>(buf.size() * 2));
    if (!buf.empty() && buf.back() == 0) {
        buf.pop_back();
    }
    return std::wstring(buf.begin(), buf.end());
}

bool Attachment::HasKey(const std::string& key) const {
    return FPDFAttachment_HasKey(attach_, key.c_str()) != 0;
}

std::wstring Attachment::GetStringValue(const std::string& key) const {
    unsigned long len = FPDFAttachment_GetStringValue(attach_, key.c_str(), nullptr, 0);
    if (len <= 0) {
        return {};
    }
    std::vector<unsigned short> buf(len / 2 + 1, 0);
    FPDFAttachment_GetStringValue(attach_, key.c_str(), buf.data(),
                                   static_cast<unsigned long>(buf.size() * 2));
    if (!buf.empty() && buf.back() == 0) {
        buf.pop_back();
    }
    return std::wstring(buf.begin(), buf.end());
}

bool Attachment::SetStringValue(const std::string& key, const std::wstring& value) {
    return FPDFAttachment_SetStringValue(attach_, key.c_str(),
                                          reinterpret_cast<FPDF_WIDESTRING>(value.c_str())) != 0;
}

bool Attachment::SetFile(const Document& doc, const void* contents, unsigned long len) {
    return FPDFAttachment_SetFile(
               attach_, doc.Handle(), contents, len) != 0;
}

std::vector<uint8_t> Attachment::GetFile() const {
    unsigned long out_buflen = 0;
    if (!FPDFAttachment_GetFile(attach_, nullptr, 0, &out_buflen)) {
        return {};
    }
    std::vector<uint8_t> buf(out_buflen);
    if (!FPDFAttachment_GetFile(attach_, buf.data(),
                                 static_cast<unsigned long>(buf.size()),
                                 &out_buflen)) {
        return {};
    }
    buf.resize(out_buflen);
    return buf;
}

std::wstring Attachment::GetSubtype() const {
    unsigned long len = FPDFAttachment_GetSubtype(attach_, nullptr, 0);
    if (len <= 0) {
        return {};
    }
    std::vector<unsigned short> buf(len / 2 + 1, 0);
    FPDFAttachment_GetSubtype(attach_, buf.data(),
                               static_cast<unsigned long>(buf.size() * 2));
    if (!buf.empty() && buf.back() == 0) {
        buf.pop_back();
    }
    return std::wstring(buf.begin(), buf.end());
}

} // namespace pdfium
