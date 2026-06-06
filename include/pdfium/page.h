#pragma once
#include <memory>
#include <vector>
#include "types.h"

namespace pdfium {

// Forward declarations
class Document;
class TextPage;
class Link;

class Page {
public:
    ~Page();
    Page(const Page&) = delete;
    Page& operator=(const Page&) = delete;
    Page(Page&& other) noexcept;
    Page& operator=(Page&& other) noexcept;

    // Dimensions
    double GetWidth() const;
    double GetHeight() const;
    bool GetBoundingBox(FS_RECTF* rect) const;
    static bool GetSizeByIndex(const Document& doc, int index, double* width, double* height);
    static bool GetSizeByIndexF(const Document& doc, int index, FS_SIZEF* size);

    // Rotation
    int GetRotation() const;
    void SetRotation(int rotate);
    
    // Rendering to bitmap
    void Render(Bitmap& bitmap, int start_x, int start_y, int size_x, int size_y, 
                int rotate = 0, int flags = 0) const;
    void RenderWithMatrix(Bitmap& bitmap, const FS_MATRIX& matrix, 
                          const FS_RECTF* clipping = nullptr, int flags = 0) const;

    // Content generation
    bool GenerateContent();
    
    // Transparency
    bool HasTransparency() const;

    // Page objects
    int CountObjects() const;
    std::unique_ptr<PageObject> GetObject(int index) const;
    
    // Annotations
    int GetAnnotCount() const;

    // Text
    std::unique_ptr<TextPage> LoadTextPage();

    // Thumbnail
    std::vector<uint8_t> GetDecodedThumbnailData() const;
    std::vector<uint8_t> GetRawThumbnailData() const;
    std::unique_ptr<Bitmap> GetThumbnailAsBitmap();

    // Media/Crop/Trim/Bleed/Art box
    bool GetMediaBox(float* left, float* bottom, float* right, float* top) const;
    void SetMediaBox(float left, float bottom, float right, float top);
    bool GetCropBox(float* left, float* bottom, float* right, float* top) const;
    void SetCropBox(float left, float bottom, float right, float top);
    bool GetBleedBox(float* left, float* bottom, float* right, float* top) const;
    void SetBleedBox(float left, float bottom, float right, float top);
    bool GetTrimBox(float* left, float* bottom, float* right, float* top) const;
    void SetTrimBox(float left, float bottom, float right, float top);
    bool GetArtBox(float* left, float* bottom, float* right, float* top) const;
    void SetArtBox(float left, float bottom, float right, float top);
    
    // Transform
    void TransformWithClip(const FS_MATRIX* matrix, const FS_RECTF* clip_rect);
    void TransformAnnots(double a, double b, double c, double d, double e, double f);
    
    // Clip path
    void InsertClipPath(FPDF_CLIPPATH clip_path);
    
    // Coordinate conversion
    void DeviceToPage(int start_x, int start_y, int size_x, int size_y, int rotate,
                      int device_x, int device_y, double* page_x, double* page_y) const;
    void PageToDevice(int start_x, int start_y, int size_x, int size_y, int rotate,
                      double page_x, double page_y, int* device_x, int* device_y) const;
                      
    // Flatten annotations
    int Flatten(int flags = 0);

    // Links
    Link GetLinkAtPoint(double x, double y);
    int GetLinkZOrderAtPoint(double x, double y);
    bool LinkEnum(int* start_pos, FPDF_LINK* link_annot);

    // Raw handle
    FPDF_PAGE Handle() const { return page_; }
    explicit operator bool() const { return page_ != nullptr; }

private:
    friend class Document;
    Page(FPDF_PAGE page);
    FPDF_PAGE page_;
};

} // namespace pdfium
