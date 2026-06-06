#include "pdfium/core.h"
#include <fpdfview.h>

namespace pdfium {

bool Library::global_initialized_ = false;

Library::Library() {
    if (!global_initialized_) {
        FPDF_InitLibrary();
        global_initialized_ = true;
        initialized_ = true;
    }
}

Library::Library(const Config& config) {
    if (!global_initialized_) {
        FPDF_LIBRARY_CONFIG cfg{};
        cfg.version = config.version;
        cfg.user_data_size = config.user_data_size;
        cfg.user_data = const_cast<void*>(config.user_data);
        FPDF_InitLibraryWithConfig(&cfg);
        global_initialized_ = true;
        initialized_ = true;
    }
}

Library::~Library() {
    if (initialized_ && global_initialized_) {
        FPDF_DestroyLibrary();
        global_initialized_ = false;
    }
}

Error Library::GetLastError() {
    return static_cast<Error>(FPDF_GetLastError());
}

void Library::SetSandBoxPolicy(SandboxPolicy policy) {
    FPDF_SetSandBoxPolicy(static_cast<FPDF_DWORD>(policy), 1);
}

void Library::SetPrintMode(PrintMode mode) {
    FPDF_SetPrintMode(static_cast<int>(mode));
}

const char* Library::Version() {
    return "PDFium via modern-pdfium wrapper";
}

} // namespace pdfium
