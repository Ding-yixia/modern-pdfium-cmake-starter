#include "pdfium/dataavail.h"
#include "pdfium/document.h"
#include <fpdfview.h>
#include <fpdf_dataavail.h>
#include <fpdf_edit.h>

namespace pdfium {

// ---------------------------------------------------------------------------
// Avail
// ---------------------------------------------------------------------------

Avail::Avail(FX_FILEAVAIL* file_avail, FPDF_FILEACCESS* file)
    : avail_(FPDFAvail_Create(file_avail, file)) {}

Avail::~Avail() {
    if (avail_) {
        FPDFAvail_Destroy(avail_);
    }
}

Avail::Avail(Avail&& other) noexcept : avail_(other.avail_) {
    other.avail_ = nullptr;
}

Avail& Avail::operator=(Avail&& other) noexcept {
    if (this != &other) {
        if (avail_) {
            FPDFAvail_Destroy(avail_);
        }
        avail_ = other.avail_;
        other.avail_ = nullptr;
    }
    return *this;
}

bool Avail::IsDocAvail() {
    return FPDFAvail_IsDocAvail(avail_, nullptr) != 0;
}

std::unique_ptr<Document> Avail::GetDocument(const std::string& password) {
    FPDF_DOCUMENT doc = FPDFAvail_GetDocument(avail_, password.c_str());
    if (!doc) {
        return nullptr;
    }
    return std::unique_ptr<Document>(new Document(doc));
}

int Avail::GetFirstPageNum(FPDF_DOCUMENT doc) const {
    return FPDFAvail_GetFirstPageNum(doc);
}

bool Avail::IsPageAvail(int page_index) {
    return FPDFAvail_IsPageAvail(avail_, page_index, nullptr) != 0;
}

bool Avail::IsLinearized() const {
    return FPDFAvail_IsLinearized(avail_) != 0;
}

// ---------------------------------------------------------------------------
// PageObject
// ---------------------------------------------------------------------------

PageObject::PageObject(FPDF_PAGEOBJECT obj) : obj_(obj) {
    // PageObject does NOT own the handle by default; the page or annotation
    // that created it retains ownership.
}

PageObject::~PageObject() {
    // Not owned by default; do not free.
}

PageObject::PageObject(PageObject&& other) noexcept : obj_(other.obj_) {
    other.obj_ = nullptr;
}

PageObject& PageObject::operator=(PageObject&& other) noexcept {
    if (this != &other) {
        obj_ = other.obj_;
        other.obj_ = nullptr;
    }
    return *this;
}

int PageObject::GetType() const {
    return FPDFPageObj_GetType(obj_);
}

bool PageObject::HasTransparency() const {
    return FPDFPageObj_HasTransparency(obj_) != 0;
}

bool PageObject::GetBounds(float* left, float* bottom, float* right,
                            float* top) const {
    return FPDFPageObj_GetBounds(obj_, left, bottom, right, top) != 0;
}

void PageObject::Transform(double a, double b, double c, double d, double e,
                            double f) {
    FPDFPageObj_Transform(obj_, a, b, c, d, e, f);
}

bool PageObject::SetMatrix(const FS_MATRIX& matrix) {
    return FPDFPageObj_SetMatrix(obj_, &matrix) != 0;
}

bool PageObject::GetMatrix(FS_MATRIX* matrix) const {
    return FPDFPageObj_GetMatrix(obj_, matrix) != 0;
}

bool PageObject::GetIsActive() const {
    FPDF_BOOL active;
    return FPDFPageObj_GetIsActive(obj_, &active) != 0 && active != 0;
}

bool PageObject::SetIsActive(bool active) {
    return FPDFPageObj_SetIsActive(obj_, active ? 1 : 0) != 0;
}

bool PageObject::SetFillColor(unsigned int R, unsigned int G, unsigned int B,
                               unsigned int A) {
    return FPDFPageObj_SetFillColor(obj_, R, G, B, A) != 0;
}

bool PageObject::GetFillColor(unsigned int* R, unsigned int* G,
                               unsigned int* B, unsigned int* A) const {
    return FPDFPageObj_GetFillColor(obj_, R, G, B, A) != 0;
}

bool PageObject::SetStrokeColor(unsigned int R, unsigned int G, unsigned int B,
                                 unsigned int A) {
    return FPDFPageObj_SetStrokeColor(obj_, R, G, B, A) != 0;
}

bool PageObject::GetStrokeColor(unsigned int* R, unsigned int* G,
                                 unsigned int* B, unsigned int* A) const {
    return FPDFPageObj_GetStrokeColor(obj_, R, G, B, A) != 0;
}

bool PageObject::SetStrokeWidth(float width) {
    return FPDFPageObj_SetStrokeWidth(obj_, width) != 0;
}

bool PageObject::GetStrokeWidth(float* width) const {
    return FPDFPageObj_GetStrokeWidth(obj_, width) != 0;
}

int PageObject::GetLineJoin() const {
    return FPDFPageObj_GetLineJoin(obj_);
}

bool PageObject::SetLineJoin(int line_join) {
    return FPDFPageObj_SetLineJoin(obj_, line_join) != 0;
}

int PageObject::GetLineCap() const {
    return FPDFPageObj_GetLineCap(obj_);
}

bool PageObject::SetLineCap(int line_cap) {
    return FPDFPageObj_SetLineCap(obj_, line_cap) != 0;
}

void PageObject::SetBlendMode(const std::string& blend_mode) {
    FPDFPageObj_SetBlendMode(obj_, blend_mode.c_str());
}

} // namespace pdfium
