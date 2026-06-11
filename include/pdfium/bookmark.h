#pragma once
#include <memory>
#include <string>
#include "types.h"

namespace pdfium {

class Dest;
class Action;

class Bookmark {
public:
    Bookmark() = default;

    // Navigation
    Bookmark GetFirstChild() const;
    Bookmark GetNextSibling() const;
    int GetCount() const;

    // Title
    std::wstring GetTitle() const;

    // Destination
    std::unique_ptr<Dest> GetDest(const Document& doc) const;

    // Action
    std::unique_ptr<Action> GetAction() const;

    FPDF_BOOKMARK Handle() const { return bookmark_; }
    explicit operator bool() const { return bookmark_ != nullptr; }

private:
    friend class Document;
    explicit Bookmark(FPDF_BOOKMARK bookmark, FPDF_DOCUMENT doc)
        : bookmark_(bookmark), doc_(doc) {}
    explicit Bookmark(FPDF_BOOKMARK bookmark) : bookmark_(bookmark) {}
    FPDF_BOOKMARK bookmark_ = nullptr;
    FPDF_DOCUMENT doc_ = nullptr;
};

class Dest {
public:
    int GetPageIndex(const Document& doc) const;
    unsigned long GetView(unsigned long* num_params, FS_FLOAT* params) const;
    bool GetLocationInPage(bool* has_x, bool* has_y, bool* has_zoom,
                           FS_FLOAT* x, FS_FLOAT* y, FS_FLOAT* zoom) const;
    FPDF_DEST Handle() const { return dest_; }
    explicit operator bool() const { return dest_ != nullptr; }
private:
    friend class Bookmark;
    friend class Action;
    friend class Link;
    friend class Document;
    explicit Dest(FPDF_DEST dest) : dest_(dest) {}
    FPDF_DEST dest_ = nullptr;
};

class Action {
public:
    unsigned long GetType() const;
    std::unique_ptr<Dest> GetDest(const Document& doc) const;
    std::string GetFilePath() const;
    std::string GetURIPath(const Document& doc) const;
    FPDF_ACTION Handle() const { return action_; }
    explicit operator bool() const { return action_ != nullptr; }
private:
    friend class Bookmark;
    friend class Link;
    friend class Page;
    explicit Action(FPDF_ACTION action) : action_(action) {}
    FPDF_ACTION action_ = nullptr;
};

} // namespace pdfium
