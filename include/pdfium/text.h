#pragma once
#include <memory>
#include <string>
#include <vector>
#include "types.h"

namespace pdfium {

class Page;
class Document;
class Bitmap;

class TextPage {
public:
    ~TextPage();
    TextPage(const TextPage&) = delete;
    TextPage& operator=(const TextPage&) = delete;
    TextPage(TextPage&& other) noexcept;
    TextPage& operator=(TextPage&& other) noexcept;

    int CharCount() const;
    unsigned int GetUnicode(int index) const;
    double GetFontSize(int index) const;
    int GetCharIndexAtPos(double x, double y, double x_tol = 0.0, double y_tol = 0.0) const;

    bool GetCharBox(int index, double* left, double* right, double* bottom, double* top) const;
    bool GetCharOrigin(int index, double* x, double* y) const;

    std::wstring GetText(int start, int count) const;
    std::wstring GetBoundedText(double left, double top, double right, double bottom) const;

    int CountRects(int start, int count) const;
    bool GetRect(int rect_index, double* left, double* top, double* right, double* bottom) const;

    FPDF_TEXTPAGE Handle() const { return text_page_; }
    explicit operator bool() const { return text_page_ != nullptr; }

private:
    friend class Page;
    explicit TextPage(FPDF_TEXTPAGE text_page);
    FPDF_TEXTPAGE text_page_;
};

class TextFind {
public:
    ~TextFind();
    TextFind(const TextFind&) = delete;
    TextFind& operator=(const TextFind&) = delete;

    bool FindNext();
    bool FindPrev();
    int GetResultIndex() const;
    int GetResultCount() const;

    FPDF_SCHHANDLE Handle() const { return handle_; }
    explicit operator bool() const { return handle_ != nullptr; }

private:
    friend class TextPage;
    TextFind(FPDF_SCHHANDLE handle);
    FPDF_SCHHANDLE handle_;
};

class WebLinks {
public:
    ~WebLinks();
    WebLinks(const WebLinks&) = delete;
    WebLinks& operator=(const WebLinks&) = delete;
    WebLinks(WebLinks&& other) noexcept;
    WebLinks& operator=(WebLinks&& other) noexcept;

    int Count() const;
    std::wstring GetURL(int index) const;
    int CountRects(int link_index) const;
    bool GetRect(int link_index, int rect_index, double* left, double* top, double* right, double* bottom) const;

    FPDF_PAGELINK Handle() const { return page_link_; }
    explicit operator bool() const { return page_link_ != nullptr; }

private:
    friend class TextPage;
    WebLinks(FPDF_PAGELINK page_link);
    FPDF_PAGELINK page_link_;
};

} // namespace pdfium
