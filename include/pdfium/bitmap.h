#pragma once
#include <cstdint>
#include <vector>
#include "types.h"

namespace pdfium {

class Bitmap {
public:
    static constexpr int kUnknown = 0;
    static constexpr int kGray = 1;
    static constexpr int kBGR = 2;
    static constexpr int kBGRx = 3;
    static constexpr int kBGRA = 4;
    static constexpr int kBGRA_Premul = 5;

    // Create a new bitmap
    Bitmap(int width, int height, int alpha = 1);
    Bitmap(int width, int height, int format, void* first_scan, int stride);
    ~Bitmap();
    Bitmap(const Bitmap&) = delete;
    Bitmap& operator=(const Bitmap&) = delete;
    Bitmap(Bitmap&& other) noexcept;
    Bitmap& operator=(Bitmap&& other) noexcept;

    // Properties
    int GetWidth() const;
    int GetHeight() const;
    int GetStride() const;
    int GetFormat() const;
    void* GetBuffer() const;

    // Operations
    void FillRect(int left, int top, int width, int height, uint32_t color);
    
    // Raw handle
    FPDF_BITMAP Handle() const { return bitmap_; }
    explicit operator bool() const { return bitmap_ != nullptr; }

private:
    FPDF_BITMAP bitmap_;
    bool owns_buffer_;
};

} // namespace pdfium
