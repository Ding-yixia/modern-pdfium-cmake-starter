#include "pdfium/link.h"
#include "pdfium/document.h"
#include "pdfium/bookmark.h"
#include <fpdfview.h>
#include <fpdf_doc.h>

namespace pdfium {

// ---------------------------------------------------------------------------
// Link
// ---------------------------------------------------------------------------

Link::Link(FPDF_LINK link) : link_(link) {}

std::unique_ptr<Dest> Link::GetDest(const Document& doc) const {
    FPDF_DEST dest = FPDFLink_GetDest(doc.Handle(), link_);
    if (!dest) {
        return nullptr;
    }
    return std::unique_ptr<Dest>(new Dest(dest));
}

std::unique_ptr<Action> Link::GetAction() const {
    FPDF_ACTION action = FPDFLink_GetAction(link_);
    if (!action) {
        return nullptr;
    }
    return std::unique_ptr<Action>(new Action(action));
}

bool Link::GetAnnotRect(FS_RECTF* rect) const {
    return FPDFLink_GetAnnotRect(link_, rect) != 0;
}

int Link::CountQuadPoints() const {
    return FPDFLink_CountQuadPoints(link_);
}

bool Link::GetQuadPoints(int quad_index, FS_QUADPOINTSF* quad_points) const {
    return FPDFLink_GetQuadPoints(link_, quad_index, quad_points) != 0;
}

} // namespace pdfium
