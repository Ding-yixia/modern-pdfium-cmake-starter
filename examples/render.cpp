#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>

#include "pdfium/pdfium.h"

// ---------------------------------------------------------------------------
// Helper: Save pdfium::Bitmap → PPM
// ---------------------------------------------------------------------------
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
            fputc(row[x*3+2], f);
            fputc(row[x*3+1], f);
            fputc(row[x*3+0], f);
        }
    }
    fclose(f);
    return true;
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::fprintf(stderr, "Usage: pdfium_render <pdf-file>\n");
        return 1;
    }

    // 1. Initialize and load PDF
    pdfium::Library lib;
    auto doc = pdfium::Document::LoadFromFile(argv[1]);
    if (!doc) {
        std::fprintf(stderr, "Failed to load '%s'\n", argv[1]);
        return 1;
    }

    int page_count = doc->GetPageCount();
    std::cout << "Loaded: " << argv[1] << "  Pages: " << page_count << "\n";

    // 2. Render each page at multiple scales
    const double scales[] = {0.5, 1.0, 2.0};   // 36, 72, 144 DPI equivalents
    const int num_scales = sizeof(scales) / sizeof(scales[0]);

    auto t_start = std::chrono::steady_clock::now();

    for (int p = 0; p < page_count; ++p) {
        auto page = doc->LoadPage(p);
        if (!page) {
            std::fprintf(stderr, "  [page %d] failed to load, skipped.\n", p);
            continue;
        }

        double pw = page->GetWidth();
        double ph = page->GetHeight();
        std::cout << "\n--- Page " << p << "  (" << pw << " x " << ph << " pts) ---\n";

        // Render at full page size (scale = 1.0) and save
        for (int s = 0; s < num_scales; ++s) {
            double scale = scales[s];
            int bmp_w = static_cast<int>(pw * scale + 0.5);
            int bmp_h = static_cast<int>(ph * scale + 0.5);

            auto t_render_start = std::chrono::steady_clock::now();

            pdfium::Bitmap bitmap(bmp_w, bmp_h, 0);
            page->Render(bitmap, 0, 0, bmp_w, bmp_h, 0, 0);

            auto t_render_end = std::chrono::steady_clock::now();
            auto render_us = std::chrono::duration_cast<std::chrono::microseconds>(
                t_render_end - t_render_start).count();

            // Save only the scale=1.0 render to a PPM file
            if (p == 0 && s == 0) {
                // Also save thumbnail separately below, so skip here
            }

            std::cout << "  Scale=" << scale
                      << "  " << bmp_w << "x" << bmp_h
                      << "  render " << render_us << " us\n";
        }

        // 3. Render a thumbnail (small scale) and save
        {
            double thumb_scale = 0.2;  // ~14 DPI at 72 DPI base
            int thumb_w = static_cast<int>(pw * thumb_scale + 0.5);
            int thumb_h = static_cast<int>(ph * thumb_scale + 0.5);
            if (thumb_w < 1) thumb_w = 1;
            if (thumb_h < 1) thumb_h = 1;

            pdfium::Bitmap thumb(thumb_w, thumb_h, 0);
            page->Render(thumb, 0, 0, thumb_w, thumb_h, 0, 0);

            char ppm_name[64];
            std::snprintf(ppm_name, sizeof(ppm_name), "page_%d.ppm", p);
            if (SaveBitmapToPPM(thumb, ppm_name)) {
                std::cout << "  Saved thumbnail: " << ppm_name
                          << " (" << thumb_w << "x" << thumb_h << ")\n";
            } else {
                std::fprintf(stderr, "  Failed to save '%s'\n", ppm_name);
            }
        }
    }

    auto t_end = std::chrono::steady_clock::now();
    auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        t_end - t_start).count();

    // 4. Print timing info
    std::cout << "\n=== Render summary ===\n";
    std::cout << "Total time: " << total_ms << " ms for " << page_count << " page(s)\n";
    std::cout << "Average   : " << (page_count > 0 ? total_ms / page_count : 0)
              << " ms/page\n";

    std::cout << "\nDone.\n";
    return 0;
}
