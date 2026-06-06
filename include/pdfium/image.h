#pragma once
#include <memory>
#include <vector>
#include <cstdint>
#include "types.h"

namespace pdfium {

struct ImageMetadata {
    int width = 0;
    int height = 0;
    float horizontal_dpi = 0.0f;
    float vertical_dpi = 0.0f;
    int bits_per_pixel = 0;
    int colorspace = 0;
    int marked_content_id = -1;
};

class ImageObject {
public:
    explicit ImageObject(FPDF_PAGEOBJECT obj);
    ~ImageObject() = default;

    // Metadata
    ImageMetadata GetMetadata(const Page& page) const;
    bool GetPixelSize(unsigned int* width, unsigned int* height) const;
    std::unique_ptr<Bitmap> GetBitmap() const;
    std::unique_ptr<Bitmap> GetRenderedBitmap(const Document& doc, const Page& page) const;

    // Image data
    std::vector<uint8_t> GetImageDataDecoded() const;
    std::vector<uint8_t> GetImageDataRaw() const;
    int GetFilterCount() const;
    std::string GetFilter(int index) const;

    // ICC profile
    std::vector<uint8_t> GetIccProfileDataDecoded(const Page& page) const;

    // Matrix
    bool SetMatrix(double a, double b, double c, double d, double e, double f);

    // Load from file
    bool LoadJpegFile(const Page* pages, int count, FPDF_FILEACCESS* file_access);
    bool LoadJpegFileInline(const Page* pages, int count, FPDF_FILEACCESS* file_access);
    bool SetBitmap(const Page* pages, int count, const Bitmap& bitmap);

    FPDF_PAGEOBJECT Handle() const { return obj_; }
    explicit operator bool() const { return obj_ != nullptr; }

private:
    FPDF_PAGEOBJECT obj_;
};

} // namespace pdfium
