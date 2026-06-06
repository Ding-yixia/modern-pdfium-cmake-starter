#pragma once
#include <memory>
#include <vector>
#include "types.h"

namespace pdfium {

class PathSegment {
public:
    bool GetPoint(float* x, float* y) const;
    int GetType() const;
    bool IsClose() const;
private:
    friend class PathObject;
    PathSegment(FPDF_PATHSEGMENT segment) : segment_(segment) {}
    FPDF_PATHSEGMENT segment_;
};

class PathObject {
public:
    static std::unique_ptr<PageObject> CreatePath(float x, float y);
    static std::unique_ptr<PageObject> CreateRect(float x, float y, float w, float h);
    static std::unique_ptr<PageObject> CreateEllipse(float x, float y, float w, float h);

    // Segments
    int CountSegments() const;
    PathSegment GetSegment(int index) const;
    bool MoveTo(float x, float y);
    bool LineTo(float x, float y);
    bool BezierTo(float x1, float y1, float x2, float y2, float x3, float y3);
    bool Close();

    // Draw mode
    bool SetDrawMode(int fill_mode, bool stroke);
    bool GetDrawMode(int* fill_mode, bool* stroke) const;

    // Color
    bool SetFillColor(unsigned int R, unsigned int G, unsigned int B, unsigned int A);
    bool GetFillColor(unsigned int* R, unsigned int* G, unsigned int* B, unsigned int* A) const;
    bool SetStrokeColor(unsigned int R, unsigned int G, unsigned int B, unsigned int A);
    bool GetStrokeColor(unsigned int* R, unsigned int* G, unsigned int* B, unsigned int* A) const;
    
    // Stroke
    bool SetStrokeWidth(float width);
    bool GetStrokeWidth(float* width) const;
    int GetLineJoin() const;
    bool SetLineJoin(int line_join);
    int GetLineCap() const;
    bool SetLineCap(int line_cap);

    // Dash
    bool GetDashPhase(float* phase) const;
    bool SetDashPhase(float phase);
    int GetDashCount() const;
    bool GetDashArray(float* dash_array, size_t dash_count) const;
    bool SetDashArray(const float* dash_array, size_t dash_count, float phase);

    // Bounds
    bool GetBounds(float* left, float* bottom, float* right, float* top) const;

    FPDF_PAGEOBJECT Handle() const { return obj_; }
    explicit operator bool() const { return obj_ != nullptr; }
private:
    FPDF_PAGEOBJECT obj_;
};

} // namespace pdfium
