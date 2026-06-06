#pragma once
#include <memory>
#include <string>
#include "types.h"

namespace pdfium {

class StructElement;

class StructTree {
public:
    ~StructTree();
    StructTree(const StructTree&) = delete;
    StructTree& operator=(const StructTree&) = delete;
    StructTree(StructTree&& other) noexcept;
    StructTree& operator=(StructTree&& other) noexcept;
    int CountChildren() const;
    StructElement GetChildAtIndex(int index) const;
    FPDF_STRUCTTREE Handle() const { return tree_; }
    explicit operator bool() const { return tree_ != nullptr; }
private:
    friend class Page;
    StructTree(FPDF_STRUCTTREE tree);
    FPDF_STRUCTTREE tree_;
};

class StructElement {
public:
    std::wstring GetType() const;
    std::wstring GetTitle() const;
    std::wstring GetAltText() const;
    std::wstring GetActualText() const;
    std::wstring GetID() const;
    std::wstring GetLang() const;
    std::wstring GetExpansion() const;
    std::wstring GetObjType() const;
    int GetMarkedContentID() const;
    int CountChildren() const;
    StructElement GetChildAtIndex(int index) const;
    StructElement GetParent() const;
    int GetAttributeCount() const;
    FPDF_STRUCTELEMENT Handle() const { return elem_; }
    explicit operator bool() const { return elem_ != nullptr; }
private:
    friend class StructTree;
    StructElement(FPDF_STRUCTELEMENT elem);
    FPDF_STRUCTELEMENT elem_;
};

} // namespace pdfium
