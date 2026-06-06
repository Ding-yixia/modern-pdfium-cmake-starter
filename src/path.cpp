#include "pdfium/path.h"
#include <fpdfview.h>
#include <fpdf_edit.h>

namespace pdfium {

// ---------------------------------------------------------------------------
// PathSegment
// ---------------------------------------------------------------------------

bool PathSegment::GetPoint(float* x, float* y) const {
    return FPDFPathSegment_GetPoint(segment_, x, y) != 0;
}

int PathSegment::GetType() const {
    return FPDFPathSegment_GetType(segment_);
}

bool PathSegment::IsClose() const {
    return FPDFPathSegment_GetClose(segment_) != 0;
}

// ---------------------------------------------------------------------------
// PathObject
// ---------------------------------------------------------------------------

std::unique_ptr<PageObject> PathObject::CreatePath(float x, float y) {
    FPDF_PAGEOBJECT obj = FPDFPageObj_CreateNewPath(x, y);
    if (!obj) {
        return nullptr;
    }
    // PageObject is forward-declared; the actual wrapper is not yet implemented.
    // Clean up to avoid leaking the underlying object.
    FPDFPageObj_Destroy(obj);
    return nullptr;
}

std::unique_ptr<PageObject> PathObject::CreateRect(float x, float y, float w, float h) {
    FPDF_PAGEOBJECT obj = FPDFPageObj_CreateNewRect(x, y, w, h);
    if (!obj) {
        return nullptr;
    }
    FPDFPageObj_Destroy(obj);
    return nullptr;
}

std::unique_ptr<PageObject> PathObject::CreateEllipse(float x, float y, float w, float h) {
    // Build an ellipse centered at (x + w/2, y + h/2) with radius w/2, h/2.
    // Use 4 Bezier cubic segments with the standard circle approximation.
    float rx = w / 2.0f;
    float ry = h / 2.0f;
    float cx = x + rx;
    float cy = y + ry;
    // Bezier approximation constant: 4/3 * tan(pi/8)
    const float k = 0.5522847498f;
    float kx = rx * k;
    float ky = ry * k;

    FPDF_PAGEOBJECT obj = FPDFPageObj_CreateNewPath(cx, cy + ry);
    if (!obj) {
        return nullptr;
    }

    // Top-right quadrant
    FPDFPath_BezierTo(obj, cx + kx, cy + ry, cx + rx, cy + ky, cx + rx, cy);
    // Bottom-right quadrant
    FPDFPath_BezierTo(obj, cx + rx, cy - ky, cx + kx, cy - ry, cx, cy - ry);
    // Bottom-left quadrant
    FPDFPath_BezierTo(obj, cx - kx, cy - ry, cx - rx, cy - ky, cx - rx, cy);
    // Top-left quadrant
    FPDFPath_BezierTo(obj, cx - rx, cy + ky, cx - kx, cy + ry, cx, cy + ry);

    FPDFPath_Close(obj);

    // PageObject wrapper not yet implemented; clean up.
    FPDFPageObj_Destroy(obj);
    return nullptr;
}

int PathObject::CountSegments() const {
    return FPDFPath_CountSegments(obj_);
}

PathSegment PathObject::GetSegment(int index) const {
    FPDF_PATHSEGMENT seg = FPDFPath_GetPathSegment(obj_, index);
    return PathSegment(seg);
}

bool PathObject::MoveTo(float x, float y) {
    return FPDFPath_MoveTo(obj_, x, y) != 0;
}

bool PathObject::LineTo(float x, float y) {
    return FPDFPath_LineTo(obj_, x, y) != 0;
}

bool PathObject::BezierTo(float x1, float y1, float x2, float y2, float x3, float y3) {
    return FPDFPath_BezierTo(obj_, x1, y1, x2, y2, x3, y3) != 0;
}

bool PathObject::Close() {
    return FPDFPath_Close(obj_) != 0;
}

bool PathObject::SetDrawMode(int fill_mode, bool stroke) {
    return FPDFPath_SetDrawMode(obj_, fill_mode, stroke ? 1 : 0) != 0;
}

bool PathObject::GetDrawMode(int* fill_mode, bool* stroke) const {
    FPDF_BOOL stroke_val = 0;
    if (!FPDFPath_GetDrawMode(obj_, fill_mode, &stroke_val)) {
        return false;
    }
    *stroke = stroke_val != 0;
    return true;
}

bool PathObject::SetFillColor(unsigned int R, unsigned int G, unsigned int B, unsigned int A) {
    return FPDFPageObj_SetFillColor(obj_, R, G, B, A) != 0;
}

bool PathObject::GetFillColor(unsigned int* R, unsigned int* G, unsigned int* B, unsigned int* A) const {
    return FPDFPageObj_GetFillColor(obj_, R, G, B, A) != 0;
}

bool PathObject::SetStrokeColor(unsigned int R, unsigned int G, unsigned int B, unsigned int A) {
    return FPDFPageObj_SetStrokeColor(obj_, R, G, B, A) != 0;
}

bool PathObject::GetStrokeColor(unsigned int* R, unsigned int* G, unsigned int* B, unsigned int* A) const {
    return FPDFPageObj_GetStrokeColor(obj_, R, G, B, A) != 0;
}

bool PathObject::SetStrokeWidth(float width) {
    return FPDFPageObj_SetStrokeWidth(obj_, width) != 0;
}

bool PathObject::GetStrokeWidth(float* width) const {
    return FPDFPageObj_GetStrokeWidth(obj_, width) != 0;
}

int PathObject::GetLineJoin() const {
    return FPDFPageObj_GetLineJoin(obj_);
}

bool PathObject::SetLineJoin(int line_join) {
    return FPDFPageObj_SetLineJoin(obj_, line_join) != 0;
}

int PathObject::GetLineCap() const {
    return FPDFPageObj_GetLineCap(obj_);
}

bool PathObject::SetLineCap(int line_cap) {
    return FPDFPageObj_SetLineCap(obj_, line_cap) != 0;
}

bool PathObject::GetDashPhase(float* phase) const {
    return FPDFPageObj_GetDashPhase(obj_, phase) != 0;
}

bool PathObject::SetDashPhase(float phase) {
    return FPDFPageObj_SetDashPhase(obj_, phase) != 0;
}

int PathObject::GetDashCount() const {
    return FPDFPageObj_GetDashCount(obj_);
}

bool PathObject::GetDashArray(float* dash_array, size_t dash_count) const {
    return FPDFPageObj_GetDashArray(obj_, dash_array, dash_count) != 0;
}

bool PathObject::SetDashArray(const float* dash_array, size_t dash_count, float phase) {
    return FPDFPageObj_SetDashArray(obj_, dash_array, dash_count, phase) != 0;
}

bool PathObject::GetBounds(float* left, float* bottom, float* right, float* top) const {
    return FPDFPageObj_GetBounds(obj_, left, bottom, right, top) != 0;
}

} // namespace pdfium
