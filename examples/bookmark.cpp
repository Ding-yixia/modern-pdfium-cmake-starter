#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "pdfium/pdfium.h"

// ---------------------------------------------------------------------------
// Forward declarations
// ---------------------------------------------------------------------------
static void PrintActionType(const pdfium::Action* action);
static std::string WStringToUTF8(const std::wstring& wstr);

// ---------------------------------------------------------------------------
// Recursively traverse bookmarks
// ---------------------------------------------------------------------------
static void DumpBookmarks(const pdfium::Document& doc,
                           const pdfium::Bookmark& bm,
                           int depth) {
    std::wstring title = bm.GetTitle();
    std::string indent(static_cast<size_t>(depth) * 2, ' ');
    std::cout << indent << "- " << WStringToUTF8(title);

    // Get destination (page number)
    auto dest = bm.GetDest(doc);
    if (dest) {
        int page_idx = dest->GetPageIndex(doc);
        std::cout << "  [page " << page_idx << "]";
    }

    // Get action type
    auto action = bm.GetAction();
    if (action) {
        std::cout << "  action=";
        PrintActionType(action.get());
    }
    std::cout << "\n";

    // Recurse into children
    pdfium::Bookmark child = bm.GetFirstChild();
    while (child) {
        DumpBookmarks(doc, child, depth + 1);
        child = child.GetNextSibling();
    }
}

// ---------------------------------------------------------------------------
// Print action type as a human-readable string
// ---------------------------------------------------------------------------
static void PrintActionType(const pdfium::Action* action) {
    if (!action) {
        std::cout << "(none)";
        return;
    }
    unsigned long type = action->GetType();
    switch (type) {
        case 0:  std::cout << "Unsupported"; break;
        case 1:  std::cout << "GoTo";         break;
        case 2:  std::cout << "RemoteGoTo";   break;
        case 3:  std::cout << "URI";          break;
        case 4:  std::cout << "Launch";       break;
        case 5:  std::cout << "GoToR";        break;
        case 6:  std::cout << "Named";        break;
        default: std::cout << "Unknown(" << type << ")"; break;
    }
}

// ---------------------------------------------------------------------------
// Wide-string → UTF-8 helper
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
        std::fprintf(stderr, "Usage: pdfium_bookmark <pdf-file>\n");
        return 1;
    }

    // 1. Initialize and load PDF
    pdfium::Library lib;
    auto doc = pdfium::Document::LoadFromFile(argv[1]);
    if (!doc) {
        std::fprintf(stderr, "Failed to load '%s'\n", argv[1]);
        return 1;
    }
    std::cout << "Loaded: " << argv[1] << "  Pages: " << doc->GetPageCount() << "\n\n";

    // 2. Get root bookmark (pass default-constructed Bookmark to GetFirstChild)
    pdfium::Bookmark root;
    pdfium::Bookmark first = doc->GetFirstChild(&root);

    // 3. Recursively traverse all bookmarks
    if (first) {
        std::cout << "=== Bookmark tree ===\n";
        DumpBookmarks(*doc, first, 0);
    } else {
        std::cout << "(no bookmarks in this document)\n";
    }

    // 4. Find a specific bookmark by title
    const wchar_t* search_titles[] = {
        L"Introduction", L"Chapter 1", L"Contents",
        L"Overview", L"Summary", L"1",
    };
    for (const wchar_t* stitle : search_titles) {
        pdfium::Bookmark found = doc->FindBookmark(stitle);
        if (found) {
            std::wstring ftitle = found.GetTitle();
            std::cout << "\nFound bookmark: \"" << WStringToUTF8(ftitle) << "\"\n";
            auto fdest = found.GetDest(*doc);
            if (fdest) {
                std::cout << "  Page index: " << fdest->GetPageIndex(*doc) << "\n";
            }
            auto faction = found.GetAction();
            if (faction) {
                std::cout << "  Action: ";
                PrintActionType(faction.get());
                std::cout << "\n";
            }
            break;
        }
    }

    std::cout << "\nDone.\n";
    return 0;
}
