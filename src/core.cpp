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
        cfg.m_pUserFontPaths = config.m_pUserFontPaths;
        cfg.m_pIsolate = config.m_pIsolate;
        cfg.m_v8EmbedderSlot = config.m_v8EmbedderSlot;
        cfg.m_pPlatform = config.m_pPlatform;
        cfg.m_RendererType = static_cast<FPDF_RENDERER_TYPE>(config.m_RendererType);
        cfg.m_FontLibraryType = static_cast<FPDF_FONT_BACKEND_TYPE>(config.m_FontLibraryType);
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
