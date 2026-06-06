#pragma once
#include <string>
#include "types.h"

namespace pdfium {

class Save {
public:
    static bool SaveAsCopy(const Document& doc, const std::string& path, 
                          bool incremental = false, bool remove_security = false);
    static bool SaveWithVersion(const Document& doc, const std::string& path, int file_version,
                               bool incremental = false, bool remove_security = false);
};

} // namespace pdfium
