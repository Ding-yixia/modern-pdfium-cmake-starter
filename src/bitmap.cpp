#include "pdfium/bitmap.h"

namespace pdfium {

Bitmap::Bitmap(int width, int height, int alpha)
    : bitmap_(FPDFBitmap_Create(width, height, alpha)),
      owns_buffer_(true) {
}

Bitmap::Bitmap(int width, int height, int format, void* first_scan, int stride)
    : bitmap_(FPDFBitmap_CreateEx(width, height, format, first_scan, stride)),
      owns_buffer_(first_scan != nullptr) {
}

Bitmap::~Bitmap() {
    if (bitmap_) {
        FPDFBitmap_Destroy(bitmap_);
    }
}

Bitmap::Bitmap(Bitmap&& other) noexcept
    : bitmap_(other.bitmap_), owns_buffer_(other.owns_buffer_) {
    other.bitmap_ = nullptr;
    other.owns_buffer_ = false;
}

Bitmap& Bitmap::operator=(Bitmap&& other) noexcept {
    if (this != &other) {
        if (bitmap_) {
            FPDFBitmap_Destroy(bitmap_);
        }
        bitmap_ = other.bitmap_;
        owns_buffer_ = other.owns_buffer_;
        other.bitmap_ = nullptr;
        other.owns_buffer_ = false;
    }
    return *this;
}

int Bitmap::GetWidth() const {
    return FPDFBitmap_GetWidth(bitmap_);
}

int Bitmap::GetHeight() const {
    return FPDFBitmap_GetHeight(bitmap_);
}

int Bitmap::GetStride() const {
    return FPDFBitmap_GetStride(bitmap_);
}

int Bitmap::GetFormat() const {
    return FPDFBitmap_GetFormat(bitmap_);
}

void* Bitmap::GetBuffer() const {
    return FPDFBitmap_GetBuffer(bitmap_);
}

void Bitmap::FillRect(int left, int top, int width, int height,
                       uint32_t color) {
    FPDFBitmap_FillRect(bitmap_, left, top, width, height, color);
}

} // namespace pdfium
