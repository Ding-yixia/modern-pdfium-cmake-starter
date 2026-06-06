#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "pdfium/pdfium.h"

// ---------------------------------------------------------------------------
// Helper: convert wide string to UTF-8 for console output
// ---------------------------------------------------------------------------
static std::string WStringToUTF8(const std::wstring& wstr) {
    std::string out;
    for (wchar_t wc : wstr) {
        if (wc < 0x80) {
            out += static_cast<char>(wc);
        } else if (wc < 0x800) {
            out += static_cast<char>(0xC0 | (wc >> 6));
            out += static_cast<char>(0x80 | (wc & 0x3F));
        } else {
            out += static_cast<char>(0xE0 | (wc >> 12));
            out += static_cast<char>(0x80 | ((wc >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (wc & 0x3F));
        }
    }
    return out;
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::fprintf(stderr, "Usage: pdfium_text <pdf-file>\n");
        return 1;
    }

    // 1. Initialize and load PDF
    pdfium::Library lib;
    auto doc = pdfium::Document::LoadFromFile(argv[1]);
    if (!doc) {
        std::fprintf(stderr, "Failed to load '%s'\n", argv[1]);
        return 1;
    }
    std::cout << "Loaded: " << argv[1] << "\n";
    std::cout << "Pages  : " << doc->GetPageCount() << "\n";

    // 2. Load first page and its TextPage
    auto page = doc->LoadPage(0);
    if (!page) {
        std::fprintf(stderr, "Failed to load page 0.\n");
        return 1;
    }

    auto text_page = page->LoadTextPage();
    if (!text_page) {
        std::fprintf(stderr, "No text content on page 0.\n");
        return 1;
    }

    // 3. Extract all text from the page
    int char_count = text_page->CharCount();
    std::cout << "\n--- Full page text ---\n";
    if (char_count > 0) {
        std::wstring all_text = text_page->GetText(0, char_count);
        std::cout << WStringToUTF8(all_text) << "\n";
    } else {
        std::cout << "(page is empty of text)\n";
    }

    // 4. Get bounded text from a region (e.g. top-left quadrant)
    double pw = page->GetWidth();
    double ph = page->GetHeight();
    std::wstring bounded = text_page->GetBoundedText(0, 0, pw / 2.0, ph / 2.0);
    if (!bounded.empty()) {
        std::cout << "\n--- Bounded text (top-left quadrant) ---\n";
        std::cout << WStringToUTF8(bounded) << "\n";
    }

    // 5. Character details
    std::cout << "\n--- Character details ---\n";
    std::cout << "Total chars : " << char_count << "\n";
    int show_count = (char_count < 10) ? char_count : 10;
    std::cout << "First " << show_count << " Unicode values: ";
    for (int i = 0; i < show_count; ++i) {
        unsigned int cp = text_page->GetUnicode(i);
        std::cout << "U+" << std::hex << cp << std::dec << " ";
    }
    std::cout << "\n";

    // Also show font sizes for first few chars
    std::cout << "Font sizes (first " << show_count << "): ";
    for (int i = 0; i < show_count; ++i) {
        std::cout << text_page->GetFontSize(i) << " ";
    }
    std::cout << "\n";

    std::cout << "\nDone.\n";
    return 0;
}
