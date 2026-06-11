#include "pdfium/signature.h"
#include "pdfium/document.h"
#include <fpdfview.h>
#include <fpdf_signature.h>
#include <vector>

namespace pdfium {

// ---------------------------------------------------------------------------
// Signature
// ---------------------------------------------------------------------------

Signature::Signature(FPDF_SIGNATURE sig) : sig_(sig) {}

std::vector<uint8_t> Signature::GetContents() const {
    unsigned long out_buflen = FPDFSignatureObj_GetContents(sig_, nullptr, 0);
    if (out_buflen == 0) {
        return {};
    }
    std::vector<uint8_t> buf(out_buflen);
    unsigned long actual = FPDFSignatureObj_GetContents(
        sig_, buf.data(), static_cast<unsigned long>(buf.size()));
    if (actual == 0) {
        return {};
    }
    buf.resize(actual);
    return buf;
}

std::vector<int> Signature::GetByteRange() const {
    unsigned long out_buflen = FPDFSignatureObj_GetByteRange(sig_, nullptr, 0);
    if (out_buflen == 0) {
        return {};
    }
    std::vector<int> buf(out_buflen);
    unsigned long actual = FPDFSignatureObj_GetByteRange(
        sig_, buf.data(), static_cast<unsigned long>(buf.size()));
    if (actual == 0) {
        return {};
    }
    buf.resize(actual);
    return buf;
}

std::string Signature::GetSubFilter() const {
    unsigned long len = FPDFSignatureObj_GetSubFilter(sig_, nullptr, 0);
    if (len <= 0) {
        return {};
    }
    std::vector<char> buf(len, '\0');
    FPDFSignatureObj_GetSubFilter(sig_, buf.data(),
                                   static_cast<unsigned long>(buf.size()));
    return std::string(buf.data());
}

std::wstring Signature::GetReason() const {
    unsigned long len = FPDFSignatureObj_GetReason(sig_, nullptr, 0);
    if (len <= 0) {
        return {};
    }
    std::vector<unsigned short> buf(len / 2 + 1, 0);
    FPDFSignatureObj_GetReason(sig_, buf.data(),
                                static_cast<unsigned long>(buf.size() * 2));
    if (!buf.empty() && buf.back() == 0) {
        buf.pop_back();
    }
    return std::wstring(buf.begin(), buf.end());
}

std::string Signature::GetTime() const {
    unsigned long len = FPDFSignatureObj_GetTime(sig_, nullptr, 0);
    if (len <= 0) {
        return {};
    }
    std::vector<char> buf(len, '\0');
    FPDFSignatureObj_GetTime(sig_, buf.data(),
                              static_cast<unsigned long>(buf.size()));
    return std::string(buf.data());
}

unsigned int Signature::GetDocMDPPermission() const {
    return FPDFSignatureObj_GetDocMDPPermission(sig_);
}

} // namespace pdfium
