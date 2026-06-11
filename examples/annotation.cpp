#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "pdfium/pdfium.h"

// ---------------------------------------------------------------------------
// Helper: convert AnnotationSubtype to readable string
// ---------------------------------------------------------------------------
static const char* SubtypeName(pdfium::AnnotationSubtype st) {
    using ST = pdfium::AnnotationSubtype;
    switch (st) {
        case ST::Unknown:        return "Unknown";
        case ST::Text:           return "Text";
        case ST::Link:           return "Link";
        case ST::FreeText:       return "FreeText";
        case ST::Line:           return "Line";
        case ST::Square:         return "Square";
        case ST::Circle:         return "Circle";
        case ST::Polygon:        return "Polygon";
        case ST::Polyline:       return "Polyline";
        case ST::Highlight:      return "Highlight";
        case ST::Underline:      return "Underline";
        case ST::Squiggly:       return "Squiggly";
        case ST::StrikeOut:      return "StrikeOut";
        case ST::Stamp:          return "Stamp";
        case ST::Caret:          return "Caret";
        case ST::Ink:            return "Ink";
        case ST::Popup:          return "Popup";
        case ST::FileAttachment: return "FileAttachment";
        case ST::Sound:          return "Sound";
        case ST::Movie:          return "Movie";
        case ST::Widget:         return "Widget";
        case ST::Screen:         return "Screen";
        case ST::PrinterMark:    return "PrinterMark";
        case ST::TrapNet:        return "TrapNet";
        case ST::Watermark:      return "Watermark";
        case ST::ThreeD:         return "ThreeD";
        case ST::RichMedia:      return "RichMedia";
        case ST::XFAWidget:      return "XFAWidget";
        case ST::Redact:         return "Redact";
        default:                 return "???";
    }
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::fprintf(stderr, "Usage: pdfium_annot <pdf-file>\n");
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

    // 2. Load first page
    auto page = doc->LoadPage(0);
    if (!page) {
        std::fprintf(stderr, "Failed to load page 0.\n");
        return 1;
    }
    double pw = page->GetWidth();
    double ph = page->GetHeight();
    std::cout << "Page 0 size: " << pw << " x " << ph << "\n";

    // 3. Print annotation count
    int annot_count = page->GetAnnotCount();
    std::cout << "Annotation count: " << annot_count << "\n";

    // 4. Iterate over annotations (using raw FPDF API for direct annot access)
    //    The Page class provides GetAnnotCount() but not GetAnnot() — we use
    //    the FPDF C functions directly to enumerate annotations.
    for (int i = 0; i < annot_count; ++i) {
        FPDF_ANNOTATION handle = FPDFPage_GetAnnot(page->Handle(), i);
        if (!handle) continue;

        pdfium::Annotation annot(handle);

        // Subtype
        auto subtype = annot.GetSubtype();
        std::cout << "\n  Annot #" << i << "\n";
        std::cout << "    Subtype : " << SubtypeName(subtype)
                  << " (enum " << static_cast<int>(subtype) << ")\n";

        // Rectangle
        FS_RECTF rect = annot.GetRect();
        std::cout << "    Rect    : ["
                  << rect.left << ", " << rect.bottom << ", "
                  << rect.right << ", " << rect.top << "]\n";

        // Flags
        int flags = annot.GetFlags();
        std::cout << "    Flags   : 0x" << std::hex << flags << std::dec << "\n";

        // Color
        unsigned int R = 0, G = 0, B = 0, A = 0;
        if (annot.GetColor(&R, &G, &B, &A)) {
            std::cout << "    Color   : RGBA(" << R << "," << G << ","
                      << B << "," << A << ")\n";
        } else {
            std::cout << "    Color   : (none/default)\n";
        }

        // Contents (if any)
        if (annot.HasKey("Contents")) {
            std::wstring contents = annot.GetStringValue("Contents");
            // Print first 80 characters
            if (!contents.empty()) {
                std::wstring preview = contents.substr(0, 80);
                std::cout << "    Contents: ";
                for (wchar_t wc : preview) {
                    if (wc < 128) std::cout << static_cast<char>(wc);
                    else          std::cout << "?";
                }
                if (contents.size() > 80) std::cout << "...";
                std::cout << "\n";
            }
        }

        // Note: Annotation is closed by the destructor when it goes out of scope.
    }

    // 5. Demonstrate creating a simple text annotation
    {
        std::cout << "\n--- Creating a sample text annotation ---\n";
        // Add a new text annotation to the page via FPDF C API
        FPDF_ANNOTATION new_annot_handle =
            FPDFPage_CreateAnnot(page->Handle(), FPDF_ANNOT_TEXT);
        if (new_annot_handle) {
            pdfium::Annotation new_annot(new_annot_handle);

            FS_RECTF new_rect{100.0f, 500.0f, 200.0f, 560.0f};
            new_annot.SetRect(new_rect);
            new_annot.SetColor(255, 0, 0, 255);   // red
            new_annot.SetFlags(0);
            new_annot.SetStringValue("Contents", L"Sample annotation from pdfium_annot");

            std::cout << "Created text annotation at (100, 500, 200, 560)\n";
            std::cout << "Final annot count: " << page->GetAnnotCount() << "\n";
        } else {
            std::cout << "Failed to create annotation (page may be read-only).\n";
        }
        // new_annot is closed when it goes out of scope; the annotation
        // remains attached to the page.
    }

    std::cout << "\nDone.\n";
    return 0;
}
