#include "pdfium/bookmark.h"
#include "pdfium/document.h"
#include <fpdfview.h>
#include <fpdf_doc.h>
#include <vector>

namespace pdfium {

// ---------------------------------------------------------------------------
// Bookmark
// ---------------------------------------------------------------------------

Bookmark Bookmark::GetFirstChild() const {
    if (!bookmark_ || !doc_) {
        return Bookmark();
    }
    return Bookmark(FPDFBookmark_GetFirstChild(doc_, bookmark_), doc_);
}

Bookmark Bookmark::GetNextSibling() const {
    if (!bookmark_ || !doc_) {
        return Bookmark();
    }
    return Bookmark(FPDFBookmark_GetNextSibling(doc_, bookmark_), doc_);
}

int Bookmark::GetCount() const {
    return FPDFBookmark_GetCount(bookmark_);
}

std::wstring Bookmark::GetTitle() const {
    unsigned long len =
        FPDFBookmark_GetTitle(bookmark_, nullptr, 0);
    if (len <= 0) {
        return {};
    }
    std::vector<unsigned short> buf(len / 2 + 1, 0);
    FPDFBookmark_GetTitle(bookmark_, buf.data(),
                           static_cast<unsigned long>(buf.size() * 2));
    // Remove null terminator
    if (!buf.empty() && buf.back() == 0) {
        buf.pop_back();
    }
    return std::wstring(buf.begin(), buf.end());
}

std::unique_ptr<Dest> Bookmark::GetDest(const Document& doc) const {
    FPDF_DEST dest = FPDFBookmark_GetDest(doc.Handle(), bookmark_);
    if (!dest) {
        return nullptr;
    }
    return std::unique_ptr<Dest>(new Dest(dest));
}

std::unique_ptr<Action> Bookmark::GetAction() const {
    FPDF_ACTION action = FPDFBookmark_GetAction(bookmark_);
    if (!action) {
        return nullptr;
    }
    return std::unique_ptr<Action>(new Action(action));
}

// ---------------------------------------------------------------------------
// Dest
// ---------------------------------------------------------------------------

int Dest::GetPageIndex(const Document& doc) const {
    return FPDFDest_GetDestPageIndex(doc.Handle(), dest_);
}

unsigned long Dest::GetView(unsigned long* num_params, FS_FLOAT* params) const {
    return FPDFDest_GetView(dest_, num_params, params);
}

bool Dest::GetLocationInPage(bool* has_x, bool* has_y, bool* has_zoom,
                              FS_FLOAT* x, FS_FLOAT* y,
                              FS_FLOAT* zoom) const {
    FPDF_BOOL b_has_x = 0, b_has_y = 0, b_has_zoom = 0;
    bool result = FPDFDest_GetLocationInPage(dest_, &b_has_x, &b_has_y,
                                              &b_has_zoom, x, y, zoom) != 0;
    if (has_x) *has_x = b_has_x != 0;
    if (has_y) *has_y = b_has_y != 0;
    if (has_zoom) *has_zoom = b_has_zoom != 0;
    return result;
}

// ---------------------------------------------------------------------------
// Action
// ---------------------------------------------------------------------------

unsigned long Action::GetType() const {
    return FPDFAction_GetType(action_);
}

std::unique_ptr<Dest> Action::GetDest(const Document& doc) const {
    FPDF_DEST dest = FPDFAction_GetDest(doc.Handle(), action_);
    if (!dest) {
        return nullptr;
    }
    return std::unique_ptr<Dest>(new Dest(dest));
}

std::string Action::GetFilePath() const {
    unsigned long len = FPDFAction_GetFilePath(action_, nullptr, 0);
    if (len <= 0) {
        return {};
    }
    std::vector<char> buf(len, '\0');
    FPDFAction_GetFilePath(action_, buf.data(),
                            static_cast<unsigned long>(buf.size()));
    return std::string(buf.data());
}

std::string Action::GetURIPath(const Document& doc) const {
    unsigned long len =
        FPDFAction_GetURIPath(doc.Handle(), action_, nullptr, 0);
    if (len <= 0) {
        return {};
    }
    std::vector<char> buf(len, '\0');
    FPDFAction_GetURIPath(doc.Handle(), action_, buf.data(),
                           static_cast<unsigned long>(buf.size()));
    return std::string(buf.data());
}

} // namespace pdfium
