#include "pdfium/save.h"
#include "pdfium/document.h"
#include <fpdfview.h>
#include <fpdf_save.h>
#include <fstream>

namespace pdfium {

namespace {

struct FileWrite : public FPDF_FILEWRITE {
    std::ofstream file;

    explicit FileWrite(const std::string& path)
        : file(path, std::ios::binary) {
        version = 1;
        WriteBlock = [](FPDF_FILEWRITE* self, const void* data,
                        unsigned long size) -> int {
            auto* fw = static_cast<FileWrite*>(self);
            if (!fw->file.is_open()) {
                return 0;
            }
            fw->file.write(static_cast<const char*>(data),
                           static_cast<std::streamsize>(size));
            return fw->file.good() ? 1 : 0;
        };
    }

    bool IsOpen() const { return file.is_open(); }
};

} // anonymous namespace

// ---------------------------------------------------------------------------
// Save
// ---------------------------------------------------------------------------

bool Save::SaveAsCopy(const Document& doc, const std::string& path,
                       bool incremental, bool remove_security) {
    FileWrite writer(path);
    if (!writer.IsOpen()) {
        return false;
    }
    unsigned int flags = 0;
    if (incremental) {
        flags |= FPDF_INCREMENTAL;
    }
    if (remove_security) {
        flags |= FPDF_REMOVE_SECURITY;
    }
    return FPDF_SaveAsCopy(doc.Handle(), &writer, flags) != 0;
}

bool Save::SaveWithVersion(const Document& doc, const std::string& path,
                            int file_version, bool incremental,
                            bool remove_security) {
    FileWrite writer(path);
    if (!writer.IsOpen()) {
        return false;
    }
    unsigned int flags = 0;
    if (incremental) {
        flags |= FPDF_INCREMENTAL;
    }
    if (remove_security) {
        flags |= FPDF_REMOVE_SECURITY;
    }
    return FPDF_SaveWithVersion(doc.Handle(), &writer, flags, file_version) != 0;
}

} // namespace pdfium
