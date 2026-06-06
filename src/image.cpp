#include "pdfium/image.h"
#include "pdfium/page.h"
#include "pdfium/document.h"
#include "pdfium/bitmap.h"
#include <fpdfview.h>
#include <fpdf_edit.h>
#include <cstring>
#include <vector>

namespace pdfium {

ImageObject::ImageObject(FPDF_PAGEOBJECT obj) : obj_(obj) {}

ImageMetadata ImageObject::GetMetadata(const Page& page) const {
    ImageMetadata meta;
    FPDF_IMAGEOBJ_METADATA pdfium_meta{};
    if (FPDFImageObj_GetImageMetadata(obj_, page.Handle(), &pdfium_meta)) {
        meta.width = static_cast<int>(pdfium_meta.width);
        meta.height = static_cast<int>(pdfium_meta.height);
        meta.horizontal_dpi = pdfium_meta.horizontal_dpi;
        meta.vertical_dpi = pdfium_meta.vertical_dpi;
        meta.bits_per_pixel = static_cast<int>(pdfium_meta.bits_per_pixel);
        meta.colorspace = pdfium_meta.colorspace;
        meta.marked_content_id = pdfium_meta.marked_content_id;
    }
    return meta;
}

bool ImageObject::GetPixelSize(unsigned int* width, unsigned int* height) const {
    return FPDFImageObj_GetImagePixelSize(obj_, width, height) != 0;
}

std::unique_ptr<Bitmap> ImageObject::GetBitmap() const {
    FPDF_BITMAP bmp = FPDFImageObj_GetBitmap(obj_);
    if (!bmp) {
        return nullptr;
    }
    int w = FPDFBitmap_GetWidth(bmp);
    int h = FPDFBitmap_GetHeight(bmp);
    int fmt = FPDFBitmap_GetFormat(bmp);
    int stride = FPDFBitmap_GetStride(bmp);
    void* src_buf = FPDFBitmap_GetBuffer(bmp);

    // FPDFImageObj_GetBitmap returns a new bitmap owned by the caller.
    // Transfer ownership into our Bitmap wrapper which will FPDFBitmap_Destroy it.
    auto result = std::make_unique<Bitmap>(w, h, fmt, nullptr, stride);
    if (src_buf && result->GetBuffer()) {
        std::memcpy(result->GetBuffer(), src_buf,
                     static_cast<size_t>(h) * stride);
    }
    FPDFBitmap_Destroy(bmp);
    return result;
}

std::unique_ptr<Bitmap> ImageObject::GetRenderedBitmap(const Document& doc, const Page& page) const {
    FPDF_BITMAP bmp = FPDFImageObj_GetRenderedBitmap(doc.Handle(), page.Handle(), obj_);
    if (!bmp) {
        return nullptr;
    }
    int w = FPDFBitmap_GetWidth(bmp);
    int h = FPDFBitmap_GetHeight(bmp);
    int fmt = FPDFBitmap_GetFormat(bmp);
    int stride = FPDFBitmap_GetStride(bmp);
    void* src_buf = FPDFBitmap_GetBuffer(bmp);

    auto result = std::make_unique<Bitmap>(w, h, fmt, nullptr, stride);
    if (src_buf && result->GetBuffer()) {
        std::memcpy(result->GetBuffer(), src_buf,
                     static_cast<size_t>(h) * stride);
    }
    FPDFBitmap_Destroy(bmp);
    return result;
}

std::vector<uint8_t> ImageObject::GetImageDataDecoded() const {
    unsigned long len = FPDFImageObj_GetImageDataDecoded(obj_, nullptr, 0);
    if (len == 0) {
        return {};
    }
    std::vector<uint8_t> buf(len);
    FPDFImageObj_GetImageDataDecoded(obj_, buf.data(),
                                      static_cast<unsigned long>(buf.size()));
    return buf;
}

std::vector<uint8_t> ImageObject::GetImageDataRaw() const {
    unsigned long len = FPDFImageObj_GetImageDataRaw(obj_, nullptr, 0);
    if (len == 0) {
        return {};
    }
    std::vector<uint8_t> buf(len);
    FPDFImageObj_GetImageDataRaw(obj_, buf.data(),
                                  static_cast<unsigned long>(buf.size()));
    return buf;
}

int ImageObject::GetFilterCount() const {
    return FPDFImageObj_GetImageFilterCount(obj_);
}

std::string ImageObject::GetFilter(int index) const {
    unsigned long len = FPDFImageObj_GetImageFilter(obj_, index, nullptr, 0);
    if (len == 0) {
        return {};
    }
    std::vector<char> buf(len, '\0');
    FPDFImageObj_GetImageFilter(obj_, index, buf.data(),
                                 static_cast<unsigned long>(buf.size()));
    return std::string(buf.data());
}

std::vector<uint8_t> ImageObject::GetIccProfileDataDecoded(const Page& page) const {
    size_t out_len = 0;
    if (!FPDFImageObj_GetIccProfileDataDecoded(obj_, page.Handle(), nullptr, 0, &out_len)) {
        return {};
    }
    std::vector<uint8_t> buf(out_len);
    if (!FPDFImageObj_GetIccProfileDataDecoded(obj_, page.Handle(), buf.data(),
                                                buf.size(), &out_len)) {
        return {};
    }
    buf.resize(out_len);
    return buf;
}

bool ImageObject::SetMatrix(double a, double b, double c, double d, double e, double f) {
    return FPDFImageObj_SetMatrix(obj_, a, b, c, d, e, f) != 0;
}

bool ImageObject::LoadJpegFile(const Page* pages, int count, FPDF_FILEACCESS* file_access) {
    // Build an array of FPDF_PAGE handles from Page objects
    std::vector<FPDF_PAGE> page_handles;
    if (pages && count > 0) {
        page_handles.reserve(count);
        for (int i = 0; i < count; ++i) {
            page_handles.push_back(pages[i].Handle());
        }
    }
    FPDF_PAGE* page_ptr = page_handles.empty() ? nullptr : page_handles.data();
    return FPDFImageObj_LoadJpegFile(page_ptr, count, obj_, file_access) != 0;
}

bool ImageObject::LoadJpegFileInline(const Page* pages, int count, FPDF_FILEACCESS* file_access) {
    std::vector<FPDF_PAGE> page_handles;
    if (pages && count > 0) {
        page_handles.reserve(count);
        for (int i = 0; i < count; ++i) {
            page_handles.push_back(pages[i].Handle());
        }
    }
    FPDF_PAGE* page_ptr = page_handles.empty() ? nullptr : page_handles.data();
    return FPDFImageObj_LoadJpegFileInline(page_ptr, count, obj_, file_access) != 0;
}

bool ImageObject::SetBitmap(const Page* pages, int count, const Bitmap& bitmap) {
    std::vector<FPDF_PAGE> page_handles;
    if (pages && count > 0) {
        page_handles.reserve(count);
        for (int i = 0; i < count; ++i) {
            page_handles.push_back(pages[i].Handle());
        }
    }
    FPDF_PAGE* page_ptr = page_handles.empty() ? nullptr : page_handles.data();
    return FPDFImageObj_SetBitmap(page_ptr, count, obj_, bitmap.Handle()) != 0;
}

} // namespace pdfium
