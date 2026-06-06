#include "pdfium/page.h"
#include "pdfium/text.h"
#include "pdfium/document.h"
#include "pdfium/bitmap.h"
#include "pdfium/dataavail.h"
#include "pdfium/link.h"

#include <vector>
#include <cstring>

namespace pdfium {

Page::Page(FPDF_PAGE page) : page_(page) {}

Page::~Page() {
    if (page_) {
        FPDF_ClosePage(page_);
    }
}

Page::Page(Page&& other) noexcept : page_(other.page_) {
    other.page_ = nullptr;
}

Page& Page::operator=(Page&& other) noexcept {
    if (this != &other) {
        if (page_) {
            FPDF_ClosePage(page_);
        }
        page_ = other.page_;
        other.page_ = nullptr;
    }
    return *this;
}

double Page::GetWidth() const {
    return FPDF_GetPageWidth(page_);
}

double Page::GetHeight() const {
    return FPDF_GetPageHeight(page_);
}

bool Page::GetBoundingBox(FS_RECTF* rect) const {
    return FPDF_GetPageBoundingBox(page_, rect) != 0;
}

bool Page::GetSizeByIndex(const Document& doc, int index, double* width,
                           double* height) {
    return FPDF_GetPageSizeByIndex(doc.Handle(), index, width, height) != 0;
}

bool Page::GetSizeByIndexF(const Document& doc, int index, FS_SIZEF* size) {
    return FPDF_GetPageSizeByIndexF(doc.Handle(), index, size) != 0;
}

int Page::GetRotation() const {
    return FPDFPage_GetRotation(page_);
}

void Page::SetRotation(int rotate) {
    FPDFPage_SetRotation(page_, rotate);
}

void Page::Render(Bitmap& bitmap, int start_x, int start_y, int size_x,
                   int size_y, int rotate, int flags) const {
    FPDF_RenderPageBitmap(bitmap.Handle(), page_, start_x, start_y, size_x,
                          size_y, rotate, flags);
}

void Page::RenderWithMatrix(Bitmap& bitmap, const FS_MATRIX& matrix,
                             const FS_RECTF* clipping, int flags) const {
    FPDF_RenderPageBitmapWithMatrix(bitmap.Handle(), page_, &matrix, clipping,
                                    flags);
}

bool Page::GenerateContent() {
    return FPDFPage_GenerateContent(page_) != 0;
}

bool Page::HasTransparency() const {
    return FPDFPage_HasTransparency(page_) != 0;
}

int Page::CountObjects() const {
    return FPDFPage_CountObjects(page_);
}

std::unique_ptr<PageObject> Page::GetObject(int index) const {
    FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page_, index);
    if (!obj) {
        return nullptr;
    }
    return std::unique_ptr<PageObject>(new PageObject(obj));
}

int Page::GetAnnotCount() const {
    return FPDFPage_GetAnnotCount(page_);
}

std::unique_ptr<TextPage> Page::LoadTextPage() {
    FPDF_TEXTPAGE text_page = FPDFText_LoadPage(page_);
    if (!text_page) {
        return nullptr;
    }
    return std::unique_ptr<TextPage>(new TextPage(text_page));
}

std::vector<uint8_t> Page::GetDecodedThumbnailData() const {
    unsigned long len = FPDFPage_GetDecodedThumbnailData(page_, nullptr, 0);
    if (len == 0) {
        return {};
    }
    std::vector<uint8_t> buf(len);
    FPDFPage_GetDecodedThumbnailData(page_, buf.data(),
                                      static_cast<unsigned long>(buf.size()));
    return buf;
}

std::vector<uint8_t> Page::GetRawThumbnailData() const {
    unsigned long len = FPDFPage_GetRawThumbnailData(page_, nullptr, 0);
    if (len == 0) {
        return {};
    }
    std::vector<uint8_t> buf(len);
    FPDFPage_GetRawThumbnailData(page_, buf.data(),
                                  static_cast<unsigned long>(buf.size()));
    return buf;
}

