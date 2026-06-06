#include "pdfium/struct_tree.h"
#include <fpdfview.h>
#include <fpdf_structtree.h>
#include <vector>

namespace pdfium {

namespace {

// Helper: convert an FPDF_WCHAR buffer returned by a Pdfium getter into a
// std::wstring.  The getter is called with a nullptr first to retrieve the
// required buffer length (in bytes), then a second time to fill the buffer.
std::wstring GetStructText(unsigned long (*getter)(FPDF_STRUCTELEMENT, FPDF_WCHAR*, unsigned long),
                           FPDF_STRUCTELEMENT elem) {
    unsigned long len = getter(elem, nullptr, 0);
    if (len <= 0) {
        return {};
    }
    std::vector<FPDF_WCHAR> buf(len / sizeof(FPDF_WCHAR) + 1, 0);
    unsigned long actual = getter(elem, buf.data(),
                                   static_cast<unsigned long>(buf.size() * sizeof(FPDF_WCHAR)));
    if (actual <= 0) {
        return {};
    }
    // Remove trailing null terminator
    if (!buf.empty() && buf.back() == 0) {
        buf.pop_back();
    }
    return std::wstring(buf.begin(), buf.end());
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// StructTree
// ---------------------------------------------------------------------------

StructTree::StructTree(FPDF_STRUCTTREE tree) : tree_(tree) {}

StructTree::~StructTree() {
    if (tree_) {
        FPDF_StructTree_Close(tree_);
    }
}

StructTree::StructTree(StructTree&& other) noexcept : tree_(other.tree_) {
    other.tree_ = nullptr;
}

StructTree& StructTree::operator=(StructTree&& other) noexcept {
    if (this != &other) {
        if (tree_) {
            FPDF_StructTree_Close(tree_);
        }
        tree_ = other.tree_;
        other.tree_ = nullptr;
    }
    return *this;
}

int StructTree::CountChildren() const {
    return FPDF_StructTree_CountChildren(tree_);
}

StructElement StructTree::GetChildAtIndex(int index) const {
    FPDF_STRUCTELEMENT elem = FPDF_StructTree_GetChildAtIndex(tree_, index);
    return StructElement(elem);
}

// ---------------------------------------------------------------------------
// StructElement
// ---------------------------------------------------------------------------

StructElement::StructElement(FPDF_STRUCTELEMENT elem) : elem_(elem) {}

std::wstring StructElement::GetType() const {
    return GetStructText(FPDF_StructElement_GetType, elem_);
}

std::wstring StructElement::GetTitle() const {
    return GetStructText(FPDF_StructElement_GetTitle, elem_);
}

std::wstring StructElement::GetAltText() const {
    return GetStructText(FPDF_StructElement_GetAltText, elem_);
}

std::wstring StructElement::GetActualText() const {
    return GetStructText(FPDF_StructElement_GetActualText, elem_);
}

std::wstring StructElement::GetID() const {
    return GetStructText(FPDF_StructElement_GetID, elem_);
}

std::wstring StructElement::GetLang() const {
    return GetStructText(FPDF_StructElement_GetLang, elem_);
}

std::wstring StructElement::GetExpansion() const {
    return GetStructText(FPDF_StructElement_GetExpansion, elem_);
}

std::wstring StructElement::GetObjType() const {
    unsigned long len = FPDF_StructElement_GetObjType(elem_, nullptr, 0);
    if (len <= 0) {
        return {};
    }
    std::vector<FPDF_WCHAR> buf(len / sizeof(FPDF_WCHAR) + 1, 0);
    unsigned long actual = FPDF_StructElement_GetObjType(
        elem_, buf.data(), static_cast<unsigned long>(buf.size() * sizeof(FPDF_WCHAR)));
    if (actual <= 0) {
        return {};
    }
    if (!buf.empty() && buf.back() == 0) {
        buf.pop_back();
    }
    return std::wstring(buf.begin(), buf.end());
}

int StructElement::GetMarkedContentID() const {
    return FPDF_StructElement_GetMarkedContentID(elem_);
}

int StructElement::CountChildren() const {
    return FPDF_StructElement_CountChildren(elem_);
}

StructElement StructElement::GetChildAtIndex(int index) const {
    FPDF_STRUCTELEMENT child = FPDF_StructElement_GetChildAtIndex(elem_, index);
    return StructElement(child);
}

StructElement StructElement::GetParent() const {
    FPDF_STRUCTELEMENT parent = FPDF_StructElement_GetParent(elem_);
    return StructElement(parent);
}

int StructElement::GetAttributeCount() const {
    return FPDF_StructElement_GetAttributeCount(elem_);
}

} // namespace pdfium
