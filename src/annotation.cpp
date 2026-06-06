#include "pdfium/annotation.h"
#include "pdfium/dataavail.h"
#include <fpdfview.h>
#include <fpdf_annot.h>
#include <vector>

namespace pdfium {

// ---------------------------------------------------------------------------
// Annotation
// ---------------------------------------------------------------------------

Annotation::Annotation(FPDF_ANNOTATION annot) : annot_(annot) {}

Annotation::~Annotation() {
    if (annot_) {
        FPDFPage_CloseAnnot(annot_);
    }
}

Annotation::Annotation(Annotation&& other) noexcept : annot_(other.annot_) {
    other.annot_ = nullptr;
}

Annotation& Annotation::operator=(Annotation&& other) noexcept {
    if (this != &other) {
        if (annot_) {
            FPDFPage_CloseAnnot(annot_);
        }
        annot_ = other.annot_;
        other.annot_ = nullptr;
    }
    return *this;
}

AnnotationSubtype Annotation::GetSubtype() const {
    return static_cast<AnnotationSubtype>(FPDFAnnot_GetSubtype(annot_));
}

FS_RECTF Annotation::GetRect() const {
    FS_RECTF rect{};
    FPDFAnnot_GetRect(annot_, &rect);
    return rect;
}

void Annotation::SetRect(const FS_RECTF& rect) {
    FPDFAnnot_SetRect(annot_, &rect);
}

int Annotation::GetFlags() const {
    return FPDFAnnot_GetFlags(annot_);
}

void Annotation::SetFlags(int flags) {
    FPDFAnnot_SetFlags(annot_, flags);
}

bool Annotation::SetColor(unsigned int R, unsigned int G, unsigned int B,
                           unsigned int A) {
    return FPDFAnnot_SetColor(annot_, FPDFANNOT_COLORTYPE_Color, R, G, B,
                               A) != 0;
}

bool Annotation::GetColor(unsigned int* R, unsigned int* G, unsigned int* B,
                           unsigned int* A) {
    return FPDFAnnot_GetColor(annot_, FPDFANNOT_COLORTYPE_Color, R, G, B,
                               A) != 0;
}

bool Annotation::SetBorder(float h_radius, float v_radius, float border_width) {
    return FPDFAnnot_SetBorder(annot_, h_radius, v_radius, border_width) != 0;
}

bool Annotation::GetBorder(float* h_radius, float* v_radius,
                            float* border_width) {
    return FPDFAnnot_GetBorder(annot_, h_radius, v_radius, border_width) != 0;
}

std::wstring Annotation::GetStringValue(const std::string& key) const {
    unsigned long len =
        FPDFAnnot_GetStringValue(annot_, key.c_str(), nullptr, 0);
    if (len <= 0) {
        return {};
    }
    // len is in bytes, includes null terminator (2 bytes for UTF-16LE)
    std::vector<unsigned short> buf(len / 2 + 1, 0);
    FPDFAnnot_GetStringValue(annot_, key.c_str(), buf.data(), len);
    // Remove null terminator
    if (!buf.empty() && buf.back() == 0) {
        buf.pop_back();
    }
    return std::wstring(buf.begin(), buf.end());
}

bool Annotation::SetStringValue(const std::string& key,
                                 const std::wstring& value) {
    auto wide = reinterpret_cast<FPDF_WIDESTRING>(value.c_str());
    return FPDFAnnot_SetStringValue(annot_, key.c_str(), wide) != 0;
}

bool Annotation::HasKey(const std::string& key) const {
    return FPDFAnnot_HasKey(annot_, key.c_str()) != 0;
}

int Annotation::GetObjectCount() const {
    return FPDFAnnot_GetObjectCount(annot_);
}

std::unique_ptr<PageObject> Annotation::GetObject(int index) const {
    FPDF_PAGEOBJECT obj = FPDFAnnot_GetObject(annot_, index);
    if (!obj) {
        return nullptr;
    }
    return std::unique_ptr<PageObject>(new PageObject(obj));
}

bool Annotation::RemoveObject(int index) {
    return FPDFAnnot_RemoveObject(annot_, index) != 0;
}

bool Annotation::AppendObject(PageObject* obj) {
    if (!obj) {
        return false;
    }
    return FPDFAnnot_AppendObject(annot_, obj->Handle()) != 0;
}

size_t Annotation::CountAttachmentPoints() const {
    return FPDFAnnot_CountAttachmentPoints(annot_);
}

bool Annotation::GetAttachmentPoints(size_t quad_index,
                                      FS_QUADPOINTSF* quad_points) const {
    return FPDFAnnot_GetAttachmentPoints(annot_, quad_index, quad_points) != 0;
}

} // namespace pdfium
