#pragma once
#include <memory>
#include "types.h"

namespace pdfium {

enum class Error : int {
    Success = 0,
    Unknown = 1,
    File = 2,
    Format = 3,
    Password = 4,
    Security = 5,
    Page = 6,
};

enum class SandboxPolicy : int {
    Default = 0,
    Disabled = 1,
};

enum class PrintMode : int {
    Emf = 0,
    TextOnly = 1,
    PostScript2 = 2,
    PostScript3 = 3,
    PostScript2PassThrough = 5,
    PostScript3PassThrough = 6,
    Annotations = 7,
};

class Library {
public:
    struct Config {
        int version = 2;
        int user_data_size = 0;
        const void* user_data = nullptr;
    };

    Library();
    explicit Library(const Config& config);
    ~Library();
    Library(const Library&) = delete;
    Library& operator=(const Library&) = delete;

    static Error GetLastError();
    static void SetSandBoxPolicy(SandboxPolicy policy);
    static void SetPrintMode(PrintMode mode);
    static const char* Version();

private:
    bool initialized_ = false;
    static bool global_initialized_;
};

} // namespace pdfium
