#pragma once
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include "types.h"

namespace pdfium {

class Font {
public:
    ~Font();
    Font(const Font&) = delete;
    Font& operator=(const Font&) = delete;
    Font(Font&& other) noexcept;
    Font& operator=(Font&& other) noexcept;

    // Load font from data
    static std::unique_ptr<Font> LoadFromData(const Document& doc, const uint8_t* data, 
                                               uint32_t size, int font_type, bool cid = false);
    static std::unique_ptr<Font> LoadStandard(const Document& doc, const std::string& font_name);

    // Query
    std::string GetBaseFontName() const;
    std::string GetFamilyName() const;
    std::vector<uint8_t> GetFontData() const;
    bool IsEmbedded() const;
    int GetFlags() const;
    int GetWeight() const;
    bool GetItalicAngle(int* angle) const;
    bool GetAscent(float font_size, float* ascent) const;
    bool GetDescent(float font_size, float* descent) const;
    bool GetGlyphWidth(uint32_t glyph, float font_size, float* width) const;

    FPDF_FONT Handle() const { return font_; }
    explicit operator bool() const { return font_ != nullptr; }

private:
    Font(FPDF_FONT font);
    FPDF_FONT font_;
};

} // namespace pdfium
