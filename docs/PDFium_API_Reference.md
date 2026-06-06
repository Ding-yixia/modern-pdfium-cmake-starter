# PDFium C API 完整参考手册

> 本文档详细记录了 PDFium 所有公开 C API 函数，包括函数原型、参数说明、返回值和使用注意事项。
> 适用于 `pdfium_cmake_support` 项目（基于 Chromium 分支的 PDFium）。

---

## 目录

1. [fpdfview.h — 核心库与文档操作](#1-fpdfviewh--核心库与文档操作)
2. [fpdf_edit.h — 文档编辑](#2-fpdf_edith--文档编辑)
3. [fpdf_annot.h — 注释](#3-fpdf_annoth--注释)
4. [fpdf_text.h — 文本提取](#4-fpdf_texth--文本提取)
5. [fpdf_formfill.h — 表单填充](#5-fpdf_formfillh--表单填充)
6. [fpdf_doc.h — 书签/目标/链接/元数据](#6-fpdf_doch--书签目标链接元数据)
7. [fpdf_attachment.h — 附件](#7-fpdf_attachmenth--附件)
8. [fpdf_signature.h — 数字签名](#8-fpdf_signatureh--数字签名)
9. [fpdf_structtree.h — 结构树/无障碍](#9-fpdf_structtreeh--结构树无障碍)
10. [fpdf_save.h — 文档保存](#10-fpdf_saveh--文档保存)
11. [fpdf_dataavail.h — 渐进加载](#11-fpdf_dataavailh--渐进加载)
12. [fpdf_progressive.h — 渐进渲染](#12-fpdf_progressiveh--渐进渲染)
13. [fpdf_transformpage.h — 页面变换](#13-fpdf_transformpageh--页面变换)
14. [fpdf_flatten.h — 展平注释](#14-fpdf_flattenh--展平注释)
15. [fpdf_ppo.h — 页面组织](#15-fpdf_ppoh--页面组织)
16. [fpdf_ext.h — 扩展功能](#16-fpdf_exth--扩展功能)
17. [fpdf_catalog.h — 目录](#17-fpdf_catalogh--目录)
18. [fpdf_searchex.h — 搜索扩展](#18-fpdf_searchexh--搜索扩展)
19. [fpdf_thumbnail.h — 缩略图](#19-fpdf_thumbnailh--缩略图)
20. [fpdf_javascript.h — JavaScript](#20-fpdf_javascripth--javascript)
21. [fpdf_sysfontinfo.h — 系统字体](#21-fpdf_sysfontinfoh--系统字体)

---

## 1. fpdfview.h — 核心库与文档操作

### 1.1 库初始化和销毁

#### FPDF_InitLibrary

```c
void FPDF_InitLibrary();
```

**说明**: 初始化 PDFium 库。在使用任何其他 PDFium API 之前必须先调用此函数。

**注意**: 
- 在整个程序生命周期中只需调用一次
- 线程不安全，必须在主线程中调用
- 程序退出时需调用 `FPDF_DestroyLibrary()`

#### FPDF_InitLibraryWithConfig

```c
void FPDF_InitLibraryWithConfig(const FPDF_LIBRARY_CONFIG* config);
```

**说明**: 使用自定义配置初始化 PDFium 库。

**参数**:
- `config`: 配置结构体指针

```c
typedef struct {
    int version;              // 结构体版本（当前为 2）
    int user_data_size;       // 用户数据大小
    void* user_data;          // 用户数据指针
} FPDF_LIBRARY_CONFIG;
```

**注意**: 版本 4 以后新增 `renderer_type` 和 `font_backend_type` 字段，可选择 AGG/Skia 渲染后端和 FreeType/Fontations 字体后端。

#### FPDF_DestroyLibrary

```c
void FPDF_DestroyLibrary();
```

**说明**: 销毁 PDFium 库，释放所有全局资源。程序退出前调用。

---

### 1.2 文档操作

#### FPDF_LoadDocument

```c
FPDF_DOCUMENT FPDF_LoadDocument(FPDF_STRING file_path, FPDF_BYTESTRING password);
```

**说明**: 从文件加载 PDF 文档。

**参数**:
- `file_path`: 文件路径（UTF-8 编码）
- `password`: 密码（无密码时传 `NULL` 或 `""`）

**返回值**: 成功返回文档句柄，失败返回 `NULL`。调用 `FPDF_GetLastError()` 获取错误码。

**错误码**:
- `FPDF_ERR_SUCCESS` (0): 成功
- `FPDF_ERR_UNKNOWN` (1): 未知错误
- `FPDF_ERR_FILE` (2): 文件未找到
- `FPDF_ERR_FORMAT` (3): 格式错误
- `FPDF_ERR_PASSWORD` (4): 密码错误
- `FPDF_ERR_SECURITY` (5): 安全机制阻止
- `FPDF_ERR_PAGE` (6): 页面不存在

#### FPDF_LoadMemDocument

```c
FPDF_DOCUMENT FPDF_LoadMemDocument(const void* data_buf, int size, FPDF_BYTESTRING password);
```

**说明**: 从内存缓冲区加载 PDF 文档。

**参数**:
- `data_buf`: 内存缓冲区指针
- `size`: 数据大小（注意：`int` 类型，最大 2GB）
- `password`: 密码

#### FPDF_LoadMemDocument64

```c
FPDF_DOCUMENT FPDF_LoadMemDocument64(const void* data_buf, size_t size, FPDF_BYTESTRING password);
```

**说明**: `FPDF_LoadMemDocument` 的 64 位版本。支持大于 2GB 的文档。

#### FPDF_LoadCustomDocument

```c
FPDF_DOCUMENT FPDF_LoadCustomDocument(FPDF_FILEACCESS* pFileAccess, FPDF_BYTESTRING password);
```

**说明**: 从自定义文件读取器加载 PDF 文档。适用于从网络下载或加密存储中加载。

**FPDF_FILEACCESS 结构体**:

```c
typedef struct {
    unsigned long m_FileLen;       // 文件长度
    int (*m_GetBlock)(void* param, unsigned long position,
                      unsigned char* pBuf, unsigned long size);  // 读取回调
    void* m_Param;                 // 用户参数
} FPDF_FILEACCESS;
```

#### FPDF_CloseDocument

```c
void FPDF_CloseDocument(FPDF_DOCUMENT document);
```

**说明**: 关闭文档句柄，释放关联资源。

#### FPDF_GetFileVersion

```c
FPDF_BOOL FPDF_GetFileVersion(FPDF_DOCUMENT doc, int* fileVersion);
```

**说明**: 获取 PDF 文件版本号。

**返回值**: `fileVersion` 被设置为版本号（如 17 代表 PDF 1.7）。

#### FPDF_GetDocPermissions

```c
unsigned long FPDF_GetDocPermissions(FPDF_DOCUMENT document);
```

**说明**: 获取文档权限标志（基于 PDF 标准权限位）。

**权限标志位**:
- 第 3 位: 打印
- 第 4 位: 修改
- 第 5 位: 复制
- 第 6 位: 添加/修改注释
- 第 9 位: 填写表单
- 第 10 位: 提取文本和图形（无障碍）
- 第 11 位: 组装文档
- 第 12 位: 高质量打印

#### FPDF_GetDocUserPermissions

```c
unsigned long FPDF_GetDocUserPermissions(FPDF_DOCUMENT document);
```

**说明**: 获取文档用户权限（在解密后可用）。

#### FPDF_GetSecurityHandlerRevision

```c
int FPDF_GetSecurityHandlerRevision(FPDF_DOCUMENT document);
```

**说明**: 获取安全处理器修订号。

#### FPDF_GetPageCount

```c
int FPDF_GetPageCount(FPDF_DOCUMENT document);
```

**说明**: 获取文档总页数。

---

### 1.3 页面操作

#### FPDF_LoadPage

```c
FPDF_PAGE FPDF_LoadPage(FPDF_DOCUMENT document, int page_index);
```

**说明**: 加载指定索引的页面。索引从 0 开始。

**注意**: 页面使用完后必须调用 `FPDF_ClosePage` 释放。

#### FPDF_ClosePage

```c
void FPDF_ClosePage(FPDF_PAGE page);
```

#### FPDF_GetPageWidth / FPDF_GetPageWidthF

```c
double FPDF_GetPageWidth(FPDF_PAGE page);
float  FPDF_GetPageWidthF(FPDF_PAGE page);   // [实验]
```

#### FPDF_GetPageHeight / FPDF_GetPageHeightF

```c
double FPDF_GetPageHeight(FPDF_PAGE page);
float  FPDF_GetPageHeightF(FPDF_PAGE page);   // [实验]
```

#### FPDF_GetPageBoundingBox

```c
FPDF_BOOL FPDF_GetPageBoundingBox(FPDF_PAGE page, FS_RECTF* rect);
```

**说明**: 获取页面的边界框（MediaBox，单位：点）。

#### FPDF_GetPageSizeByIndex / FPDF_GetPageSizeByIndexF

```c
int FPDF_GetPageSizeByIndex(FPDF_DOCUMENT document, int page_index,
                            double* width, double* height);
FPDF_BOOL FPDF_GetPageSizeByIndexF(FPDF_DOCUMENT document, int page_index,
                                    FS_SIZEF* size);
```

**说明**: 在不加载页面的情况下获取页面尺寸。

---

### 1.4 渲染

#### FPDF_RenderPage

```c
FPDF_BOOL FPDF_RenderPage(HDC dc, FPDF_PAGE page, int start_x, int start_y,
                           int size_x, int size_y, int rotate, int flags);
```

**说明**: [仅 Windows] 渲染页面到 GDI 设备上下文。

#### FPDF_RenderPageBitmap

```c
void FPDF_RenderPageBitmap(FPDF_BITMAP bitmap, FPDF_PAGE page,
                           int start_x, int start_y,
                           int size_x, int size_y,
                           int rotate, int flags);
```

**说明**: 渲染页面到位图对象。

**参数**:
- `bitmap`: 目标位图句柄
- `page`: 页面句柄
- `start_x, start_y`: 渲染起始坐标
- `size_x, size_y`: 渲染尺寸
- `rotate`: 旋转角度（0/1/2/3 对应 0°/90°/180°/270°）
- `flags`: 渲染标志（见下方）

**渲染标志**:
| 标志 | 值 | 说明 |
|------|-----|------|
| `FPDF_ANNOT` | 0x01 | 渲染注释 |
| `FPDF_LCD_TEXT` | 0x02 | LCD 文本优化 |
| `FPDF_GRAYSCALE` | 0x08 | 灰度渲染 |
| `FPDF_REVERSE_BYTE_ORDER` | 0x10 | 反转字节序 |
| `FPDF_CONVERT_FILL_TO_STROKE` | 0x20 | 填充转描边 |
| `FPDF_RENDER_LIMITEDIMAGECACHE` | 0x200 | 限制图像缓存 |
| `FPDF_RENDER_FORCEHALFTONE` | 0x400 | 强制半色调 |
| `FPDF_PRINTING` | 0x800 | 打印模式 |
| `FPDF_RENDER_NO_SMOOTHTEXT` | 0x1000 | 文本无平滑 |
| `FPDF_RENDER_NO_SMOOTHIMAGE` | 0x2000 | 图像无平滑 |
| `FPDF_RENDER_NO_SMOOTHPATH` | 0x4000 | 路径无平滑 |

#### FPDF_RenderPageBitmapWithMatrix

```c
void FPDF_RenderPageBitmapWithMatrix(FPDF_BITMAP bitmap, FPDF_PAGE page,
                                     const FS_MATRIX* matrix,
                                     const FS_RECTF* clipping,
                                     int flags);
```

**说明**: 使用 2D 变换矩阵渲染页面，支持缩放、旋转和平移。

---

### 1.5 位图操作

#### FPDFBitmap_Create

```c
FPDF_BITMAP FPDFBitmap_Create(int width, int height, int alpha);
```

**说明**: 创建位图对象。

**参数**:
- `width`, `height`: 位图尺寸（像素）
- `alpha`: 是否包含 Alpha 通道（0=不透明 BGR，1=带 Alpha BGRA）

#### FPDFBitmap_CreateEx

```c
FPDF_BITMAP FPDFBitmap_CreateEx(int width, int height, int format,
                                 void* first_scan, int stride);
```

**说明**: 创建自定义格式位图。

**format 常量**:
| 常量 | 值 | 格式 |
|------|-----|------|
| `FPDFBitmap_Unknown` | 0 | 未知 |
| `FPDFBitmap_Gray` | 1 | 8 位灰度 |
| `FPDFBitmap_BGR` | 2 | 24 位 BGR |
| `FPDFBitmap_BGRx` | 3 | 32 位 BGR（无 Alpha） |
| `FPDFBitmap_BGRA` | 4 | 32 位 BGRA |
| `FPDFBitmap_BGRA_Premul` | 5 | 预乘 Alpha |

#### FPDFBitmap_FillRect

```c
FPDF_BOOL FPDFBitmap_FillRect(FPDF_BITMAP bitmap, int left, int top,
                               int width, int height, FPDF_DWORD color);
```

**说明**: 填充位图的矩形区域。`color` 格式为 `0xAARRGGBB`。

#### FPDFBitmap_GetBuffer

```c
void* FPDFBitmap_GetBuffer(FPDF_BITMAP bitmap);
```

**说明**: 获取位图缓冲区指针。每个像素的数据格式取决于位图创建时的参数。

#### FPDFBitmap_GetWidth, FPDFBitmap_GetHeight, FPDFBitmap_GetStride

```c
int FPDFBitmap_GetWidth(FPDF_BITMAP bitmap);
int FPDFBitmap_GetHeight(FPDF_BITMAP bitmap);
int FPDFBitmap_GetStride(FPDF_BITMAP bitmap);
```

**说明**: Stride（跨距）表示每行像素的字节数，可能大于 `width * bytes_per_pixel`（因内存对齐）。

#### FPDFBitmap_GetFormat

```c
int FPDFBitmap_GetFormat(FPDF_BITMAP bitmap);
```

#### FPDFBitmap_Destroy

```c
void FPDFBitmap_Destroy(FPDF_BITMAP bitmap);
```

---

### 1.6 坐标转换

#### FPDF_DeviceToPage

```c
FPDF_BOOL FPDF_DeviceToPage(FPDF_PAGE page, int start_x, int start_y,
                             int size_x, int size_y, int rotate,
                             int device_x, int device_y,
                             double* page_x, double* page_y);
```

**说明**: 将设备坐标（像素）转换为页面坐标（点）。

#### FPDF_PageToDevice

```c
FPDF_BOOL FPDF_PageToDevice(FPDF_PAGE page, int start_x, int start_y,
                             int size_x, int size_y, int rotate,
                             double page_x, double page_y,
                             int* device_x, int* device_y);
```

**说明**: 将页面坐标（点）转换为设备坐标（像素）。

---

### 1.7 文档信息

#### FPDF_GetMetaText

```c
unsigned long FPDF_GetMetaText(FPDF_DOCUMENT document, FPDF_BYTESTRING tag,
                                void* buffer, unsigned long buflen);
```

**说明**: 获取文档元数据。`tag` 可以是 `"Title"`, `"Author"`, `"Subject"`, `"Keywords"`, `"Creator"`, `"Producer"`, `"CreationDate"`, `"ModDate"` 等。

**返回值**: 实际需要的缓冲区长度（字符数）。输出为 UTF-16LE 编码。

#### FPDF_GetPageLabel

```c
unsigned long FPDF_GetPageLabel(FPDF_DOCUMENT document, int page_index,
                                void* buffer, unsigned long buflen);
```

**说明**: 获取页面标签（如 "i", "ii", "1-1" 等）。

---

### 1.8 查看器偏好

#### FPDF_VIEWERREF_GetPrintScaling

```c
FPDF_BOOL FPDF_VIEWERREF_GetPrintScaling(FPDF_DOCUMENT document);
```

#### FPDF_VIEWERREF_GetNumCopies

```c
int FPDF_VIEWERREF_GetNumCopies(FPDF_DOCUMENT document);
```

#### FPDF_VIEWERREF_GetPrintPageRange

```c
FPDF_PAGERANGE FPDF_VIEWERREF_GetPrintPageRange(FPDF_DOCUMENT document);
```

#### FPDF_VIEWERREF_GetDuplex

```c
FPDF_DUPLEXTYPE FPDF_VIEWERREF_GetDuplex(FPDF_DOCUMENT document);
```

**返回值**: `DuplexUndefined=0`, `Simplex=1`, `DuplexFlipShortEdge=2`, `DuplexFlipLongEdge=3`。

---

### 1.9 其他

#### FPDF_GetLastError

```c
unsigned long FPDF_GetLastError();
```

**说明**: 获取最后操作的错误码。在 `FPDF_LoadDocument` 返回 NULL 后调用。

#### FPDF_SetSandBoxPolicy

```c
void FPDF_SetSandBoxPolicy(FPDF_DWORD policy, FPDF_BOOL enable);
```

#### FPDF_SetPrintMode

```c
FPDF_BOOL FPDF_SetPrintMode(int mode);
```

---

## 2. fpdf_edit.h — 文档编辑

### 2.1 页面创建和删除

#### FPDF_CreateNewDocument

```c
FPDF_DOCUMENT FPDF_CreateNewDocument();
```

**说明**: 创建一个新的空白 PDF 文档。

#### FPDFPage_New

```c
FPDF_PAGE FPDFPage_New(FPDF_DOCUMENT document, int page_index,
                        double width, double height);
```

**说明**: 在指定位置创建新页面。

#### FPDFPage_Delete

```c
void FPDFPage_Delete(FPDF_DOCUMENT document, int page_index);
```

---

### 2.2 页面对象

#### FPDFPageObj_NewTextObj

```c
FPDF_PAGEOBJECT FPDFPageObj_NewTextObj(FPDF_DOCUMENT document,
                                        FPDF_BYTESTRING font,
                                        float font_size);
```

**说明**: 使用标准 14 种字体创建新的文本对象。

**标准字体名称**: `"Times-Roman"`, `"Times-Bold"`, `"Times-Italic"`, `"Helvetica"`, `"Helvetica-Bold"`, `"Courier"`, `"Symbol"`, `"ZapfDingbats"` 等。

#### FPDFText_SetText

```c
FPDF_BOOL FPDFText_SetText(FPDF_PAGEOBJECT text_object, FPDF_WIDESTRING text);
```

**说明**: 设置文本对象的文本内容。`text` 为 UTF-16LE 编码。

#### FPDFPage_InsertObject

```c
FPDF_BOOL FPDFPage_InsertObject(FPDF_PAGE page, FPDF_PAGEOBJECT page_object);
```

**说明**: 将页面对象插入到页面中。插入后页面拥有该对象的所有权。

#### FPDFPage_GenerateContent

```c
FPDF_BOOL FPDFPage_GenerateContent(FPDF_PAGE page);
```

**说明**: 在添加/修改页面对象后调用，重新生成页面内容流。

#### FPDFPageObj_Destroy

```c
void FPDFPageObj_Destroy(FPDF_PAGEOBJECT page_object);
```

**说明**: 销毁页面对象。如果对象已通过 `FPDFPage_InsertObject` 插入页面，则由页面管理。

---

### 2.3 路径对象

| 函数 | 说明 |
|------|------|
| `FPDFPageObj_CreateNewPath(x, y)` | 创建新路径对象 |
| `FPDFPageObj_CreateNewRect(x, y, w, h)` | 创建矩形路径 |
| `FPDFPath_MoveTo(path, x, y)` | 移动当前点 |
| `FPDFPath_LineTo(path, x, y)` | 添加直线段 |
| `FPDFPath_BezierTo(path, x1,y1, x2,y2, x3,y3)` | 添加贝塞尔曲线 |
| `FPDFPath_Close(path)` | 闭合子路径 |
| `FPDFPath_SetDrawMode(path, fillmode, stroke)` | 设置绘制模式（填充/描边） |

---

### 2.4 图像对象

| 函数 | 说明 |
|------|------|
| `FPDFPageObj_NewImageObj(doc)` | 创建新的图像对象 |
| `FPDFImageObj_LoadJpegFile(pages, count, obj, file_access)` | 从 JPEG 文件加载图像 |
| `FPDFImageObj_SetBitmap(pages, count, obj, bitmap)` | 设置图像的位图数据 |
| `FPDFImageObj_GetBitmap(obj)` | 获取图像的位图 |
| `FPDFImageObj_GetImageDataDecoded(obj, buf, len)` | 获取解码后的图像数据 |
| `FPDFImageObj_GetImageMetadata(obj, page, meta)` | 获取图像元数据 |
| `FPDFImageObj_SetMatrix(obj, a, b, c, d, e, f)` | 设置图像变换矩阵 |

**FPDF_IMAGEOBJ_METADATA 结构体**:
```c
typedef struct {
    int width;              // 图像宽度（像素）
    int height;             // 图像高度（像素）
    float horizontal_dpi;   // 水平 DPI
    float vertical_dpi;     // 垂直 DPI
    int bits_per_pixel;     // 每像素位数
    int colorspace;         // 色彩空间
    int marked_content_id;  // 标记内容 ID
} FPDF_IMAGEOBJ_METADATA;
```

---

### 2.5 字体操作

| 函数 | 说明 |
|------|------|
| `FPDFText_LoadFont(doc, data, size, font_type, cid)` | 从内存加载字体数据 |
| `FPDFText_LoadStandardFont(doc, font_name)` | 加载标准 14 种字体（实验性） |
| `FPDFPageObj_CreateTextObj(doc, font, size)` | 使用加载的字体创建文本对象 |
| `FPDFFont_Close(font)` | 关闭字体句柄 |
| `FPDFTextObj_GetText(obj, text_page, buf, len)` | 获取文本对象的文本内容 |
| `FPDFTextObj_GetFontSize(text, size)` | 获取文本对象的字体大小 |

---

### 2.6 对象属性

**颜色和样式**:
| 函数 | 说明 |
|------|------|
| `FPDFPageObj_SetFillColor(obj, R, G, B, A)` | 设置填充颜色 |
| `FPDFPageObj_GetFillColor(obj, R, G, B, A)` | 获取填充颜色 |
| `FPDFPageObj_SetStrokeColor(obj, R, G, B, A)` | 设置描边颜色 |
| `FPDFPageObj_GetStrokeColor(obj, R, G, B, A)` | 获取描边颜色 |
| `FPDFPageObj_SetStrokeWidth(obj, width)` | 设置描边宽度 |
| `FPDFPageObj_GetStrokeWidth(obj, width)` | 获取描边宽度 |
| `FPDFPageObj_GetLineJoin(obj)` | 获取线条连接方式 |
| `FPDFPageObj_SetLineJoin(obj, join)` | 设置线条连接方式 |
| `FPDFPageObj_GetLineCap(obj)` | 获取线条端点样式 |
| `FPDFPageObj_SetLineCap(obj, cap)` | 设置线条端点样式 |
| `FPDFPageObj_SetBlendMode(obj, mode)` | 设置混合模式 |

**变换**:
| 函数 | 说明 |
|------|------|
| `FPDFPageObj_Transform(obj, a, b, c, d, e, f)` | 变换对象（矩阵） |
| `FPDFPageObj_TransformF(obj, matrix)` | 使用 FS_MATRIX 变换（实验性） |
| `FPDFPageObj_SetMatrix(obj, matrix)` | 设置变换矩阵（实验性） |
| `FPDFPageObj_GetMatrix(obj, matrix)` | 获取变换矩阵（实验性） |
| `FPDFPageObj_GetBounds(obj, l, b, r, t)` | 获取边界框 |

---

## 3. fpdf_annot.h — 注释

### 3.1 注释子类型

```c
typedef enum {
    FPDF_ANNOT_UNKNOWN = 0,
    FPDF_ANNOT_TEXT = 1,           // 文本注释
    FPDF_ANNOT_LINK = 2,           // 链接注释
    FPDF_ANNOT_FREETEXT = 3,       // 自由文本
    FPDF_ANNOT_LINE = 4,           // 线条
    FPDF_ANNOT_SQUARE = 5,         // 矩形
    FPDF_ANNOT_CIRCLE = 6,         // 圆形
    FPDF_ANNOT_POLYGON = 7,        // 多边形
    FPDF_ANNOT_POLYLINE = 8,       // 折线
    FPDF_ANNOT_HIGHLIGHT = 9,      // 高亮
    FPDF_ANNOT_UNDERLINE = 10,     // 下划线
    FPDF_ANNOT_SQUIGGLY = 11,      // 波浪线
    FPDF_ANNOT_STRIKEOUT = 12,     // 删除线
    FPDF_ANNOT_STAMP = 13,         // 图章
    FPDF_ANNOT_CARET = 14,         // 插入符号
    FPDF_ANNOT_INK = 15,           // 墨迹
    FPDF_ANNOT_POPUP = 16,         // 弹出窗口
    FPDF_ANNOT_FILEATTACHMENT = 17, // 文件附件
    FPDF_ANNOT_SOUND = 18,         // 声音
    FPDF_ANNOT_MOVIE = 19,         // 电影
    FPDF_ANNOT_WIDGET = 20,        // 表单控件
    FPDF_ANNOT_SCREEN = 21,        // 屏幕
    FPDF_ANNOT_PRINTERMARK = 22,   // 打印标记
    FPDF_ANNOT_TRAPNET = 23,       // 陷印网络
    FPDF_ANNOT_WATERMARK = 24,     // 水印
    FPDF_ANNOT_THREED = 25,        // 3D
    FPDF_ANNOT_RICHMEDIA = 26,     // 富媒体
    FPDF_ANNOT_XFAWIDGET = 27,     // XFA 表单控件
    FPDF_ANNOT_REDACT = 28         // 密文
} FPDF_ANNOTATION_SUBTYPE;
```

### 3.2 注释操作

**创建和获取**:
| 函数 | 说明 |
|------|------|
| `FPDFPage_CreateAnnot(page, subtype)` | 在页面上创建注释 |
| `FPDFPage_GetAnnotCount(page)` | 获取页面注释数量 |
| `FPDFPage_GetAnnot(page, index)` | 获取指定索引的注释 |
| `FPDFPage_GetAnnotIndex(page, annot)` | 获取注释索引 |
| `FPDFPage_RemoveAnnot(page, index)` | 移除注释 |
| `FPDFPage_CloseAnnot(annot)` | 关闭注释句柄 |

**属性**:
| 函数 | 说明 |
|------|------|
| `FPDFAnnot_GetSubtype(annot)` | 获取注释子类型 |
| `FPDFAnnot_SetColor(annot, type, R, G, B, A)` | 设置颜色 |
| `FPDFAnnot_GetColor(annot, type, R, G, B, A)` | 获取颜色 |
| `FPDFAnnot_SetRect(annot, rect)` | 设置矩形 |
| `FPDFAnnot_GetRect(annot, rect)` | 获取矩形 |
| `FPDFAnnot_GetFlags(annot)` | 获取标志 |
| `FPDFAnnot_SetFlags(annot, flags)` | 设置标志 |
| `FPDFAnnot_SetBorder(annot, h, v, w)` | 设置边框 |
| `FPDFAnnot_GetBorder(annot, h, v, w)` | 获取边框 |

**内容操作**:
| 函数 | 说明 |
|------|------|
| `FPDFAnnot_HasKey(annot, key)` | 检查字典键是否存在 |
| `FPDFAnnot_GetStringValue(annot, key, buf, len)` | 获取字符串值 |
| `FPDFAnnot_SetStringValue(annot, key, value)` | 设置字符串值 |
| `FPDFAnnot_SetAP(annot, mode, value)` | 设置外观字符串 |

**附着点（文本标记注释专用）**:
| 函数 | 说明 |
|------|------|
| `FPDFAnnot_HasAttachmentPoints(annot)` | 是否有附着点 |
| `FPDFAnnot_CountAttachmentPoints(annot)` | 附着点数量 |
| `FPDFAnnot_GetAttachmentPoints(annot, index, points)` | 获取附着点 |
| `FPDFAnnot_SetAttachmentPoints(annot, index, points)` | 设置附着点 |

**对象操作**:
| 函数 | 说明 |
|------|------|
| `FPDFAnnot_GetObjectCount(annot)` | 获取对象数量 |
| `FPDFAnnot_GetObject(annot, index)` | 获取对象 |
| `FPDFAnnot_AppendObject(annot, obj)` | 追加对象 |
| `FPDFAnnot_RemoveObject(annot, index)` | 移除对象 |
| `FPDFAnnot_UpdateObject(annot, obj)` | 更新对象 |

**表单字段查询**:
| 函数 | 说明 |
|------|------|
| `FPDFAnnot_GetFormFieldAtPoint(handle, page, point)` | 获取点的表单字段 |
| `FPDFAnnot_GetFormFieldType(handle, annot)` | 获取字段类型 |
| `FPDFAnnot_GetFormFieldName(handle, annot, buf, len)` | 获取字段名 |
| `FPDFAnnot_GetFormFieldValue(handle, annot, buf, len)` | 获取字段值 |
| `FPDFAnnot_GetFormFieldFlags(handle, annot)` | 获取字段标志 |
| `FPDFAnnot_IsChecked(handle, annot)` | 复选框是否选中 |

---

## 4. fpdf_text.h — 文本提取

### 4.1 文本页面

**生命周期**:
| 函数 | 说明 |
|------|------|
| `FPDFText_LoadPage(page)` | 加载页面的文本信息 |
| `FPDFText_ClosePage(text_page)` | 关闭文本页面 |

**字符信息**:
| 函数 | 说明 |
|------|------|
| `FPDFText_CountChars(text_page)` | 获取字符数量 |
| `FPDFText_GetUnicode(text_page, index)` | 获取字符 Unicode 编码 |
| `FPDFText_GetFontSize(text_page, index)` | 获取字符字体大小 |
| `FPDFText_GetCharBox(text_page, index, l, r, b, t)` | 获取字符包围盒 |
| `FPDFText_GetCharOrigin(text_page, index, x, y)` | 获取字符原点 |
| `FPDFText_GetCharIndexAtPos(text_page, x, y, xTol, yTol)` | 获取位置处的字符索引 |

**文本提取**:
| 函数 | 说明 |
|------|------|
| `FPDFText_GetText(text_page, start, count, result)` | 提取 Unicode 文本 |
| `FPDFText_GetBoundedText(text_page, l, t, r, b, buf, len)` | 提取矩形区域文本 |
| `FPDFText_CountRects(text_page, start, count)` | 计算文本矩形数 |
| `FPDFText_GetRect(text_page, index, l, t, r, b)` | 获取文本矩形 |

### 4.2 文本搜索

| 函数 | 说明 |
|------|------|
| `FPDFText_FindStart(text_page, find_what, flags, start)` | 开始搜索 |
| `FPDFText_FindNext(handle)` | 查找下一个 |
| `FPDFText_FindPrev(handle)` | 查找上一个 |
| `FPDFText_GetSchResultIndex(handle)` | 获取匹配结果起始索引 |
| `FPDFText_GetSchCount(handle)` | 获取匹配字符数 |
| `FPDFText_FindClose(handle)` | 关闭搜索 |

**搜索标志**:
| 标志 | 值 | 说明 |
|------|-----|------|
| `FPDF_MATCHCASE` | 1 | 区分大小写 |
| `FPDF_MATCHWHOLEWORD` | 2 | 全字匹配 |
| `FPDF_CONSECUTIVE` | 4 | 连续匹配 |

### 4.3 Web 链接

| 函数 | 说明 |
|------|------|
| `FPDFLink_LoadWebLinks(text_page)` | 加载文本中的 Web 链接 |
| `FPDFLink_CountWebLinks(link_page)` | 统计 Web 链接数 |
| `FPDFLink_GetURL(link_page, index, buf, len)` | 获取链接 URL |
| `FPDFLink_CountRects(link_page, link_index)` | 统计链接矩形数 |
| `FPDFLink_GetRect(link_page, link_index, rect_index, l, t, r, b)` | 获取链接矩形 |
| `FPDFLink_CloseWebLinks(link_page)` | 关闭 Web 链接 |

---

## 5. fpdf_formfill.h — 表单填充

### 5.1 表单环境

| 函数 | 说明 |
|------|------|
| `FPDFDOC_InitFormFillEnvironment(doc, form_info)` | 初始化表单填充环境 |
| `FPDFDOC_ExitFormFillEnvironment(handle)` | 退出表单填充环境 |
| `FPDF_GetFormType(doc)` | 获取表单类型（None/AcroForm/XFA） |

### 5.2 页面事件

| 函数 | 说明 |
|------|------|
| `FORM_OnAfterLoadPage(page, handle)` | 页面加载后回调 |
| `FORM_OnBeforeClosePage(page, handle)` | 页面关闭前回调 |

### 5.3 鼠标和键盘事件

| 函数 | 说明 |
|------|------|
| `FORM_OnMouseMove(handle, page, modifier, x, y)` | 鼠标移动 |
| `FORM_OnLButtonDown(handle, page, modifier, x, y)` | 左键按下 |
| `FORM_OnLButtonUp(handle, page, modifier, x, y)` | 左键释放 |
| `FORM_OnRButtonDown(handle, page, modifier, x, y)` | 右键按下 |
| `FORM_OnRButtonUp(handle, page, modifier, x, y)` | 右键释放 |
| `FORM_OnLButtonDoubleClick(handle, page, modifier, x, y)` | 左键双击 |
| `FORM_OnKeyDown(handle, page, nKeyCode, modifier)` | 按键按下 |
| `FORM_OnKeyUp(handle, page, nKeyCode, modifier)` | 按键释放 |
| `FORM_OnChar(handle, page, nChar, modifier)` | 字符输入 |

### 5.4 选择和撤销

| 函数 | 说明 |
|------|------|
| `FORM_GetSelectedText(handle, page, buf, len)` | 获取选中文本 |
| `FORM_ReplaceSelection(handle, page, text)` | 替换选中文本 |
| `FORM_SelectAllText(handle, page)` | 全选（实验性） |
| `FORM_CanUndo(handle, page)` | 能否撤销 |
| `FORM_CanRedo(handle, page)` | 能否重做 |
| `FORM_Undo(handle, page)` | 撤销 |
| `FORM_Redo(handle, page)` | 重做 |

### 5.5 表单字段

| 函数 | 说明 |
|------|------|
| `FPDFPage_HasFormFieldAtPoint(handle, page, x, y)` | 获取点的字段类型 |
| `FPDF_SetFormFieldHighlightColor(handle, type, color)` | 设置高亮颜色 |
| `FPDF_SetFormFieldHighlightAlpha(handle, alpha)` | 设置高亮透明度 |
| `FPDF_RemoveFormFieldHighlight(handle)` | 移除高亮 |

### 5.6 表单渲染

| 函数 | 说明 |
|------|------|
| `FPDF_FFLDraw(handle, bitmap, page, x, y, w, h, rotate, flags)` | 渲染表单字段 |

---

## 6. fpdf_doc.h — 书签/目标/链接/元数据

### 6.1 书签

| 函数 | 说明 |
|------|------|
| `FPDFBookmark_GetFirstChild(doc, bookmark)` | 获取第一个子书签（bookmark 为 NULL 时获取根级别） |
| `FPDFBookmark_GetNextSibling(doc, bookmark)` | 获取下一个兄弟书签 |
| `FPDFBookmark_GetTitle(bookmark, buf, len)` | 获取书签标题（UTF-16LE） |
| `FPDFBookmark_GetCount(bookmark)` | 获取子书签计数 |
| `FPDFBookmark_Find(doc, title)` | 按标题查找书签 |
| `FPDFBookmark_GetDest(doc, bookmark)` | 获取书签目标 |
| `FPDFBookmark_GetAction(bookmark)` | 获取书签动作 |

### 6.2 目标

| 函数 | 说明 |
|------|------|
| `FPDFDest_GetDestPageIndex(doc, dest)` | 获取目标页面索引 |
| `FPDFDest_GetView(dest, num_params, params)` | 获取视图类型和参数 |
| `FPDFDest_GetLocationInPage(dest, hasX, hasY, hasZoom, x, y, zoom)` | 获取 XYZ 位置 |

**视图类型**:
| 类型 | 值 | 说明 |
|------|-----|------|
| `PDFDEST_VIEW_UNKNOWN_MODE` | 0 | 未知 |
| `PDFDEST_VIEW_XYZ` | 1 | 指定位置和缩放 |
| `PDFDEST_VIEW_FIT` | 2 | 适应窗口 |
| `PDFDEST_VIEW_FITH` | 3 | 适应宽度 |
| `PDFDEST_VIEW_FITV` | 4 | 适应高度 |
| `PDFDEST_VIEW_FITR` | 5 | 适应矩形 |
| `PDFDEST_VIEW_FITB` | 6 | 适应边界框 |
| `PDFDEST_VIEW_FITBH` | 7 | 适应边界框宽度 |
| `PDFDEST_VIEW_FITBV` | 8 | 适应边界框高度 |

### 6.3 动作

| 函数 | 说明 |
|------|------|
| `FPDFAction_GetType(action)` | 获取动作类型 |
| `FPDFAction_GetDest(doc, action)` | 获取动作目标 |
| `FPDFAction_GetFilePath(action, buf, len)` | 获取文件路径（远程跳转） |
| `FPDFAction_GetURIPath(doc, action, buf, len)` | 获取 URI 路径 |

**动作类型**:
| 类型 | 值 | 说明 |
|------|-----|------|
| `PDFACTION_UNSUPPORTED` | 0 | 不支持 |
| `PDFACTION_GOTO` | 1 | 跳转到本文档目标 |
| `PDFACTION_REMOTEGOTO` | 2 | 跳转到外部文档 |
| `PDFACTION_URI` | 3 | 打开 URI |
| `PDFACTION_LAUNCH` | 4 | 启动应用程序 |
| `PDFACTION_EMBEDDEDGOTO` | 5 | 跳转到嵌入式文件 |

### 6.4 链接

| 函数 | 说明 |
|------|------|
| `FPDFLink_GetLinkAtPoint(page, x, y)` | 获取点的链接 |
| `FPDFLink_GetLinkZOrderAtPoint(page, x, y)` | 获取链接 Z 顺序 |
| `FPDFLink_GetDest(doc, link)` | 获取链接目标 |
| `FPDFLink_GetAction(link)` | 获取链接动作 |
| `FPDFLink_Enumerate(page, start_pos, link_annot)` | 枚举链接 |
| `FPDFLink_GetAnnotRect(link, rect)` | 获取链接矩形 |

---

## 7. fpdf_attachment.h — 附件

| 函数 | 说明 |
|------|------|
| `FPDFDoc_GetAttachmentCount(doc)` | 获取附件数量 |
| `FPDFDoc_AddAttachment(doc, name)` | 添加新附件 |
| `FPDFDoc_GetAttachment(doc, index)` | 获取附件 |
| `FPDFDoc_DeleteAttachment(doc, index)` | 删除附件 |
| `FPDFAttachment_GetName(attachment, buf, len)` | 获取附件名称 |
| `FPDFAttachment_HasKey(attachment, key)` | 检查键是否存在 |
| `FPDFAttachment_GetStringValue(attachment, key, buf, len)` | 获取字符串值 |
| `FPDFAttachment_SetStringValue(attachment, key, value)` | 设置字符串值 |
| `FPDFAttachment_SetFile(attachment, doc, data, len)` | 设置文件内容 |
| `FPDFAttachment_GetFile(attachment, buf, len, out_len)` | 获取文件内容 |

---

## 8. fpdf_signature.h — 数字签名

| 函数 | 说明 |
|------|------|
| `FPDF_GetSignatureCount(doc)` | 获取签名数量 |
| `FPDF_GetSignatureObject(doc, index)` | 获取签名对象 |
| `FPDFSignatureObj_GetContents(sig, buf, len)` | 获取签名内容 |
| `FPDFSignatureObj_GetByteRange(sig, buf, len)` | 获取字节范围 |
| `FPDFSignatureObj_GetSubFilter(sig, buf, len)` | 获取子过滤器 |
| `FPDFSignatureObj_GetReason(sig, buf, len)` | 获取签名原因 |
| `FPDFSignatureObj_GetTime(sig, buf, len)` | 获取签名时间 |
| `FPDFSignatureObj_GetDocMDPPermission(sig)` | 获取文档 MDP 权限 |

---

## 9. fpdf_structtree.h — 结构树/无障碍

| 函数 | 说明 |
|------|------|
| `FPDF_StructTree_GetForPage(page)` | 获取页面的结构树 |
| `FPDF_StructTree_Close(tree)` | 关闭结构树 |
| `FPDF_StructTree_CountChildren(tree)` | 获取子元素数量 |
| `FPDF_StructTree_GetChildAtIndex(tree, index)` | 获取子元素 |
| `FPDF_StructElement_GetType(elem, buf, len)` | 获取元素类型 |
| `FPDF_StructElement_GetTitle(elem, buf, len)` | 获取标题 |
| `FPDF_StructElement_GetAltText(elem, buf, len)` | 获取替代文本（无障碍） |
| `FPDF_StructElement_GetLang(elem, buf, len)` | 获取语言 |
| `FPDF_StructElement_GetID(elem, buf, len)` | 获取 ID |
| `FPDF_StructElement_GetExpansion(elem, buf, len)` | 获取缩写扩展 |
| `FPDF_StructElement_GetActualText(elem, buf, len)` | 获取实际文本 |
| `FPDF_StructElement_GetMarkedContentID(elem)` | 获取标记内容 ID |
| `FPDF_StructElement_CountChildren(elem)` | 获取子元素数 |
| `FPDF_StructElement_GetChildAtIndex(elem, index)` | 获取子元素 |
| `FPDF_StructElement_GetParent(elem)` | 获取父元素 |
| `FPDF_StructElement_GetAttributeCount(elem)` | 获取属性数 |

---

## 10. fpdf_save.h — 文档保存

```c
typedef struct {
    unsigned long (*WriteBlock)(void* param, const void* data, unsigned long size);
    void* m_Param;
} FPDF_FILEWRITE;
```

| 函数 | 说明 |
|------|------|
| `FPDF_SaveAsCopy(doc, writer, flags)` | 保存文档副本 |
| `FPDF_SaveWithVersion(doc, writer, flags, version)` | 指定版本保存 |

**flags 标志**:
| 标志 | 值 | 说明 |
|------|-----|------|
| `FPDF_INCREMENTAL` | 0x01 | 增量保存（速度最快） |
| `FPDF_NO_INCREMENTAL` | 0x02 | 全量保存 |
| `FPDF_REMOVE_SECURITY` | 0x04 | 移除安全设置 |
| `FPDF_SUBSET_NEW_FONTS` | 0x08 | 新字体子集化 |

---

## 11. fpdf_dataavail.h — 渐进加载

适用于从网络流式加载 PDF（线性化 PDF 优化）。

| 函数 | 说明 |
|------|------|
| `FPDFAvail_Create(file_avail, file)` | 创建可用性提供者 |
| `FPDFAvail_Destroy(avail)` | 销毁提供者 |
| `FPDFAvail_IsDocAvail(avail, hints)` | 检查文档是否可用 |
| `FPDFAvail_GetDocument(avail, password)` | 获取文档 |
| `FPDFAvail_GetFirstPageNum(doc)` | 获取第一可用页码 |
| `FPDFAvail_IsPageAvail(avail, page, hints)` | 检查页面是否可用 |
| `FPDFAvail_IsFormAvail(avail, hints)` | 检查表单数据是否可用 |
| `FPDFAvail_IsLinearized(avail)` | 检查是否为线性化 PDF |

---

## 12. fpdf_progressive.h — 渐进渲染

适用于长时间渲染时的进度控制。

| 函数 | 说明 |
|------|------|
| `FPDF_RenderPageBitmap_Start(bitmap, page, x, y, w, h, rotate, flags, pause)` | 开始渐进渲染 |
| `FPDF_RenderPage_Continue(page, pause)` | 继续渲染 |
| `FPDF_RenderPage_Close(page)` | 关闭渲染 |

**状态常量**:
| 常量 | 值 | 说明 |
|------|-----|------|
| `FPDF_RENDER_READY` | 0 | 准备渲染 |
| `FPDF_RENDER_TOBECONTINUED` | 1 | 需要继续 |
| `FPDF_RENDER_DONE` | 2 | 完成 |
| `FPDF_RENDER_FAILED` | 3 | 失败 |

---

## 13. fpdf_transformpage.h — 页面变换

### 页面盒子

PDF 定义了 5 种盒子：
- **MediaBox**: 介质框（物理页面大小）
- **CropBox**: 裁剪框（显示区域）
- **BleedBox**: 出血框（印刷用）
- **TrimBox**: 裁切框（最终成品尺寸）
- **ArtBox**: 艺术框（内容区域）

| 函数 | 说明 |
|------|------|
| `FPDFPage_GetMediaBox(page, l, b, r, t)` | 获取介质框 |
| `FPDFPage_SetMediaBox(page, l, b, r, t)` | 设置介质框 |
| `FPDFPage_GetCropBox(page, l, b, r, t)` | 获取裁剪框 |
| `FPDFPage_SetCropBox(page, l, b, r, t)` | 设置裁剪框 |
| `FPDFPage_GetBleedBox(page, l, b, r, t)` | 获取出血框 |
| `FPDFPage_GetTrimBox(page, l, b, r, t)` | 获取裁切框 |
| `FPDFPage_GetArtBox(page, l, b, r, t)` | 获取艺术框 |

### 变换和裁剪

| 函数 | 说明 |
|------|------|
| `FPDFPage_TransFormWithClip(page, matrix, clip)` | 使用矩阵变换页面 |
| `FPDFPage_TransformAnnots(page, a, b, c, d, e, f)` | 变换注释 |
| `FPDFPageObj_TransformClipPath(obj, a, b, c, d, e, f)` | 变换裁剪路径 |
| `FPDF_CreateClipPath(l, b, r, t)` | 创建裁剪路径 |
| `FPDF_DestroyClipPath(clip_path)` | 销毁裁剪路径 |
| `FPDFPage_InsertClipPath(page, clip_path)` | 插入裁剪路径 |

---

## 14. fpdf_flatten.h — 展平注释

| 函数 | 说明 |
|------|------|
| `FPDFPage_Flatten(page, nFlag)` | 展平页面的所有注释 |

**返回值**: `FLATTEN_FAIL=0`, `FLATTEN_SUCCESS=1`, `FLATTEN_NOTHINGTODO=2`

**flag 参数**: `FLAT_NORMALDISPLAY=0`, `FLAT_PRINT=1`

---

## 15. fpdf_ppo.h — 页面组织

| 函数 | 说明 |
|------|------|
| `FPDF_ImportPages(dest, src, pagerange, index)` | 从源文档导入页面 |
| `FPDF_ImportPagesByIndex(dest, src, indices, len, index)` | 按索引导入 |
| `FPDF_ImportNPagesToOne(src, w, h, nx, ny)` | N-Up 拼版 |
| `FPDF_NewXObjectFromPage(dest, src, page)` | 从页面创建 XObject |
| `FPDF_CloseXObject(xobject)` | 关闭 XObject |
| `FPDF_NewFormObjectFromXObject(xobject)` | 从 XObject 创建表单对象 |
| `FPDF_CopyViewerPreferences(dest, src)` | 复制查看器偏好 |

---

## 16. fpdf_ext.h — 扩展功能

| 函数 | 说明 |
|------|------|
| `FSDK_SetUnSpObjProcessHandler(unsp_info)` | 设置不支持对象处理器 |
| `FPDFDoc_GetPageMode(doc)` | 获取文档页面模式 |

**页面模式**:
| 模式 | 值 | 说明 |
|------|-----|------|
| `PAGEMODE_UNKNOWN` | -1 | 未知 |
| `PAGEMODE_USENONE` | 0 | 无 |
| `PAGEMODE_USEOUTLINES` | 1 | 显示书签 |
| `PAGEMODE_USETHUMBS` | 2 | 显示缩略图 |
| `PAGEMODE_FULLSCREEN` | 3 | 全屏 |
| `PAGEMODE_USEOC` | 4 | 显示图层 |
| `PAGEMODE_USEATTACHMENTS` | 5 | 显示附件 |

---

## 17. fpdf_catalog.h — 目录

| 函数 | 说明 |
|------|------|
| `FPDFCatalog_IsTagged(doc)` | 检查是否为标签 PDF（无障碍） |
| `FPDFCatalog_GetLanguage(doc, buf, len)` | 获取文档语言 |
| `FPDFCatalog_SetLanguage(doc, lang)` | 设置文档语言 |

---

## 18. fpdf_searchex.h — 搜索扩展

| 函数 | 说明 |
|------|------|
| `FPDFText_GetCharIndexFromTextIndex(text_page, text_index)` | 从文本索引获取字符索引 |
| `FPDFText_GetTextIndexFromCharIndex(text_page, char_index)` | 从字符索引获取文本索引 |

**说明**: 用于转换文本提取索引和字符索引之间的映射关系。有些字符（如连字符、软换行）在提取的文本中不可见。

---

## 19. fpdf_thumbnail.h — 缩略图

| 函数 | 说明 |
|------|------|
| `FPDFPage_GetDecodedThumbnailData(page, buf, len)` | 获取解码后的缩略图数据 |
| `FPDFPage_GetRawThumbnailData(page, buf, len)` | 获取原始缩略图数据 |
| `FPDFPage_GetThumbnailAsBitmap(page)` | 渲染缩略图为位图 |

**说明**: 缩略图用于快速预览页面内容，无需完整渲染页面。`GetDecodedThumbnailData` 返回解码后的图像数据（通常是 RGB/BGR），`GetRawThumbnailData` 返回 PDF 中的原始压缩数据。

---

## 20. fpdf_javascript.h — JavaScript

| 函数 | 说明 |
|------|------|
| `FPDFDoc_GetJavaScriptActionCount(doc)` | 获取 JavaScript 动作数量 |
| `FPDFDoc_GetJavaScriptAction(doc, index)` | 获取 JavaScript 动作 |
| `FPDFDoc_CloseJavaScriptAction(js)` | 关闭 JavaScript 动作句柄 |
| `FPDFJavaScriptAction_GetName(js, buf, len)` | 获取脚本名称 |
| `FPDFJavaScriptAction_GetScript(js, buf, len)` | 获取脚本内容 |

---

## 21. fpdf_sysfontinfo.h — 系统字体信息

| 函数 | 说明 |
|------|------|
| `FPDF_GetDefaultTTFMap()` | 获取默认字符集到字体名映射 |
| `FPDF_GetDefaultTTFMapCount()` | 获取映射条目数（实验性） |
| `FPDF_GetDefaultTTFMapEntry(index)` | 获取映射条目（实验性） |
| `FPDF_AddInstalledFont(mapper, face, charset)` | 添加已安装字体 |
| `FPDF_SetSystemFontInfo(info)` | 设置系统字体信息接口 |
| `FPDF_GetDefaultSystemFontInfo()` | 获取默认系统字体信息 |
| `FPDF_FreeDefaultSystemFontInfo(info)` | 释放默认系统字体信息 |

---

## 附录 A：常用数据结构

### FS_MATRIX (2D 变换矩阵)

```c
typedef struct {
    float a;  // 水平缩放
    float b;  // 水平倾斜
    float c;  // 垂直倾斜
    float d;  // 垂直缩放
    float e;  // 水平平移
    float f;  // 垂直平移
} FS_MATRIX;
```

变换公式: `x' = a*x + c*y + e, y' = b*x + d*y + f`

### FS_RECTF (矩形)

```c
typedef struct {
    float left;
    float top;
    float right;
    float bottom;
} FS_RECTF;
```

### FS_POINTF (点)

```c
typedef struct {
    float x;
    float y;
} FS_POINTF;
```

### FS_SIZEF (尺寸)

```c
typedef struct {
    float width;
    float height;
} FS_SIZEF;
```

### FS_QUADPOINTSF (四边形)

```c
typedef struct {
    float x1, y1;  // 左上
    float x2, y2;  // 右上
    float x3, y3;  // 左下
    float x4, y4;  // 右下
} FS_QUADPOINTSF;
```

### FPDF_BSTR (字符串)

```c
typedef struct {
    char* str;   // 字符串指针
    int len;     // 长度
} FPDF_BSTR;
```

## 附录 B：常用常量

### 对象类型

```c
FPDF_OBJECT_UNKNOWN = 0;
FPDF_OBJECT_BOOLEAN = 1;
FPDF_OBJECT_NUMBER = 2;
FPDF_OBJECT_STRING = 3;
FPDF_OBJECT_NAME = 4;
FPDF_OBJECT_ARRAY = 5;
FPDF_OBJECT_DICTIONARY = 6;
FPDF_OBJECT_STREAM = 7;
FPDF_OBJECT_NULLOBJ = 8;
FPDF_OBJECT_REFERENCE = 9;
```

### 表单字段类型

```c
FPDF_FORMFIELD_UNKNOWN    = 0;
FPDF_FORMFIELD_PUSHBUTTON = 1;   // 按钮
FPDF_FORMFIELD_CHECKBOX   = 2;   // 复选框
FPDF_FORMFIELD_RADIOBUTTON = 3;  // 单选按钮
FPDF_FORMFIELD_COMBOBOX   = 4;   // 组合框
FPDF_FORMFIELD_LISTBOX    = 5;   // 列表框
FPDF_FORMFIELD_TEXTFIELD  = 6;   // 文本字段
FPDF_FORMFIELD_SIGNATURE  = 7;   // 签名字段
```

> 本文档由 `modern-pdfium-cmake-starter` 项目维护。
> 最后更新: 2025-06