std::unique_ptr<Bitmap> Page::GetThumbnailAsBitmap() {
    FPDF_BITMAP bmp = FPDFPage_GetThumbnailAsBitmap(page_);
    if (!bmp) {
        return nullptr;
    }
    int w = FPDFBitmap_GetWidth(bmp);
    int h = FPDFBitmap_GetHeight(bmp);
    int fmt = FPDFBitmap_GetFormat(bmp);
    void* src_buf = FPDFBitmap_GetBuffer(bmp);
    int stride = FPDFBitmap_GetStride(bmp);

    auto result = std::make_unique<Bitmap>(w, h, fmt, nullptr, stride);
    if (src_buf && result->GetBuffer()) {
        std::memcpy(result->GetBuffer(), src_buf,
                     static_cast<size_t>(h * stride));
    }
    return result;
}

bool Page::GetMediaBox(float* left, float* bottom, float* right,
                        float* top) const {
    return FPDFPage_GetMediaBox(page_, left, bottom, right, top) != 0;
}

void Page::SetMediaBox(float left, float bottom, float right, float top) {
    FPDFPage_SetMediaBox(page_, left, bottom, right, top);
}

bool Page::GetCropBox(float* left, float* bottom, float* right,
                       float* top) const {
    return FPDFPage_GetCropBox(page_, left, bottom, right, top) != 0;
}

void Page::SetCropBox(float left, float bottom, float right, float top) {
    FPDFPage_SetCropBox(page_, left, bottom, right, top);
}

bool Page::GetBleedBox(float* left, float* bottom, float* right,
                        float* top) const {
    return FPDFPage_GetBleedBox(page_, left, bottom, right, top) != 0;
}

void Page::SetBleedBox(float left, float bottom, float right, float top) {
    FPDFPage_SetBleedBox(page_, left, bottom, right, top);
}

bool Page::GetTrimBox(float* left, float* bottom, float* right,
                       float* top) const {
    return FPDFPage_GetTrimBox(page_, left, bottom, right, top) != 0;
}

void Page::SetTrimBox(float left, float bottom, float right, float top) {
    FPDFPage_SetTrimBox(page_, left, bottom, right, top);
}

bool Page::GetArtBox(float* left, float* bottom, float* right,
                      float* top) const {
    return FPDFPage_GetArtBox(page_, left, bottom, right, top) != 0;
}

void Page::SetArtBox(float left, float bottom, float right, float top) {
    FPDFPage_SetArtBox(page_, left, bottom, right, top);
}

void Page::TransformWithClip(const FS_MATRIX* matrix,
                              const FS_RECTF* clip_rect) {
    FPDFPage_TransFormWithClip(page_, matrix, clip_rect);
}

void Page::TransformAnnots(double a, double b, double c, double d, double e,
                            double f) {
    FPDFPage_TransformAnnots(page_, a, b, c, d, e, f);
}

void Page::InsertClipPath(FPDF_CLIPPATH clip_path) {
    FPDFPage_InsertClipPath(page_, clip_path);
}

void Page::DeviceToPage(int start_x, int start_y, int size_x, int size_y,
                         int rotate, int device_x, int device_y, double* page_x,
                         double* page_y) const {
    FPDF_DeviceToPage(page_, start_x, start_y, size_x, size_y, rotate, device_x,
                      device_y, page_x, page_y);
}

void Page::PageToDevice(int start_x, int start_y, int size_x, int size_y,
                         int rotate, double page_x, double page_y,
                         int* device_x, int* device_y) const {
    FPDF_PageToDevice(page_, start_x, start_y, size_x, size_y, rotate, page_x,
                      page_y, device_x, device_y);
}

int Page::Flatten(int flags) {
    return FPDFPage_Flatten(page_, flags);
}

Link Page::GetLinkAtPoint(double x, double y) {
    FPDF_LINK link = FPDFLink_GetLinkAtPoint(page_, x, y);
    return Link(link);
}

int Page::GetLinkZOrderAtPoint(double x, double y) {
    return FPDFLink_GetLinkZOrderAtPoint(page_, x, y);
}

bool Page::LinkEnum(int* start_pos, FPDF_LINK* link_annot) {
    return FPDFLink_Enumerate(page_, start_pos, link_annot) != 0;
}

} // namespace pdfium
