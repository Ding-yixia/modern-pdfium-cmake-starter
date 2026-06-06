#include "pdfium/font.h"
#include "pdfium/document.h"
#include <fpdfview.h>
#include <fpdf_edit.h>
#include <vector>
#include <cstring>

namespace pdfium {

Font::Font(FPDF_FONT font) : font_(font) {}

Font::~Font() {
    if (font_) {
        FPDFFont_Close(font_);
    }
}

Font::Font(Font&& other) noexcept : font_(other.font_) {
    other.font_ = nullptr;
}

Font& Font::operator=(Font&& other) noexcept {
    if (this != &other) {
        if (font_) {
            FPDFFont_Close(font_);
        }
        font_ = other.font_;
        other.font_ = nullptr;
    }
    return *this;
}

std::unique_ptr<Font> Font::LoadFromData(const Document& doc, const uint8_t* data, 
                                          uint32_t size, int font_type, bool cid) {
    FPDF_FONT font = FPDFText_LoadFont(doc.Handle(), data, size, font_type, 
                                        cid ? 1 : 0);
    if (!font) {
        return nullptr;
    }
    return std::unique_ptr<Font>(new Font(font));
}

std::unique_ptr<Font> Font::LoadStandard(const Document& doc, const std::string& font_name) {
    FPDF_FONT font = FPDFText_LoadStandardFont(doc.Handle(), font_name.c_str());
    if (!font) {
        return nullptr;
    }
    return std::unique_ptr<Font>(new Font(font));
}

std::string Font::GetBaseFontName() const {
    size_t len = FPDFFont_GetBaseFontName(font_, nullptr, 0);
    if (len == 0) {
        return {};
    }
    std::vector<char> buf(len, '\0');
    FPDFFont_GetBaseFontName(font_, buf.data(), len);
    // The returned length includes the trailing NUL; trim it.
    std::string result(buf.data());
    return result;
}

std::string Font::GetFamilyName() const {
    size_t len = FPDFFont_GetFamilyName(font_, nullptr, 0);
    if (len == 0) {
        return {};
    }
    std::vector<char> buf(len, '\0');
    FPDFFont_GetFamilyName(font_, buf.data(), len);
    std::string result(buf.data());
    return result;
}

std::vector<uint8_t> Font::GetFontData() const {
    size_t out_len = 0;
    if (!FPDFFont_GetFontData(font_, nullptr, 0, &out_len)) {
        return {};
    }
    std::vector<uint8_t> buf(out_len);
    if (!FPDFFont_GetFontData(font_, buf.data(), buf.size(), &out_len)) {
        return {};
    }
    buf.resize(out_len);
    return buf;
}

bool Font::IsEmbedded() const {
    int result = FPDFFont_GetIsEmbedded(font_);
    return result == 1;
}

int Font::GetFlags() const {
    return FPDFFont_GetFlags(font_);
}

int Font::GetWeight() const {
    return FPDFFont_GetWeight(font_);
}

bool Font::GetItalicAngle(int* angle) const {
    return FPDFFont_GetItalicAngle(font_, angle) != 0;
}

bool Font::GetAscent(float font_size, float* ascent) const {
    return FPDFFont_GetAscent(font_, font_size, ascent) != 0;
}

bool Font::GetDescent(float font_size, float* descent) const {
    return FPDFFont_GetDescent(font_, font_size, descent) != 0;
}

bool Font::GetGlyphWidth(uint32_t glyph, float font_size, float* width) const {
    return FPDFFont_GetGlyphWidth(font_, glyph, font_size, width) != 0;
}

} // namespace pdfium
