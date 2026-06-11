#include "pdfium/text.h"
#include <fpdfview.h>
#include <fpdf_text.h>
#include <vector>

namespace pdfium {

// ---------------------------------------------------------------------------
// TextPage
// ---------------------------------------------------------------------------

TextPage::TextPage(FPDF_TEXTPAGE text_page) : text_page_(text_page) {}

TextPage::~TextPage() {
    if (text_page_) {
        FPDFText_ClosePage(text_page_);
    }
}

TextPage::TextPage(TextPage&& other) noexcept : text_page_(other.text_page_) {
    other.text_page_ = nullptr;
}

TextPage& TextPage::operator=(TextPage&& other) noexcept {
    if (this != &other) {
        if (text_page_) {
            FPDFText_ClosePage(text_page_);
        }
        text_page_ = other.text_page_;
        other.text_page_ = nullptr;
    }
    return *this;
}

int TextPage::CharCount() const {
    return FPDFText_CountChars(text_page_);
}

unsigned int TextPage::GetUnicode(int index) const {
    return FPDFText_GetUnicode(text_page_, index);
}

double TextPage::GetFontSize(int index) const {
    return FPDFText_GetFontSize(text_page_, index);
}

int TextPage::GetCharIndexAtPos(double x, double y, double x_tol,
                                 double y_tol) const {
    return FPDFText_GetCharIndexAtPos(text_page_, x, y, x_tol, y_tol);
}

bool TextPage::GetCharBox(int index, double* left, double* right,
                           double* bottom, double* top) const {
    return FPDFText_GetCharBox(text_page_, index, left, right, bottom,
                                top) != 0;
}

bool TextPage::GetCharOrigin(int index, double* x, double* y) const {
    return FPDFText_GetCharOrigin(text_page_, index, x, y) != 0;
}

std::wstring TextPage::GetText(int start, int count) const {
    if (count <= 0) {
        return {};
    }
    std::vector<unsigned short> buf(static_cast<size_t>(count) + 1);
    int written = FPDFText_GetText(text_page_, start, count, buf.data());
    if (written <= 0) {
        return {};
    }
    buf.resize(static_cast<size_t>(written));
    // Remove null terminator
    if (!buf.empty() && buf.back() == 0) {
        buf.pop_back();
    }
    return std::wstring(buf.begin(), buf.end());
}

std::wstring TextPage::GetBoundedText(double left, double top, double right,
                                       double bottom) const {
    int buf_len = FPDFText_GetBoundedText(text_page_, left, top, right, bottom,
                                           nullptr, 0);
    if (buf_len <= 0) {
        return {};
    }
    std::vector<unsigned short> buf(static_cast<size_t>(buf_len));
    FPDFText_GetBoundedText(text_page_, left, top, right, bottom, buf.data(),
                             buf_len);
    // Remove null terminator
    if (!buf.empty() && buf.back() == 0) {
        buf.pop_back();
    }
    return std::wstring(buf.begin(), buf.end());
}

int TextPage::CountRects(int start, int count) const {
    return FPDFText_CountRects(text_page_, start, count);
}

bool TextPage::GetRect(int rect_index, double* left, double* top,
                        double* right, double* bottom) const {
    return FPDFText_GetRect(text_page_, rect_index, left, top, right,
                             bottom) != 0;
}

// ---------------------------------------------------------------------------
// TextFind
// ---------------------------------------------------------------------------

TextFind::TextFind(FPDF_SCHHANDLE handle) : handle_(handle) {}

TextFind::~TextFind() {
    if (handle_) {
        FPDFText_FindClose(handle_);
    }
}

bool TextFind::FindNext() {
    return FPDFText_FindNext(handle_) != 0;
}

bool TextFind::FindPrev() {
    return FPDFText_FindPrev(handle_) != 0;
}

int TextFind::GetResultIndex() const {
    return FPDFText_GetSchResultIndex(handle_);
}

int TextFind::GetResultCount() const {
    return FPDFText_GetSchCount(handle_);
}

// ---------------------------------------------------------------------------
// WebLinks
// ---------------------------------------------------------------------------

WebLinks::WebLinks(FPDF_PAGELINK page_link) : page_link_(page_link) {}

WebLinks::~WebLinks() {
    if (page_link_) {
        FPDFLink_CloseWebLinks(page_link_);
    }
}

WebLinks::WebLinks(WebLinks&& other) noexcept
    : page_link_(other.page_link_) {
    other.page_link_ = nullptr;
}

WebLinks& WebLinks::operator=(WebLinks&& other) noexcept {
    if (this != &other) {
        if (page_link_) {
            FPDFLink_CloseWebLinks(page_link_);
        }
        page_link_ = other.page_link_;
        other.page_link_ = nullptr;
    }
    return *this;
}

int WebLinks::Count() const {
    return FPDFLink_CountWebLinks(page_link_);
}

std::wstring WebLinks::GetURL(int index) const {
    int buf_len = FPDFLink_GetURL(page_link_, index, nullptr, 0);
    if (buf_len <= 0) {
        return {};
    }
    std::vector<unsigned short> buf(static_cast<size_t>(buf_len));
    FPDFLink_GetURL(page_link_, index, buf.data(), buf_len);
    // Remove null terminator
    if (!buf.empty() && buf.back() == 0) {
        buf.pop_back();
    }
    return std::wstring(buf.begin(), buf.end());
}

int WebLinks::CountRects(int link_index) const {
    return FPDFLink_CountRects(page_link_, link_index);
}

bool WebLinks::GetRect(int link_index, int rect_index, double* left,
                        double* top, double* right, double* bottom) const {
    return FPDFLink_GetRect(page_link_, link_index, rect_index, left, top,
                             right, bottom) != 0;
}

} // namespace pdfium
