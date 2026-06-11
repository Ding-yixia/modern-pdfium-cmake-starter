#pragma once
#include <memory>
#include <string>
#include <vector>
#include "types.h"

namespace pdfium {

enum class AnnotationSubtype {
    Unknown = 0, Text = 1, Link = 2, FreeText = 3, Line = 4, Square = 5,
    Circle = 6, Polygon = 7, Polyline = 8, Highlight = 9, Underline = 10,
    Squiggly = 11, StrikeOut = 12, Stamp = 13, Caret = 14, Ink = 15,
    Popup = 16, FileAttachment = 17, Sound = 18, Movie = 19, Widget = 20,
    Screen = 21, PrinterMark = 22, TrapNet = 23, Watermark = 24,
    ThreeD = 25, RichMedia = 26, XFAWidget = 27, Redact = 28
};

class Annotation {
public:
    ~Annotation();
    Annotation(const Annotation&) = delete;
    Annotation& operator=(const Annotation&) = delete;
    Annotation(Annotation&& other) noexcept;
    Annotation& operator=(Annotation&& other) noexcept;

    // Properties
    AnnotationSubtype GetSubtype() const;
    FS_RECTF GetRect() const;
    void SetRect(const FS_RECTF& rect);
    int GetFlags() const;
    void SetFlags(int flags);

    // Color
    bool SetColor(unsigned int R, unsigned int G, unsigned int B, unsigned int A);
    bool GetColor(unsigned int* R, unsigned int* G, unsigned int* B, unsigned int* A);

    // Border
    bool SetBorder(float h_radius, float v_radius, float border_width);
    bool GetBorder(float* h_radius, float* v_radius, float* border_width);

    // Contents (string value for "Contents" key)
    std::wstring GetStringValue(const std::string& key) const;
    bool SetStringValue(const std::string& key, const std::wstring& value);
    bool HasKey(const std::string& key) const;

    // Object operations
    int GetObjectCount() const;
    std::unique_ptr<PageObject> GetObject(int index) const;
    bool RemoveObject(int index);
    bool AppendObject(PageObject* obj);

    // Attachment points (for text markup annotations)
    size_t CountAttachmentPoints() const;
    bool GetAttachmentPoints(size_t quad_index, FS_QUADPOINTSF* quad_points) const;

    FPDF_ANNOTATION Handle() const { return annot_; }
    explicit operator bool() const { return annot_ != nullptr; }

    explicit Annotation(FPDF_ANNOTATION annot);
    FPDF_ANNOTATION annot_;
};

} // namespace pdfium
