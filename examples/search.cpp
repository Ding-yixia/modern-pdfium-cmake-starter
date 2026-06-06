#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "pdfium/pdfium.h"

// ---------------------------------------------------------------------------
// Helper: wide-string → UTF-8
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
        std::fprintf(stderr, "Usage: pdfium_search <pdf-file> [search-term]\n");
        return 1;
    }

    // 1. Initialize and load PDF
    pdfium::Library lib;
    auto doc = pdfium::Document::LoadFromFile(argv[1]);
    if (!doc) {
        std::fprintf(stderr, "Failed to load '%s'\n", argv[1]);
        return 1;
    }
    std::cout << "Loaded: " << argv[1] << "  Pages: " << doc->GetPageCount() << "\n";

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
    int total_chars = text_page->CharCount();
    std::cout << "Page 0 has " << total_chars << " characters.\n";

    // 3. Determine search term
    std::wstring search_term;
    if (argc >= 3) {
        const char* s = argv[2];
        while (*s) {
            unsigned char c = static_cast<unsigned char>(*s);
            if (c < 0x80) {
                search_term += static_cast<wchar_t>(c);
            }
            ++s;
        }
    } else {
        std::cout << "Enter search term: ";
        std::string line;
        std::getline(std::cin, line);
        for (char c : line) {
            search_term += static_cast<wchar_t>(static_cast<unsigned char>(c));
        }
    }

    if (search_term.empty()) {
        std::cout << "No search term provided. Skipping text search.\n";
        return 0;
    }

    std::cout << "Searching for: \"" << WStringToUTF8(search_term) << "\"\n";

    // 4. Start search
    FPDF_SCHHANDLE find_handle =
        FPDFText_FindStart(text_page->Handle(),
                           reinterpret_cast<FPDF_WIDESTRING>(search_term.c_str()),
                           0, 0);
    if (!find_handle) {
        std::fprintf(stderr, "FPDFText_FindStart failed.\n");
        return 1;
    }

    // 5. Loop through all matches
    int match_count = 0;
    while (FPDFText_FindNext(find_handle)) {
        int idx = FPDFText_GetSchResultIndex(find_handle);
        int cnt = FPDFText_GetSchCount(find_handle);

        double left = 0, right = 0, bottom = 0, top = 0;
        text_page->GetCharBox(idx, &left, &right, &bottom, &top);

        std::cout << "  Match #" << (++match_count)
                  << "  char_index=" << idx
                  << "  count=" << cnt
                  << "  pos=(" << left << ", " << top << ")"
                  << "\n";
    }
    FPDFText_FindClose(find_handle);

    if (match_count == 0) {
        std::cout << "  (no matches found)\n";
    } else {
        std::cout << "Total matches: " << match_count << "\n";
    }

    // 6. Extract all URLs using WebLinks (via raw FPDF API since the
    //    wrapper's WebLinks has a private constructor not exposed by TextPage)
    {
        // Note: WebLinks wrapper exists in the library but its constructor
        // is private and only TextPage is a friend. Since TextPage doesn't
        // expose a LoadWebLinks() factory method, we use the underlying
        // FPDF C functions directly.
        FPDF_PAGELINK page_link = FPDFLink_LoadWebLinks(text_page->Handle());
        if (page_link) {
            int link_count = FPDFLink_CountWebLinks(page_link);
            std::cout << "\n--- WebLinks (URLs found: " << link_count << ") ---\n";
            for (int li = 0; li < link_count; ++li) {
                // Get URL length
                int url_len = FPDFLink_GetURL(page_link, li, nullptr, 0);
                if (url_len <= 0) continue;

                std::vector<unsigned short> url_buf(static_cast<size_t>(url_len));
                FPDFLink_GetURL(page_link, li, url_buf.data(), url_len);
                // Remove null terminator
                if (!url_buf.empty() && url_buf.back() == 0) {
                    url_buf.pop_back();
                }
                std::wstring url(url_buf.begin(), url_buf.end());
                std::cout << "  URL #" << li << ": " << WStringToUTF8(url) << "\n";

                int rect_count = FPDFLink_CountRects(page_link, li);
                for (int ri = 0; ri < rect_count; ++ri) {
                    double l = 0, t = 0, r = 0, b = 0;
                    if (FPDFLink_GetRect(page_link, li, ri, &l, &t, &r, &b)) {
                        std::cout << "    rect " << ri << ": (" << l << ", "
                                  << t << ", " << r << ", " << b << ")\n";
                    }
                }
            }
            FPDFLink_CloseWebLinks(page_link);
        } else {
            std::cout << "\n(no WebLinks on this page)\n";
        }
    }

    std::cout << "\nDone.\n";
    return 0;
}
