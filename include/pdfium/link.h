#pragma once
#include <memory>
#include "types.h"

namespace pdfium {

class Link {
public:
    std::unique_ptr<Dest> GetDest(const Document& doc) const;
    std::unique_ptr<Action> GetAction() const;
    bool GetAnnotRect(FS_RECTF* rect) const;
    int CountQuadPoints() const;
    bool GetQuadPoints(int quad_index, FS_QUADPOINTSF* quad_points) const;
    FPDF_LINK Handle() const { return link_; }
    explicit operator bool() const { return link_ != nullptr; }
private:
    friend class Page;
    Link(FPDF_LINK link);
    FPDF_LINK link_;
};

} // namespace pdfium
