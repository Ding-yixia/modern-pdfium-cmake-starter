#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

#include "pdfium/pdfium.h"

// ----------------------------------------------------------
// Helper: Save a pdfium::Bitmap as a PPM file (P6 format)
// ----------------------------------------------------------
static bool SaveBitmapToPPM(const pdfium::Bitmap& bitmap, const std::string& path) {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;
    int w = bitmap.GetWidth(), h = bitmap.GetHeight();
    fprintf(f, "P6\n%d %d\n255\n", w, h);
    unsigned char* buf = (unsigned char*)bitmap.GetBuffer();
    int stride = bitmap.GetStride();
    for (int y = 0; y < h; y++) {
        unsigned char* row = buf + y * stride;
        for (int x = 0; x < w; x++) {
            fputc(row[x*3+2], f); // R
            fputc(row[x*3+1], f); // G
            fputc(row[x*3+0], f); // B
        }
    }
    fclose(f);
    return true;
}

// ----------------------------------------------------------
// main
// ----------------------------------------------------------
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::fprintf(stderr, "Usage: pdfium_basic <pdf-file> [output-ppm]\n");
        return 1;
    }

    const std::string pdf_path = argv[1];
    const std::string ppm_path = (argc >= 3) ? argv[2] : "output.ppm";

    // 1. Initialize the PDFium library
    pdfium::Library lib;
    std::cout << "PDFium library initialized.\n";

    // 2. Load document
    auto doc = pdfium::Document::LoadFromFile(pdf_path);
    if (!doc) {
        pdfium::Error err = pdfium::Library::GetLastError();
        std::fprintf(stderr, "Failed to load PDF '%s' (error %d)\n",
                     pdf_path.c_str(), static_cast<int>(err));
        return 1;
    }
    std::cout << "Loaded: " << pdf_path << "\n";

    // 3. Print document info
    int page_count = doc->GetPageCount();
    int version = doc->GetFileVersion();
    unsigned long permissions = doc->GetPermissions();
    std::cout << "  Page count : " << page_count << "\n";
    std::cout << "  Version    : " << ((version >= 0) ? std::to_string(version) : "unknown") << "\n";
    std::cout << "  Permissions: 0x" << std::hex << permissions << std::dec << "\n";

    // Metadata
    std::string title    = doc->GetMetaText("Title");
    std::string author   = doc->GetMetaText("Author");
    std::string subject  = doc->GetMetaText("Subject");
    std::string keywords = doc->GetMetaText("Keywords");
    std::string creator  = doc->GetMetaText("Creator");
    std::string producer = doc->GetMetaText("Producer");
    if (!title.empty())    std::cout << "  Title      : " << title << "\n";
    if (!author.empty())   std::cout << "  Author     : " << author << "\n";
    if (!subject.empty())  std::cout << "  Subject    : " << subject << "\n";
    if (!keywords.empty()) std::cout << "  Keywords   : " << keywords << "\n";
    if (!creator.empty())  std::cout << "  Creator    : " << creator << "\n";
    if (!producer.empty()) std::cout << "  Producer   : " << producer << "\n";

    // 4. Load the first page
    auto page = doc->LoadPage(0);
    if (!page) {
        std::fprintf(stderr, "Failed to load page 0.\n");
        return 1;
    }
    std::cout << "Page 0 loaded.\n";

    // 5. Get page size
    double page_w = page->GetWidth();
    double page_h = page->GetHeight();
    std::cout << "  Page size  : " << page_w << " x " << page_h << " pts\n";

    // 6. Create bitmap and render
    int render_dpi = 150;
    double scale = render_dpi / 72.0;
    int bmp_w = static_cast<int>(page_w * scale);
    int bmp_h = static_cast<int>(page_h * scale);

    pdfium::Bitmap bitmap(bmp_w, bmp_h, 0 /* no alpha */);
    page->Render(bitmap, 0, 0, bmp_w, bmp_h, 0, 0);

    // 7. Save as PPM
    if (SaveBitmapToPPM(bitmap, ppm_path)) {
        std::cout << "Saved: " << ppm_path << " (" << bmp_w << "x" << bmp_h << ")\n";
    } else {
        std::fprintf(stderr, "Failed to save '%s'\n", ppm_path.c_str());
        return 1;
    }

    std::cout << "Done.\n";
    return 0;
}
