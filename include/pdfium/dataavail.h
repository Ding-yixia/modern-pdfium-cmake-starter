#pragma once
#include <memory>
#include "types.h"

namespace pdfium {

class Avail {
public:
    Avail(FX_FILEAVAIL* file_avail, FPDF_FILEACCESS* file);
    ~Avail();
    Avail(const Avail&) = delete;
    Avail& operator=(const Avail&) = delete;
    Avail(Avail&& other) noexcept;
    Avail& operator=(Avail&& other) noexcept;
    bool IsDocAvail();
    std::unique_ptr<Document> GetDocument(const std::string& password = "");
    int GetFirstPageNum() const;
    bool IsPageAvail(int page_index);
    bool IsLinearized() const;
    FPDF_AVAIL Handle() const { return avail_; }
    explicit operator bool() const { return avail_ != nullptr; }
private:
    FPDF_AVAIL avail_;
};

class PageObject {
public:
    ~PageObject();
    PageObject(const PageObject&) = delete;
    PageObject& operator=(const PageObject&) = delete;
    PageObject(PageObject&& other) noexcept;
    PageObject& operator=(PageObject&& other) noexcept;
    int GetType() const;
    bool HasTransparency() const;
    bool GetBounds(float* left, float* bottom, float* right, float* top) const;
    void Transform(double a, double b, double c, double d, double e, double f);
    bool SetMatrix(const FS_MATRIX& matrix);
    bool GetMatrix(FS_MATRIX* matrix) const;
    bool GetIsActive() const;
    bool SetIsActive(bool active);
    // Color
    bool SetFillColor(unsigned int R, unsigned int G, unsigned int B, unsigned int A);
    bool GetFillColor(unsigned int* R, unsigned int* G, unsigned int* B, unsigned int* A) const;
    bool SetStrokeColor(unsigned int R, unsigned int G, unsigned int B, unsigned int A);
    bool GetStrokeColor(unsigned int* R, unsigned int* G, unsigned int* B, unsigned int* A) const;
    bool SetStrokeWidth(float width);
    bool GetStrokeWidth(float* width) const;
    int GetLineJoin() const;
    bool SetLineJoin(int line_join);
    int GetLineCap() const;
    bool SetLineCap(int line_cap);
    void SetBlendMode(const std::string& blend_mode);
    FPDF_PAGEOBJECT Handle() const { return obj_; }
    explicit operator bool() const { return obj_ != nullptr; }
private:
    friend class Page;
    friend class Annotation;
    friend class PathObject;
    PageObject(FPDF_PAGEOBJECT obj);
    FPDF_PAGEOBJECT obj_;
};

} // namespace pdfium
