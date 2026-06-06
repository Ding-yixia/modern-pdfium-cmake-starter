#include "pdfium/form.h"
#include <fpdfview.h>
#include <fpdf_formfill.h>

namespace pdfium {

FormHandle::FormHandle(FPDF_FORMHANDLE handle) : handle_(handle) {}

FormHandle::~FormHandle() {
    if (handle_) {
        FPDFDOC_ExitFormFillEnvironment(handle_);
    }
}

FormHandle::FormHandle(FormHandle&& other) noexcept
    : handle_(other.handle_) {
    other.handle_ = nullptr;
}

FormHandle& FormHandle::operator=(FormHandle&& other) noexcept {
    if (this != &other) {
        if (handle_) FPDFDOC_ExitFormFillEnvironment(handle_);
        handle_ = other.handle_;
        other.handle_ = nullptr;
    }
    return *this;
}

} // namespace pdfium
