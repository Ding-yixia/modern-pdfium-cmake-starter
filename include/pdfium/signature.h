#pragma once
#include <memory>
#include <string>
#include <vector>
#include "types.h"

namespace pdfium {

class Signature {
public:
    std::vector<uint8_t> GetContents() const;
    std::vector<int> GetByteRange() const;
    std::string GetSubFilter() const;
    std::wstring GetReason() const;
    std::string GetTime() const;
    unsigned int GetDocMDPPermission() const;
    FPDF_SIGNATURE Handle() const { return sig_; }
    explicit operator bool() const { return sig_ != nullptr; }
private:
    friend class Document;
    Signature(FPDF_SIGNATURE sig);
    FPDF_SIGNATURE sig_;
};

} // namespace pdfium
