# modern-pdfium-cmake-starter 使用指南

## 目录

1. [快速开始](#快速开始)
2. [API 参考](#api-参考)
3. [示例详解](#示例详解)
4. [构建配置](#构建配置)
5. [高级用法](#高级用法)

---

## 快速开始

### 前置条件

```bash
# 克隆两个仓库
git clone https://github.com/Ding-yixia/pdfium_cmake_support.git
git clone https://github.com/Ding-yixia/modern-pdfium-cmake-starter.git

# 先构建 pdfium_cmake_support（含示例程序）
cd pdfium_cmake_support
python build.py --examples

# 回到封装库项目
cd ../modern-pdfium-cmake-starter
```

### 构建封装库 + 示例

```bash
mkdir build && cd build

cmake .. \
  -DPDFIUM_ROOT=../pdfium_cmake_support \
  -DBUILD_EXAMPLES=ON \
  -G Ninja

cmake --build . --parallel

# 运行示例
./examples/pdfium_basic ../pdfium_cmake_support/test.pdf
./examples/pdfium_text ../pdfium_cmake_support/test.pdf
```

---

## API 参考

### 1. 核心模块 (`core.h`)

```cpp
namespace pdfium {

// ── 错误码 ──
enum class Error {
    Success = 0,   // 无错误
    Unknown = 1,   // 未知错误
    File = 2,      // 文件未找到
    Format = 3,    // PDF 格式错误
    Password = 4,  // 密码错误
    Security = 5,  // 安全机制阻止
    Page = 6,      // 页面不存在
};

// ── 库初始化（RAII）──
class Library {
public:
    Library();                              // 初始化 PDFium
    explicit Library(const Config& config); // 自定义配置初始化
    ~Library();                             // 自动销毁

    static Error GetLastError();  // 获取最后错误
    static void SetPrintMode(PrintMode mode); // 设置打印模式
};

// ── 使用方式 ──
int main() {
    pdfium::Library lib;  // 构造时初始化
    // ... 使用 PDFium API ...
    return 0;             // 析构时自动销毁
}
```

### 2. 文档操作 (`document.h`)

```cpp
class Document {
public:
    // ── 加载文档（工厂方法）──
    static std::unique_ptr<Document> LoadFromFile(
        const std::string& path,
        const std::string& password = "");

    static std::unique_ptr<Document> LoadFromMemory(
        const void* data, size_t size,
        const std::string& password = "");

    static std::unique_ptr<Document> Create();

    // ── 页面操作 ──
    int  GetPageCount() const;
    std::unique_ptr<Page> LoadPage(int index);
    void DeletePage(int index);
    std::unique_ptr<Page> NewPage(int index, double width, double height);

    // ── 文档信息 ──
    int GetFileVersion() const;
    unsigned long GetPermissions() const;
    std::string GetMetaText(const std::string& tag) const;

    // ── 书签 ──
    Bookmark GetFirstChild(const Bookmark* bookmark = nullptr) const;
    Bookmark FindBookmark(const std::wstring& title) const;

    // ── 保存 ──
    bool SaveAsCopy(const std::string& path, bool incremental = false);
    bool SaveWithVersion(const std::string& path, int version);

    // ── 导入页面 ──
    bool ImportPages(const Document& src, const std::string& range = "", int index = -1);

    // ── 附件 / 签名 / JavaScript ──
    int GetAttachmentCount() const;
    int GetSignatureCount() const;
    int GetJavaScriptActionCount() const;

    // ── 原始句柄 ──
    FPDF_DOCUMENT Handle() const;
    explicit operator bool() const;
};
```

### 3. 页面操作 (`page.h`)

```cpp
class Page {
public:
    // ── 尺寸 ──
    double GetWidth() const;
    double GetHeight() const;

    // ── 渲染 ──
    void Render(Bitmap& bitmap, int x, int y, int w, int h,
                int rotate = 0, int flags = 0) const;
    void RenderWithMatrix(Bitmap& bitmap, const FS_MATRIX& matrix,
                          const FS_RECTF* clipping = nullptr, int flags = 0) const;

    // ── 文本 ──
    std::unique_ptr<TextPage> LoadTextPage();

    // ── 注释 ──
    int GetAnnotCount() const;

    // ── 页面盒子 ──
    bool GetMediaBox(float* left, float* bottom, float* right, float* top) const;
    void SetMediaBox(float left, float bottom, float right, float top);
    // ... GetCropBox, GetBleedBox, GetTrimBox, GetArtBox ...

    // ── 展平 ──
    int Flatten(int flags = 0);

    // ── 缩略图 ──
    std::unique_ptr<Bitmap> GetThumbnailAsBitmap();

    // ── 结构树（无障碍）──
    StructTree GetStructTree();

    // ── 坐标转换 ──
    void DeviceToPage(..., double* page_x, double* page_y) const;
    void PageToDevice(..., int* device_x, int* device_y) const;

    FPDF_PAGE Handle() const;
    explicit operator bool() const;
};
```

### 4. 位图渲染 (`bitmap.h`)

```cpp
class Bitmap {
public:
    // 格式常量
    static constexpr int kUnknown = 0;
    static constexpr int kGray = 1;
    static constexpr int kBGR = 2;
    static constexpr int kBGRx = 3;
    static constexpr int kBGRA = 4;
    static constexpr int kBGRA_Premul = 5;

    // 创建位图
    Bitmap(int width, int height, int alpha = 1);
    Bitmap(int width, int height, int format, void* first_scan, int stride);
    ~Bitmap();

    int GetWidth() const;
    int GetHeight() const;
    int GetStride() const;
    void* GetBuffer() const;
    void FillRect(int left, int top, int width, int height, uint32_t color);

    FPDF_BITMAP Handle() const;
    explicit operator bool() const;
};

// ── 渲染示例 ──
auto page = doc->LoadPage(0);
int w = (int)page->GetWidth();
int h = (int)page->GetHeight();

pdfium::Bitmap bitmap(w, h, 0);  // 不透明
page->Render(bitmap, 0, 0, w, h);
// bitmap.GetBuffer() 包含 BGRA 像素数据
```

### 5. 文本提取 (`text.h`)

```cpp
class TextPage {
public:
    int CharCount() const;                    // 字符总数
    unsigned int GetUnicode(int index) const; // 获取 Unicode
    double GetFontSize(int index) const;      // 字号

    std::wstring GetText(int start, int count) const;   // 提取文本
    std::wstring GetBoundedText(double l, double t,      // 区域文本
                                double r, double b) const;

    bool GetCharBox(int index, double* l, double* r,
                    double* b, double* t) const;  // 字符包围盒
};

class TextFind {
public:
    bool FindNext();          // 向后搜索
    bool FindPrev();          // 向前搜索
    int GetResultIndex() const;  // 匹配起始位置
    int GetResultCount() const;  // 匹配长度
};

class WebLinks {
public:
    int Count() const;                    // URL 数量
    std::wstring GetURL(int index) const;  // 获取 URL
    int CountRects(int link_index) const;  // 链接矩形数
};
```

### 6. 书签导航 (`bookmark.h`)

```cpp
class Bookmark {
public:
    Bookmark GetFirstChild() const;          // 第一个子书签
    Bookmark GetNextSibling() const;         // 下一个兄弟书签
    int GetCount() const;                    // 子书签计数（仅用于展开状态）
    std::wstring GetTitle() const;           // 标题

    std::unique_ptr<Dest> GetDest(const Document& doc) const;   // 关联目标
    std::unique_ptr<Action> GetAction() const;                  // 关联动作
};

class Dest {
public:
    int GetPageIndex(const Document& doc) const;       // 目标页码
    unsigned long GetView(unsigned long* np, FS_FLOAT* p);  // 视图类型
};

class Action {
public:
    unsigned long GetType() const;
    std::unique_ptr<Dest> GetDest(const Document& doc) const;
    std::string GetURIPath(const Document& doc) const;
};
```

### 7. 注释 (`annotation.h`)

```cpp
enum class AnnotationSubtype {
    Unknown, Text, Link, FreeText, Line, Square, Circle,
    Highlight, Underline, StrikeOut, Stamp, Ink,
    Popup, FileAttachment, Widget, Screen, ...  // 28 种
};

class Annotation {
public:
    AnnotationSubtype GetSubtype() const;
    FS_RECTF GetRect() const;
    void SetRect(const FS_RECTF& rect);
    int GetFlags() const;
    void SetFlags(int flags);

    bool SetColor(unsigned int R, unsigned int G, unsigned int B, unsigned int A);
    bool GetColor(unsigned int* R, unsigned int* G, unsigned int* B, unsigned int* A);
    bool SetBorder(float h_radius, float v_radius, float border_width);

    std::wstring GetStringValue(const std::string& key) const;
    bool SetStringValue(const std::string& key, const std::wstring& value);
};
```

### 8. 数字签名 (`signature.h`)

```cpp
class Signature {
public:
    std::vector<uint8_t> GetContents() const;    // 签名内容
    std::vector<int> GetByteRange() const;        // 字节范围
    std::string GetSubFilter() const;             // 子过滤器
    std::wstring GetReason() const;               // 签名原因
    std::string GetTime() const;                  // 签名时间
    unsigned int GetDocMDPPermission() const;      // 权限
};
```

### 9. 附件 (`attachment.h`)

```cpp
class Attachment {
public:
    std::wstring GetName() const;
    bool HasKey(const std::string& key) const;
    std::wstring GetStringValue(const std::string& key) const;
    bool SetStringValue(const std::string& key, const std::wstring& value);
    bool SetFile(const Document& doc, const void* data, unsigned long len);
    std::vector<uint8_t> GetFile() const;
    std::wstring GetSubtype() const;
};
```

### 10. 表单填充 (`form.h`)

```cpp
class FormHandle {
public:
    FPDF_FORMHANDLE Handle() const;
    explicit operator bool() const;
};
```

### 11. 图像对象 (`image.h`)

```cpp
struct ImageMetadata {
    int width, height;
    float horizontal_dpi, vertical_dpi;
    int bits_per_pixel, colorspace;
};

class ImageObject {
public:
    ImageMetadata GetMetadata(const Page& page) const;
    std::unique_ptr<Bitmap> GetBitmap() const;
    std::unique_ptr<Bitmap> GetRenderedBitmap(const Document& doc, const Page& page) const;
    std::vector<uint8_t> GetImageDataDecoded() const;
    std::vector<uint8_t> GetImageDataRaw() const;
    int GetFilterCount() const;
    std::string GetFilter(int index) const;
};
```

### 12. 字体 (`font.h`)

```cpp
class Font {
public:
    static std::unique_ptr<Font> LoadFromData(const Document& doc,
        const uint8_t* data, uint32_t size, int font_type, bool cid = false);
    static std::unique_ptr<Font> LoadStandard(const Document& doc,
        const std::string& font_name);

    std::string GetBaseFontName() const;
    std::string GetFamilyName() const;
    bool IsEmbedded() const;
    int GetFlags() const;
    int GetWeight() const;
    bool GetAscent(float font_size, float* ascent) const;
    bool GetDescent(float font_size, float* descent) const;
    bool GetGlyphWidth(uint32_t glyph, float font_size, float* width) const;
};
```

### 13. 路径对象 (`path.h`)

```cpp
class PathSegment {
public:
    bool GetPoint(float* x, float* y) const;
    int GetType() const;  // 0=LineTo, 1=BezierTo, 2=MoveTo
    bool IsClose() const;
};

class PathObject {
public:
    static std::unique_ptr<PageObject> CreatePath(float x, float y);
    static std::unique_ptr<PageObject> CreateRect(float x, float y, float w, float h);

    int CountSegments() const;
    PathSegment GetSegment(int index) const;

    bool MoveTo(float x, float y);
    bool LineTo(float x, float y);
    bool BezierTo(float x1, float y1, float x2, float y2, float x3, float y3);
    bool Close();

    bool SetFillColor(unsigned int R, unsigned int G, unsigned int B, unsigned int A);
    bool SetStrokeColor(unsigned int R, unsigned int G, unsigned int B, unsigned int A);
    bool SetStrokeWidth(float width);
    bool SetDashArray(const float* dash_array, size_t dash_count, float phase);
};
```

### 14. 结构树 / 无障碍 (`struct_tree.h`)

```cpp
class StructTree {
public:
    int CountChildren() const;
    StructElement GetChildAtIndex(int index) const;
};

class StructElement {
public:
    std::wstring GetType() const;       // 元素类型（如 "Document", "P", "H1"）
    std::wstring GetTitle() const;
    std::wstring GetAltText() const;    // 替代文本（无障碍）
    std::wstring GetActualText() const;
    std::wstring GetID() const;
    std::wstring GetLang() const;       // 语言
    int CountChildren() const;
    StructElement GetChildAtIndex(int index) const;
    StructElement GetParent() const;
};
```

### 15. 渐进加载 (`dataavail.h`)

```cpp
class Avail {
public:
    Avail(FX_FILEAVAIL* file_avail, FPDF_FILEACCESS* file);
    ~Avail();

    bool IsDocAvail();                      // 文档是否可获取
    std::unique_ptr<Document> GetDocument(const std::string& password = "");
    int GetFirstPageNum() const;             // 第一可显示页
    bool IsPageAvail(int page_index);        // 页面是否可用
    bool IsLinearized() const;               // 是否线性化 PDF
};
```

---

## 示例详解

### 示例 1: basic — 基础文档操作

```cpp
// examples/basic.cpp
#include <pdfium/pdfium.h>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "用法: pdfium_basic <pdf文件>\n";
        return 1;
    }

    // 1. 初始化 PDFium 库
    pdfium::Library lib;

    // 2. 加载文档
    auto doc = pdfium::Document::LoadFromFile(argv[1]);
    if (!doc) {
        auto err = pdfium::Library::GetLastError();
        std::cerr << "加载失败, 错误码: " << static_cast<int>(err) << "\n";
        return 1;
    }

    // 3. 输出文档信息
    int pages = doc->GetPageCount();
    int version = doc->GetFileVersion();
    std::cout << "页数: " << pages << "\n"
              << "版本: " << version / 10 << "." << version % 10 << "\n"
              << "标题: " << doc->GetMetaText("Title") << "\n"
              << "作者: " << doc->GetMetaText("Author") << "\n";

    // 4. 渲染第一页
    auto page = doc->LoadPage(0);
    if (page) {
        double pw = page->GetWidth();
        double ph = page->GetHeight();
        std::cout << "页面尺寸: " << pw << " x " << ph << "\n";

        pdfium::Bitmap bitmap(static_cast<int>(pw), static_cast<int>(ph));
        page->Render(bitmap, 0, 0, static_cast<int>(pw), static_cast<int>(ph));

        // 保存为 PPM
        // ...（见 basic.cpp 完整代码）
    }

    return 0;
}
```

**编译与运行**:
```bash
cmake .. -DPDFIUM_ROOT=../pdfium_cmake_support -DBUILD_EXAMPLES=ON
cmake --build .
./examples/pdfium_basic test.pdf
```

### 示例 2: text — 文本提取

```cpp
auto text = page->LoadTextPage();
int count = text->CharCount();
std::wstring content = text->GetText(0, count);
std::wcout << content;
```

**提取所有文本** — 按字符遍历或按区域提取。

### 示例 3: bookmark — 书签遍历

递归遍历书签树，打印每级标题和目标页码。

### 示例 4: annotation — 注释枚举

遍历页面注释，打印类型、位置、颜色。

### 示例 5: search — 文本搜索

使用 `TextFind` 类在文档中搜索关键字，返回所有匹配位置。

### 示例 6: render — 多页面渲染

遍历所有页面，渲染为不同缩放比例的 PPM 图像。

---

## 构建配置

### CMake 选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `PDFIUM_ROOT` | 空 | PDFium 编译产物的路径（含 include/ 和 lib/） |
| `BUILD_EXAMPLES` | OFF | 是否构建示例程序 |

### 与 pdfium_cmake_support 配合使用

```bash
# 方式一：设置 PDFIUM_ROOT
cd modern-pdfium-cmake-starter
mkdir build && cd build
cmake .. -DPDFIUM_ROOT=../pdfium_cmake_support -DBUILD_EXAMPLES=ON -G Ninja
cmake --build . --parallel

# 方式二：使用系统安装的 PDFium
cmake .. -DBUILD_EXAMPLES=ON
```

### 依赖关系

```
modern-pdfium-cmake-starter  (C++ Wrapper)
  └── pdfium_cmake_support    (PDFium CMake Build)
        ├── pdfium.dll / pdfium.lib
        ├── include/fpdfview.h
        └── deps/               (ICU, HarfBuzz, Abseil, JPEG)
              ├── include/
              └── lib/
```

---

## 高级用法

### 自定义文件访问

```cpp
// 通过 FPDF_FILEACCESS 自定义文件读取器
struct MyFileAccess : public FPDF_FILEACCESS {
    // 实现 m_GetBlock 回调
};

auto doc = pdfium::Document::LoadFromCustom(&file_access);
```

### 渐进加载（网络 PDF）

```cpp
// 适用于从网络流式加载 PDF
FX_FILEAVAIL file_avail;
FPDF_FILEACCESS file_access;

pdfium::Avail avail(&file_avail, &file_access);
while (!avail.IsDocAvail()) {
    // 下载更多数据...  
}
auto doc = avail.GetDocument("password");
// 现在可以访问文档，但页面可能尚未完全下载
while (!avail.IsPageAvail(0)) {
    // 等待页面数据...
}
auto page = doc->LoadPage(0);
```

### 文本搜索与定位

```cpp
auto text = page->LoadTextPage();
auto find = pdfium::TextFind(
    FPDFText_FindStart(text->Handle(), L"search_term", 0, 0));

while (find.FindNext()) {
    int idx = find.GetResultIndex();
    int len = find.GetResultCount();
    std::wstring match = text->GetText(idx, len);
    // 获取位置
    double l, t, r, b;
    text->GetRect(idx, &l, &t, &r, &b);
}
```

### 注释创建

```cpp
// 创建高亮注释
FPDF_ANNOTATION annot_handle = FPDFPage_CreateAnnot(
    page->Handle(), FPDF_ANNOT_HIGHLIGHT);

pdfium::Annotation annot(annot_handle);
annot.SetRect({100, 100, 200, 200});
annot.SetColor(255, 255, 0, 128);  // 黄色半透明

// 设置附着点（高亮文本区域）
FS_QUADPOINTSF qp = {100, 200, 200, 200, 100, 100, 200, 100};
FPDFAnnot_AppendAttachmentPoints(annot_handle, &qp);
```

### 字体加载与使用

```cpp
// 加载自定义 TrueType 字体
std::vector<uint8_t> font_data = LoadFile("myfont.ttf");
auto font = pdfium::Font::LoadFromData(
    *doc, font_data.data(), font_data.size(), FPDF_FONT_TRUETYPE);

// 创建文本对象
auto text_obj = pdfium::PageObject::CreateTextObj(*doc, font, 12.0f);
auto page_obj = text_obj->Handle();
FPDFText_SetText(page_obj, L"Hello PDFium!");
page->InsertObject(page_obj);
```
