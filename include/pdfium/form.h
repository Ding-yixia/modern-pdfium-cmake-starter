#pragma once
#include <memory>
#include <string>
#include "types.h"

namespace pdfium {

class FormHandle {
public:
    ~FormHandle();
    FormHandle(const FormHandle&) = delete;
    FormHandle& operator=(const FormHandle&) = delete;
    FormHandle(FormHandle&& other) noexcept;
    FormHandle& operator=(FormHandle&& other) noexcept;

    FPDF_FORMHANDLE Handle() const { return handle_; }
    explicit operator bool() const { return handle_ != nullptr; }

private:
    friend class Document;
    FormHandle(FPDF_FORMHANDLE handle);
    FPDF_FORMHANDLE handle_;
};

} // namespace pdfium
