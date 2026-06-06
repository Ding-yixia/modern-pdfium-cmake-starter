#pragma once
#include <memory>
#include <string>
#include <vector>
#include "types.h"

namespace pdfium {

class Attachment {
public:
    ~Attachment();
    Attachment(const Attachment&) = delete;
    Attachment& operator=(const Attachment&) = delete;
    Attachment(Attachment&& other) noexcept;
    Attachment& operator=(Attachment&& other) noexcept;
    std::wstring GetName() const;
    bool HasKey(const std::string& key) const;
    std::wstring GetStringValue(const std::string& key) const;
    bool SetStringValue(const std::string& key, const std::wstring& value);
    bool SetFile(const Document& doc, const void* contents, unsigned long len);
    std::vector<uint8_t> GetFile() const;
    std::wstring GetSubtype() const;
    FPDF_ATTACHMENT Handle() const { return attach_; }
    explicit operator bool() const { return attach_ != nullptr; }
private:
    friend class Document;
    Attachment(FPDF_ATTACHMENT attach);
    FPDF_ATTACHMENT attach_;
};

} // namespace pdfium
