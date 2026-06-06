# PDFium API 使用流程指南

> 本文档详细介绍 PDFium 的各种使用场景和 API 调用流程，从简单到复杂，
> 涵盖完整的开发工作流和最佳实践。
> 
> 配合阅读: [API 参考手册](PDFium_API_Reference.md) | [C++ 封装库 API](API.md)

---

## 目录

1. [基础流程：打开与渲染 PDF](#1-基础流程打开与渲染-pdf)
2. [文本提取流程](#2-文本提取流程)
3. [文本搜索流程](#3-文本搜索流程)
4. [书签导航流程](#4-书签导航流程)
5. [注释操作流程](#5-注释操作流程)
6. [表单填充流程](#6-表单填充流程)
7. [创建和编辑 PDF 流程](#7-创建和编辑-pdf-流程)
8. [页面变换流程](#8-页面变换流程)
9. [数字签名验签流程](#9-数字签名验签流程)
10. [渐进加载流程（网络 PDF）](#10-渐进加载流程网络-pdf)
11. [结构树/无障碍流程](#11-结构树无障碍流程)
12. [附件操作流程](#12-附件操作流程)
13. [保存文档流程](#13-保存文档流程)
14. [完整错误处理流程](#14-完整错误处理流程)
15. [多线程安全注意事项](#15-多线程安全注意事项)

---

## 1. 基础流程：打开与渲染 PDF

### 流程图

```
初始化库 → 加载文档 → 获取页数 → 加载页面 → 获取尺寸 → 创建位图 → 渲染 → 保存/显示 → 释放资源 → 销毁库
```

### 详细步骤

#### 步骤 1：初始化 PDFium 库

每次使用 PDFium 必须先调用初始化。推荐在程序启动时初始化，退出时销毁。

```c
// 标准初始化
FPDF_InitLibrary();

// 或使用配置（推荐）
FPDF_LIBRARY_CONFIG config;
config.version = 2;
config.user_data_size = 0;
config.user_data = NULL;
FPDF_InitLibraryWithConfig(&config);
```

**注意**: 
- 全局只需初始化一次
- 必须在主线程中调用
- 对应程序结束前调用 `FPDF_DestroyLibrary()`

#### 步骤 2：加载文档

从文件加载：
```c
FPDF_DOCUMENT doc = FPDF_LoadDocument("document.pdf", NULL);
if (!doc) {
    unsigned long err = FPDF_GetLastError();
    // 处理错误
    return;
}
```

从内存加载（适用于网络下载、加密文件等）：
```c
// data 是从某处读取的 PDF 数据
FPDF_DOCUMENT doc = FPDF_LoadMemDocument64(data, data_size, NULL);
```

带密码加载：
```c
FPDF_DOCUMENT doc = FPDF_LoadDocument("encrypted.pdf", "password123");
```

自定义文件读取器（适用于自定义存储）：
```c
// 实现 FPDF_FILEACCESS 结构体
FPDF_FILEACCESS file_access;
file_access.m_FileLen = file_length;
file_access.m_GetBlock = MyGetBlockCallback;  // 自定义读取函数
file_access.m_Param = my_file_handle;
FPDF_DOCUMENT doc = FPDF_LoadCustomDocument(&file_access, NULL);
```

#### 步骤 3：获取文档信息

```c
int page_count = FPDF_GetPageCount(doc);
int version = 0;
FPDF_GetFileVersion(doc, &version);  // version = 17 表示 1.7
unsigned long permissions = FPDF_GetDocPermissions(doc);

// 元数据（需要分配缓冲区）
char title[256];
unsigned long len = FPDF_GetMetaText(doc, "Title", title, sizeof(title));
// title 中是 UTF-16LE 编码的字符串
```

#### 步骤 4：加载页面

```c
FPDF_PAGE page = FPDF_LoadPage(doc, 0);  // 加载第一页
if (!page) {
    // 页面加载失败
}
```

#### 步骤 5：获取页面尺寸

```c
double width = FPDF_GetPageWidth(page);
double height = FPDF_GetPageHeight(page);
// 或使用 float 版本
float width_f = FPDF_GetPageWidthF(page);
```

#### 步骤 6：创建位图并渲染

```c
// 创建位图（72 DPI 下 1 点 = 1 像素）
FPDF_BITMAP bitmap = FPDFBitmap_Create((int)width, (int)height, 0);

// 填充白色背景
FPDFBitmap_FillRect(bitmap, 0, 0, (int)width, (int)height, 0xFFFFFFFF);

// 渲染页面
FPDF_RenderPageBitmap(bitmap, page, 0, 0, (int)width, (int)height, 0, 0);

// 获取像素数据
unsigned char* buffer = (unsigned char*)FPDFBitmap_GetBuffer(bitmap);
int stride = FPDFBitmap_GetStride(bitmap);
// buffer 中为 BGRA 格式（alpha 通道取决于创建参数）
```

**高清渲染（指定 DPI）**:
```c
int dpi = 300;
float scale = dpi / 72.0f;
int render_width = (int)(width * scale);
int render_height = (int)(height * scale);

FPDF_BITMAP hd_bitmap = FPDFBitmap_Create(render_width, render_height, 0);
FPDF_RenderPageBitmap(hd_bitmap, page, 0, 0, render_width, render_height, 0, 0);
```

#### 步骤 7：保存位图为 PPM 文件

```c
FILE* fp = fopen("output.ppm", "wb");
fprintf(fp, "P6\n%d %d\n255\n", render_width, render_height);
for (int y = 0; y < render_height; y++) {
    unsigned char* row = buffer + y * stride;
    for (int x = 0; x < render_width; x++) {
        fputc(row[x * 4 + 2], fp);  // R
        fputc(row[x * 4 + 1], fp);  // G
        fputc(row[x * 4 + 0], fp);  // B
    }
}
fclose(fp);
```

#### 步骤 8：释放资源（顺序很重要）

```c
FPDFBitmap_Destroy(bitmap);
FPDF_ClosePage(page);
FPDF_CloseDocument(doc);
FPDF_DestroyLibrary();
```

### 完整示例代码

```c
#include <fpdfview.h>

int main() {
    // 1. 初始化
    FPDF_InitLibrary();
    
    // 2. 加载
    FPDF_DOCUMENT doc = FPDF_LoadDocument("test.pdf", NULL);
    if (!doc) {
        printf("加载失败: %lu\n", FPDF_GetLastError());
        FPDF_DestroyLibrary();
        return 1;
    }
    
    // 3. 遍历每一页
    int pages = FPDF_GetPageCount(doc);
    for (int i = 0; i < pages; i++) {
        FPDF_PAGE page = FPDF_LoadPage(doc, i);
        if (!page) continue;
        
        double w = FPDF_GetPageWidth(page);
        double h = FPDF_GetPageHeight(page);
        
        FPDF_BITMAP bitmap = FPDFBitmap_Create((int)w, (int)h, 0);
        FPDFBitmap_FillRect(bitmap, 0, 0, (int)w, (int)h, 0xFFFFFFFF);
        FPDF_RenderPageBitmap(bitmap, page, 0, 0, (int)w, (int)h, 0, 0);
        
        char filename[64];
        sprintf(filename, "page_%d.ppm", i);
        // ... 保存 PPM ...
        
        FPDFBitmap_Destroy(bitmap);
        FPDF_ClosePage(page);
    }
    
    FPDF_CloseDocument(doc);
    FPDF_DestroyLibrary();
    return 0;
}
```

### C++ 封装库版本

```cpp
#include <pdfium/pdfium.h>

int main() {
    pdfium::Library lib;
    
    auto doc = pdfium::Document::LoadFromFile("test.pdf");
    if (!doc) return 1;
    
    for (int i = 0; i < doc->GetPageCount(); i++) {
        auto page = doc->LoadPage(i);
        if (!page) continue;
        
        int w = (int)page->GetWidth();
        int h = (int)page->GetHeight();
        
        pdfium::Bitmap bmp(w, h);
        page->Render(bmp, 0, 0, w, h);
        
        // bmp.GetBuffer() 可直接获取像素数据
    }
    return 0;
}
```

---

## 2. 文本提取流程

### 流程图

```
加载页面 → 创建 TextPage → 获取字符数 → 按需提取文本
                               ├── 提取全部文本 → 转为 UTF-8
                               ├── 按矩形区域提取
                               ├── 逐字符遍历（获取位置/字体信息）
                               └── 获取文本矩形
```

### 详细步骤

#### 步骤 1-2：加载页面和文本信息

```c
FPDF_PAGE page = FPDF_LoadPage(doc, 0);
FPDF_TEXTPAGE text_page = FPDFText_LoadPage(page);
```

#### 步骤 3：提取全部文本

```c
int char_count = FPDFText_CountChars(text_page);

// 分配缓冲区（UTF-16LE，每个字符 2 字节）
int buf_len = char_count * 2 + 2;
unsigned short* buffer = (unsigned short*)malloc(buf_len);
int actual_len = FPDFText_GetText(text_page, 0, char_count, buffer);
// actual_len 包含终止 NULL，buffer 中为 UTF-16LE 编码
```

#### 步骤 4：提取指定区域文本

```c
// 提取位置 (50, 100) 到 (200, 300) 矩形内的文本
unsigned short region_buf[1024];
int region_len = FPDFText_GetBoundedText(text_page, 50, 100, 200, 300,
                                          region_buf, 1024);
```

#### 步骤 5：逐字符遍历（获取详细信息）

```c
for (int i = 0; i < char_count; i++) {
    unsigned int unicode = FPDFText_GetUnicode(text_page, i);
    double font_size = FPDFText_GetFontSize(text_page, i);
    
    double left, right, bottom, top;
    FPDFText_GetCharBox(text_page, i, &left, &right, &bottom, &top);
    
    double origin_x, origin_y;
    FPDFText_GetCharOrigin(text_page, i, &origin_x, &origin_y);
    
    printf("Char %d: U+%04X, size=%.1f, box=(%.0f,%.0f,%.0f,%.0f)\n",
           i, unicode, font_size, left, bottom, right, top);
}
```

#### 步骤 6：获取文本矩形（用于高亮显示）

```c
int rect_count = FPDFText_CountRects(text_page, 0, char_count);
for (int r = 0; r < rect_count; r++) {
    double left, top, right, bottom;
    FPDFText_GetRect(text_page, r, &left, &top, &right, &bottom);
    // 此矩形可用于在页面上绘制选择区域
}
```

#### 步骤 7：查找特定位置的字符

```c
int char_index = FPDFText_GetCharIndexAtPos(text_page, x, y, 2.0, 2.0);
if (char_index >= 0) {
    // 用户点击了此字符
}
```

### C++ 封装库版本

```cpp
auto page = doc->LoadPage(0);
auto text = page->LoadTextPage();

// 全文提取
auto content = text->GetText(0, text->CharCount());

// 区域提取
auto region = text->GetBoundedText(50, 100, 200, 300);

// 逐字符
for (int i = 0; i < text->CharCount(); i++) {
    auto ch = text->GetUnicode(i);
    double l, r, b, t;
    text->GetCharBox(i, &l, &r, &b, &t);
}
```

---

## 3. 文本搜索流程

### 流程图

```
加载 TextPage → 创建搜索上下文 → 循环 FindNext/FindPrev → 获取匹配位置 → 处理结果 → 关闭搜索
```

### 详细步骤

```c
// 1. 加载文本
FPDF_TEXTPAGE text_page = FPDFText_LoadPage(page);

// 2. 创建搜索上下文
// 参数: text_page, 搜索词, 标志, 起始位置(0 表示从头)
FPDF_SCHHANDLE search = FPDFText_FindStart(
    text_page, L"PDF",  // 搜索词（宽字符串）
    FPDF_MATCHCASE,      // 搜索标志
    0);                  // 起始索引

// 3. 查找所有匹配
int match_count = 0;
while (FPDFText_FindNext(search)) {
    int index = FPDFText_GetSchResultIndex(search);
    int count = FPDFText_GetSchCount(search);
    
    // 提取匹配文本
    unsigned short match_buf[256];
    FPDFText_GetText(text_page, index, count, match_buf);
    
    // 获取匹配位置的矩形
    double left, top, right, bottom;
    FPDFText_GetRect(text_page, index, &left, &top, &right, &bottom);
    
    printf("匹配 %d: 位置 %d, 长度 %d, 矩形=(%.0f,%.0f,%.0f,%.0f)\n",
           ++match_count, index, count, left, top, right, bottom);
}

// 4. 关闭搜索
FPDFText_FindClose(search);
```

**搜索标志组合**:
```c
// 不区分大小写（默认）
FPDF_SCHHANDLE s1 = FPDFText_FindStart(tp, L"pdf", 0, 0);

// 区分大小写
FPDF_SCHHANDLE s2 = FPDFText_FindStart(tp, L"PDF", FPDF_MATCHCASE, 0);

// 全字匹配
FPDF_SCHHANDLE s3 = FPDFText_FindStart(tp, L"PDF", FPDF_MATCHCASE | FPDF_MATCHWHOLEWORD, 0);
```

### C++ 封装库版本

```cpp
auto page = doc->LoadPage(0);
auto text = page->LoadTextPage();

auto find = pdfium::TextFind(
    FPDFText_FindStart(text->Handle(), L"search_term", 0, 0));

while (find.FindNext()) {
    int idx = find.GetResultIndex();
    int len = find.GetResultCount();
    auto match = text->GetText(idx, len);
    std::wcout << L"Found: " << match << std::endl;
}
```

---

## 4. 书签导航流程

### 流程图

```
获取根书签（NULL）→ 获取第一个子书签 → 递归遍历 → 获取标题/目标/动作
                               ├── 获取目标页码
                               ├── 获取动作类型和参数
                               └── 递归子书签
```

### 详细步骤

```c
// 递归打印书签
void PrintBookmarks(FPDF_DOCUMENT doc, FPDF_BOOKMARK bookmark, int depth) {
    FPDF_BOOKMARK child = FPDFBookmark_GetFirstChild(doc, bookmark);
    while (child) {
        // 缩进
        for (int i = 0; i < depth; i++) printf("  ");
        
        // 获取标题
        unsigned short title[256];
        FPDFBookmark_GetTitle(child, title, sizeof(title));
        printf("📑 %ls", title);
        
        // 获取目标（跳转到文档内位置）
        FPDF_DEST dest = FPDFBookmark_GetDest(doc, child);
        if (dest) {
            int page_index = FPDFDest_GetDestPageIndex(doc, dest);
            printf(" → 第 %d 页", page_index + 1);
            
            // 获取视图类型（可选）
            unsigned long num_params;
            FS_FLOAT params[4];
            unsigned long view_mode = FPDFDest_GetView(dest, &num_params, params);
            if (view_mode == PDFDEST_VIEW_XYZ) {
                printf(" (位置: %.0f, %.0f, 缩放: %.2f)",
                       params[0], params[1], params[2]);
            }
        }
        
        // 获取动作（跳转到外部文档或 URL）
        FPDF_ACTION action = FPDFBookmark_GetAction(child);
        if (action) {
            unsigned long type = FPDFAction_GetType(action);
            if (type == PDFACTION_URI) {
                char uri[512];
                FPDFAction_GetURIPath(doc, action, uri, sizeof(uri));
                printf(" 🔗 %s", uri);
            } else if (type == PDFACTION_GOTO) {
                FPDF_DEST action_dest = FPDFAction_GetDest(doc, action);
                if (action_dest) {
                    int p = FPDFDest_GetDestPageIndex(doc, action_dest);
                    printf(" → 第 %d 页", p + 1);
                }
            }
        }
        
        printf("\n");
        
        // 递归子书签
        PrintBookmarks(doc, child, depth + 1);
        
        // 下一个兄弟书签
        child = FPDFBookmark_GetNextSibling(doc, child);
    }
}

// 调用
PrintBookmarks(doc, NULL, 0);
```

### 查找特定书签

```c
FPDF_BOOKMARK bm = FPDFBookmark_Find(doc, L"第一章");
if (bm) {
    FPDF_DEST dest = FPDFBookmark_GetDest(doc, bm);
    if (dest) {
        int page_num = FPDFDest_GetDestPageIndex(doc, dest);
        printf("'第一章' 在第 %d 页\n", page_num + 1);
    }
}
```

---

## 5. 注释操作流程

### 5.1 列举注释

```c
int annot_count = FPDFPage_GetAnnotCount(page);
printf("页面有 %d 个注释\n", annot_count);

for (int i = 0; i < annot_count; i++) {
    FPDF_ANNOTATION annot = FPDFPage_GetAnnot(page, i);
    if (!annot) continue;
    
    // 获取子类型
    FPDF_ANNOTATION_SUBTYPE subtype = FPDFAnnot_GetSubtype(annot);
    
    // 获取矩形位置
    FS_RECTF rect;
    FPDFAnnot_GetRect(annot, &rect);
    
    // 获取颜色
    unsigned int R, G, B, A;
    if (FPDFAnnot_GetColor(annot, FPDFANNOT_COLORTYPE_Color, &R, &G, &B, &A)) {
        printf("  颜色: rgba(%d,%d,%d,%d)\n", R, G, B, A);
    }
    
    // 获取标志
    int flags = FPDFAnnot_GetFlags(annot);
    
    printf("注释 %d: 类型=%d, 矩形=(%.0f,%.0f,%.0f,%.0f), 标志=0x%X\n",
           i, subtype, rect.left, rect.top, rect.right, rect.bottom, flags);
    
    // 对于文本标记注释（高亮、下划线等），获取附着点
    if (subtype >= FPDF_ANNOT_HIGHLIGHT && subtype <= FPDF_ANNOT_STRIKEOUT) {
        size_t quads = FPDFAnnot_CountAttachmentPoints(annot);
        for (size_t q = 0; q < quads; q++) {
            FS_QUADPOINTSF qp;
            FPDFAnnot_GetAttachmentPoints(annot, q, &qp);
            // qp 包含四边形的四个顶点
        }
    }
    
    FPDFPage_CloseAnnot(annot);
}
```

### 5.2 创建注释

```c
// 创建高亮注释
FPDF_ANNOTATION highlight = FPDFPage_CreateAnnot(page, FPDF_ANNOT_HIGHLIGHT);
if (highlight) {
    // 设置矩形
    FS_RECTF rect = {100, 200, 300, 400};
    FPDFAnnot_SetRect(highlight, &rect);
    
    // 设置颜色（黄色）
    FPDFAnnot_SetColor(highlight, FPDFANNOT_COLORTYPE_Color, 255, 255, 0, 128);
    
    // 设置附着点（被高亮的文本区域）
    FS_QUADPOINTSF qp = {
        100, 400,   // 左上
        300, 400,   // 右上
        100, 350,   // 左下
        300, 350    // 右下
    };
    FPDFAnnot_SetAttachmentPoints(highlight, 0, &qp);
    
    // 设置文本内容（鼠标悬停时显示）
    FPDFAnnot_SetStringValue(highlight, "Contents", L"这是高亮注释");
    
    // 设置外观
    FPDFAnnot_SetAP(highlight, FPDF_ANNOT_APPEARANCEMODE_NORMAL, L"/H");
    
    FPDFPage_CloseAnnot(highlight);
}
```

### 5.3 表单字段注释

```c
// 获取点下的表单字段
FS_POINTF pt = {150, 250};
FPDF_ANNOTATION field_annot = FPDFAnnot_GetFormFieldAtPoint(handle, page, &pt);
if (field_annot) {
    // 获取字段信息
    int field_type = FPDFAnnot_GetFormFieldType(handle, field_annot);
    // FPDF_FORMFIELD_PUSHBUTTON = 1
    // FPDF_FORMFIELD_CHECKBOX   = 2
    // FPDF_FORMFIELD_RADIOBUTTON= 3
    // FPDF_FORMFIELD_COMBOBOX   = 4
    // FPDF_FORMFIELD_LISTBOX    = 5
    // FPDF_FORMFIELD_TEXTFIELD  = 6
    // FPDF_FORMFIELD_SIGNATURE  = 7
    
    // 获取字段值
    unsigned short value[256];
    FPDFAnnot_GetFormFieldValue(handle, field_annot, value, sizeof(value));
    
    // 复选框是否选中
    FPDF_BOOL checked = FPDFAnnot_IsChecked(handle, field_annot);
    
    FPDFPage_CloseAnnot(field_annot);
}
```

---

## 6. 表单填充流程

### 流程图

```
初始化库 → 加载文档 → 创建表单环境（FORM_Init）→ 加载页面 → OnAfterLoadPage
→ 处理用户交互（鼠标/键盘事件）→ 渲染表单（FFLDraw）→ OnBeforeClosePage → 退出表单环境
```

### 完整步骤

```c
// 1. 初始化库
FPDF_InitLibrary();

// 2. 加载文档
FPDF_DOCUMENT doc = FPDF_LoadDocument("form.pdf", NULL);

// 3. 实现 FORM 回调接口
int form_version = FPDF_GetFormType(doc);
if (form_version == FORMTYPE_ACRO_FORM) {
    // 4. 填写 FPDF_FORMFILLINFO 结构体
    FPDF_FORMFILLINFO form_info;
    memset(&form_info, 0, sizeof(form_info));
    form_info.version = 1;
    // 实现必要回调（FFI_* 函数）
    
    // 5. 创建表单环境
    FPDF_FORMHANDLE form = FPDFDOC_InitFormFillEnvironment(doc, &form_info);
    
    // 6. 加载页面
    FPDF_PAGE page = FPDF_LoadPage(doc, 0);
    
    // 7. 通知表单页面已加载
    FORM_OnAfterLoadPage(page, form);
    
    // 8. 处理鼠标事件
    // 鼠标移动
    FORM_OnMouseMove(form, page, 0, x, y);
    // 左键按下（触发焦点）
    FORM_OnLButtonDown(form, page, 0, x, y);
    // 左键释放
    FORM_OnLButtonUp(form, page, 0, x, y);
    
    // 9. 键盘输入（当字段获得焦点后）
    FORM_OnKeyDown(form, page, FWL_VKEY_A, 0);
    FORM_OnChar(form, page, 'A', 0);
    
    // 10. 渲染（包含表单字段）
    FPDF_BITMAP bitmap = FPDFBitmap_Create(width, height, 0);
    FPDF_RenderPageBitmap(bitmap, page, 0, 0, width, height, 0, 0);
    FPDF_FFLDraw(form, bitmap, page, 0, 0, width, height, 0, 0);  // 第二次绘制表单
    
    // 11. 撤销/重做
    if (FORM_CanUndo(form, page)) {
        FORM_Undo(form, page);
    }
    
    // 12. 关闭页面
    FORM_OnBeforeClosePage(page, form);
    FPDF_ClosePage(page);
    
    // 13. 退出表单环境
    FPDFDOC_ExitFormFillEnvironment(form);
}

FPDF_CloseDocument(doc);
FPDF_DestroyLibrary();
```

### FPDF_FORMFILLINFO 重要回调

```c
// 必填回调（最低版本 1）
form_info.version = 1;
form_info.FFI_Invalidate = MyInvalidate;     // 页面刷新请求
form_info.FFI_OutputSelectedRect = MyOutputRect; // 选中文本区域
form_info.FFI_SetCursor = MySetCursor;         // 改变光标形状
form_info.FFI_SetTimer = MySetTimer;           // 设置定时器
form_info.FFI_KillTimer = MyKillTimer;         // 清除定时器

// 实验性回调（版本 2+）
form_info.FFI_GetCurrentPage = MyGetCurrentPage; // 获取当前页面
```

---

## 7. 创建和编辑 PDF 流程

### 7.1 创建新文档并添加内容

```
创建文档 → 创建页面 → 插入对象（文本/路径/图像）→ 生成内容 → 保存
```

**详细步骤**:

```c
// 1. 创建空文档
FPDF_DOCUMENT doc = FPDF_CreateNewDocument();

// 2. 创建页面
// 参数: 文档, 索引, 宽度(点), 高度(点)
FPDF_PAGE page = FPDFPage_New(doc, 0, 612, 792);  // A4 页面

// 3. 添加文本
FPDF_PAGEOBJECT text_obj = FPDFPageObj_NewTextObj(doc, "Helvetica", 24.0f);
FPDFText_SetText(text_obj, L"Hello, PDFium!");
// 设置位置（通过变换矩阵）
FPDFPageObj_Transform(text_obj, 1, 0, 0, 1, 100, 700);
FPDFPage_InsertObject(page, text_obj);

// 4. 添加矩形路径
FPDF_PAGEOBJECT rect = FPDFPageObj_CreateNewRect(100, 600, 200, 100);
FPDFPageObj_SetFillColor(rect, 0, 128, 255, 255);  // 蓝色填充
FPDFPageObj_SetStrokeColor(rect, 0, 0, 0, 255);     // 黑色描边
FPDFPath_SetDrawMode(rect, FPDF_FILLMODE_ALTERNATE, TRUE);
FPDFPage_InsertObject(page, rect);

// 5. 添加直线路径
FPDF_PAGEOBJECT path = FPDFPageObj_CreateNewPath(100, 500);
FPDFPath_LineTo(path, 400, 550);
FPDFPath_LineTo(path, 400, 450);
FPDFPath_Close(path);
FPDFPageObj_SetStrokeColor(path, 255, 0, 0, 255);
FPDFPageObj_SetStrokeWidth(path, 3.0f);
FPDFPath_SetDrawMode(path, 0, TRUE);  // 仅描边
FPDFPage_InsertObject(page, path);

// 6. 生成页面内容（必须调用）
FPDFPage_GenerateContent(page);

// 7. 保存
struct FPDF_FILEWRITE writer;
writer.WriteBlock = MyWriteBlock;
FPDF_SaveAsCopy(doc, &writer, FPDF_NO_INCREMENTAL);

// 8. 释放
FPDF_ClosePage(page);
FPDF_CloseDocument(doc);
```

### 7.2 插入图像

```c
// 1. 创建图像对象
FPDF_PAGEOBJECT image = FPDFPageObj_NewImageObj(doc);

// 2. 从文件加载 JPEG
FPDF_FILEACCESS file_acc;
file_acc.m_FileLen = jpeg_file_size;
file_acc.m_GetBlock = MyGetBlock;
file_acc.m_Param = jpeg_handle;
FPDFImageObj_LoadJpegFile(&page, 1, image, &file_acc);

// 3. 设置位置
FPDFImageObj_SetMatrix(image, scale_x, 0, 0, scale_y, pos_x, pos_y);

// 4. 插入页面
FPDFPage_InsertObject(page, image);
FPDFPage_GenerateContent(page);
```

### 7.3 编辑现有文档

```c
// 1. 加载现有文档
FPDF_DOCUMENT doc = FPDF_LoadDocument("existing.pdf", NULL);

// 2. 加载要编辑的页面
FPDF_PAGE page = FPDF_LoadPage(doc, 0);

// 3. 获取现有对象
int obj_count = FPDFPage_CountObjects(page);
for (int i = 0; i < obj_count; i++) {
    FPDF_PAGEOBJECT obj = FPDFPage_GetObject(page, i);
    int type = FPDFPageObj_GetType(obj);
    // FPDF_PAGEOBJ_UNKNOWN=0, TEXT=1, PATH=2, IMAGE=3, SHADING=4, FORM=5
}

// 4. 变换现有对象
FPDFPageObj_Transform(existing_obj, 1.5, 0, 0, 1.5, 0, 0);  // 放大 1.5 倍

// 5. 删除对象
FPDFPage_RemoveObject(page, obj_to_remove);
FPDFPageObj_Destroy(obj_to_remove);

// 6. 调整页面大小
FPDFPage_SetMediaBox(page, 0, 0, 800, 1000);

// 7. 再生内容
FPDFPage_GenerateContent(page);

FPDF_ClosePage(page);
FPDF_CloseDocument(doc);
```

---

## 8. 页面变换流程

### 8.1 获取和设置页面盒子

```c
// 获取
float ml, mb, mr, mt;  // MediaBox
float cl, cb, cr, ct;  // CropBox
FPDFPage_GetMediaBox(page, &ml, &mb, &mr, &mt);
FPDFPage_GetCropBox(page, &cl, &cb, &cr, &ct);

// 设置（可以裁切页面）
FPDFPage_SetCropBox(page, ml + 50, mb + 50, mr - 50, mt - 50);
```

### 8.2 页面矩阵变换

```c
// 旋转 90 度
FS_MATRIX matrix = {0, -1, 1, 0, 0, page_height};
FPDFPage_TransFormWithClip(page, &matrix, NULL);

// 缩放和平移
FS_MATRIX scale_matrix = {0.5, 0, 0, 0.5, 0, 0};  // 缩小到 50%
FPDFPage_TransFormWithClip(page, &scale_matrix, NULL);
```

### 8.3 裁剪路径

```c
// 创建圆形裁剪区域
FPDF_CLIPPATH clip = FPDF_CreateClipPath(100, 100, 300, 300);

// 应用到页面
FPDFPage_InsertClipPath(page, clip);
FPDF_DestroyClipPath(clip);
```

### 8.4 N-Up 拼版（多页合为一张）

```c
// 将 4 页合为一张（2×2 网格）
FPDF_DOCUMENT nup_doc = FPDF_ImportNPagesToOne(doc, 612, 792, 2, 2);
```

---

## 9. 数字签名验签流程

### 获取签名信息

```c
int sig_count = FPDF_GetSignatureCount(doc);
printf("文档有 %d 个数字签名\n", sig_count);

for (int i = 0; i < sig_count; i++) {
    FPDF_SIGNATURE sig = FPDF_GetSignatureObject(doc, i);
    
    // 签名内容（PKCS7 格式）
    unsigned char contents[4096];
    unsigned long len = FPDFSignatureObj_GetContents(sig, contents, sizeof(contents));
    
    // 字节范围（签名覆盖的字节范围）
    int byte_range[8];
    unsigned long range_count = FPDFSignatureObj_GetByteRange(sig, byte_range, sizeof(byte_range));
    // 每两个值一组（起始位置, 长度）
    // 例如: byte_range[0]=0, byte_range[1]=1000 → 前 1000 字节
    //       byte_range[2]=2000, byte_range[3]=500 → 1000-2000 外的区间
    
    // 子过滤器
    char subfilter[64];
    FPDFSignatureObj_GetSubFilter(sig, subfilter, sizeof(subfilter));
    // "adbe.pkcs7.detached" — 分离式 PKCS7 签名
    // "adbe.pkcs7.sha1" — PKCS7 SHA1 签名
    
    // 签名原因
    unsigned short reason[256];
    FPDFSignatureObj_GetReason(sig, reason, sizeof(reason));
    
    // 签名时间
    char time_str[64];
    FPDFSignatureObj_GetTime(sig, time_str, sizeof(time_str));
    
    // 文档 MDP 权限
    unsigned int mdp = FPDFSignatureObj_GetDocMDPPermission(sig);
    
    printf("签名 %d: 过滤器=%s, 原因=%ls, 时间=%s, MDP=%u\n",
           i, subfilter, reason, time_str, mdp);
}
```

---

## 10. 渐进加载流程（网络 PDF）

适用于从网络流式加载 PDF 文件，特别是线性化（优化网络加载）的 PDF。

```
创建 Avail 对象 → 循环检查文档可用性 → 获取文档 → 循环检查页面可用性 → 加载页面
```

```c
// 1. 准备文件读取回调
FX_FILEAVAIL file_avail;
file_avail.version = 1;
file_avail.IsDataAvail = MyIsDataAvail;  // 检查范围是否已下载

FPDF_FILEACCESS file_access;
file_access.m_FileLen = content_length;
file_access.m_GetBlock = MyGetBlock;
file_access.m_Param = &my_download_context;

FX_DOWNLOADHINTS hints;
hints.version = 1;
hints.AddSegment = MyAddSegment;  // 提示需要下载更多数据

// 2. 创建 Avail 对象
FPDF_AVAIL avail = FPDFAvail_Create(&file_avail, &file_access);

// 3. 等待文档可用
while (!FPDFAvail_IsDocAvail(avail, &hints)) {
    // 下载更多数据...
    DownloadMoreData();
}

// 4. 获取文档
FPDF_DOCUMENT doc = FPDFAvail_GetDocument(avail, NULL);
if (!doc) {
    // 错误处理
    FPDFAvail_Destroy(avail);
    return;
}

// 5. 检查线性化
int is_linearized = FPDFAvail_IsLinearized(avail);

// 6. 获取第一可显示页（线性化 PDF 会优化此页的显示速度）
int first_page = FPDFAvail_GetFirstPageNum(doc);

// 7. 等待第一页可用并显示
while (!FPDFAvail_IsPageAvail(avail, first_page, &hints)) {
    DownloadMoreData();
}

// 8. 加载并渲染页面
FPDF_PAGE page = FPDF_LoadPage(doc, first_page);
// 渲染...

// 9. 后续页面可在后台逐步下载
for (int i = 0; i < FPDF_GetPageCount(doc); i++) {
    while (!FPDFAvail_IsPageAvail(avail, i, &hints)) {
        DownloadMoreData();
    }
    // 加载和渲染页面
}

// 10. 销毁
FPDFAvail_Destroy(avail);
```

---

## 11. 结构树/无障碍流程

### 获取文档结构树（标签 PDF）

```c
void PrintStructure(FPDF_STRUCTELEMENT elem, int depth);

// 1. 获取结构树
FPDF_STRUCTTREE tree = FPDF_StructTree_GetForPage(page);
if (!tree) {
    printf("此页面没有结构树（不是标签 PDF）\n");
    return;
}

// 2. 检查是否为标签 PDF
FPDF_BOOL is_tagged = FPDFCatalog_IsTagged(doc);
printf("是否为标签 PDF: %s\n", is_tagged ? "是" : "否");

// 3. 遍历结构树
int children = FPDF_StructTree_CountChildren(tree);
for (int i = 0; i < children; i++) {
    FPDF_STRUCTELEMENT elem = FPDF_StructTree_GetChildAtIndex(tree, i);
    PrintStructure(elem, 0);
}

FPDF_StructTree_Close(tree);

// 递归打印结构元素
void PrintStructure(FPDF_STRUCTELEMENT elem, int depth) {
    // 缩进
    for (int i = 0; i < depth; i++) printf("  ");
    
    // 获取类型（如 Document, H1, P, Figure, Table 等）
    unsigned short type[256];
    FPDF_StructElement_GetType(elem, type, sizeof(type));
    
    // 获取标题
    unsigned short title[256];
    FPDF_StructElement_GetTitle(elem, title, sizeof(title));
    
    // 获取替代文本（屏幕阅读器使用）
    unsigned short alt[512];
    FPDF_StructElement_GetAltText(elem, alt, sizeof(alt));
    
    // 获取语言
    unsigned short lang[64];
    FPDF_StructElement_GetLang(elem, lang, sizeof(lang));
    
    printf("[%ls]", type);
    if (title[0]) printf(" \"%ls\"", title);
    if (alt[0]) printf(" alt=\"%ls\"", alt);
    if (lang[0]) printf(" lang=%ls", lang);
    printf("\n");
    
    // 递归子元素
    int child_count = FPDF_StructElement_CountChildren(elem);
    for (int i = 0; i < child_count; i++) {
        FPDF_STRUCTELEMENT child = FPDF_StructElement_GetChildAtIndex(elem, i);
        PrintStructure(child, depth + 1);
    }
}
```

---

## 12. 附件操作流程

### 列举附件

```c
int count = FPDFDoc_GetAttachmentCount(doc);
printf("文档有 %d 个附件\n", count);

for (int i = 0; i < count; i++) {
    FPDF_ATTACHMENT att = FPDFDoc_GetAttachment(doc, i);
    
    // 名称
    unsigned short name[256];
    FPDFAttachment_GetName(att, name, sizeof(name));
    
    // 文件内容
    unsigned long content_len = FPDFAttachment_GetFile(att, NULL, 0, NULL);
    unsigned char* content = (unsigned char*)malloc(content_len);
    FPDFAttachment_GetFile(att, content, content_len, &content_len);
    
    // 描述
    unsigned short desc[256];
    FPDFAttachment_GetStringValue(att, "Description", desc, sizeof(desc));
    
    printf("附件 %d: %ls (%lu 字节)\n", i, name, content_len);
    
    free(content);
}
```

### 添加附件

```c
FPDF_ATTACHMENT att = FPDFDoc_AddAttachment(doc, L"readme.txt");
FPDFAttachment_SetFile(att, doc, file_data, file_data_len);
FPDFAttachment_SetStringValue(att, "Description", L"说明文档");
```

### 删除附件

```c
FPDFDoc_DeleteAttachment(doc, 0);  // 删除第一个附件
```

---

## 13. 保存文档流程

### 13.1 实现 FPDF_FILEWRITE 回调

```c
// 写入到文件
struct FileWrite {
    FPDF_FILEWRITE writer;
    FILE* fp;
};

unsigned long MyWriteBlock(FPDF_FILEWRITE* pw, const void* data, unsigned long size) {
    FileWrite* fw = (FileWrite*)pw;
    return (unsigned long)fwrite(data, 1, size, fw->fp);
}

// 使用
FILE* fp = fopen("output.pdf", "wb");
FileWrite fw;
fw.writer.WriteBlock = MyWriteBlock;
fw.fp = fp;

FPDF_SaveAsCopy(doc, &fw.writer, FPDF_NO_INCREMENTAL);
fclose(fp);
```

### 13.2 保存到内存缓冲区

```c
struct MemWrite {
    FPDF_FILEWRITE writer;
    unsigned char* buffer;
    unsigned long size;
    unsigned long capacity;
};

unsigned long MemWriteBlock(FPDF_FILEWRITE* pw, const void* data, unsigned long size) {
    MemWrite* mw = (MemWrite*)pw;
    if (mw->size + size > mw->capacity) {
        mw->capacity = mw->size + size + 65536;
        mw->buffer = (unsigned char*)realloc(mw->buffer, mw->capacity);
    }
    memcpy(mw->buffer + mw->size, data, size);
    mw->size += size;
    return size;
}
```

### 13.3 保存选项

```c
// 增量保存（快速，但文件会变大）
FPDF_SaveAsCopy(doc, &writer, FPDF_INCREMENTAL);

// 全量保存（清理文件结构）
FPDF_SaveAsCopy(doc, &writer, FPDF_NO_INCREMENTAL);

// 去除安全设置后保存
FPDF_SaveAsCopy(doc, &writer, FPDF_NO_INCREMENTAL | FPDF_REMOVE_SECURITY);

// 指定版本保存
FPDF_SaveWithVersion(doc, &writer, FPDF_NO_INCREMENTAL, 17);  // 保存为 PDF 1.7
```

---

## 14. 完整错误处理流程

### 错误码映射

```c
const char* GetErrorString(unsigned long err) {
    switch (err) {
        case FPDF_ERR_SUCCESS:  return "成功";
        case FPDF_ERR_UNKNOWN:  return "未知错误";
        case FPDF_ERR_FILE:     return "文件未找到或无法打开";
        case FPDF_ERR_FORMAT:   return "PDF 格式错误";
        case FPDF_ERR_PASSWORD: return "密码错误";
        case FPDF_ERR_SECURITY: return "安全机制阻止";
        case FPDF_ERR_PAGE:     return "页面不存在";
        default:                return "未知错误码";
    }
}
```

### 完整的健壮加载模式

```c
FPDF_DOCUMENT SafeLoadDocument(const char* path, const char* password) {
    // 1. 尝试加载
    FPDF_DOCUMENT doc = FPDF_LoadDocument(path, password);
    if (doc) return doc;
    
    // 2. 获取错误
    unsigned long err = FPDF_GetLastError();
    switch (err) {
        case FPDF_ERR_PASSWORD:
            // 密码错误——可尝试其他密码或提示用户
            if (!password || strlen(password) == 0) {
                // 可能需要密码
                printf("文档需要密码\n");
            } else {
                printf("密码错误\n");
            }
            break;
            
        case FPDF_ERR_FILE:
            // 文件路径问题
            printf("文件不存在或无法访问: %s\n", path);
            break;
            
        case FPDF_ERR_FORMAT:
            // 尝试作为内存流加载（某些非标准扩展名文件）
            {
                FILE* fp = fopen(path, "rb");
                if (fp) {
                    fseek(fp, 0, SEEK_END);
                    long size = ftell(fp);
                    fseek(fp, 0, SEEK_SET);
                    unsigned char* data = (unsigned char*)malloc(size);
                    fread(data, 1, size, fp);
                    fclose(fp);
                    
                    doc = FPDF_LoadMemDocument64(data, size, password);
                    free(data);
                    if (doc) return doc;
                }
            }
            break;
    }
    
    return NULL;
}
```

### 资源释放保证

```c
// 确保异常路径也能正确释放
void ProcessPDF(const char* path) {
    FPDF_DOCUMENT doc = NULL;
    FPDF_PAGE page = NULL;
    FPDF_BITMAP bitmap = NULL;
    
    // 使用 goto 模式确保释放（或使用 C++ RAII）
    doc = FPDF_LoadDocument(path, NULL);
    if (!doc) goto cleanup;
    
    page = FPDF_LoadPage(doc, 0);
    if (!page) goto cleanup;
    
    bitmap = FPDFBitmap_Create(100, 100, 0);
    if (!bitmap) goto cleanup;
    
    // ... 处理 ...
    
cleanup:
    if (bitmap) FPDFBitmap_Destroy(bitmap);
    if (page) FPDF_ClosePage(page);
    if (doc) FPDF_CloseDocument(doc);
}
```

---

## 15. 多线程安全注意事项

### 核心规则

| 操作 | 线程安全 | 说明 |
|------|---------|------|
| `FPDF_InitLibrary` | ❌ 非线程安全 | 必须在主线程调用一次 |
| `FPDF_DestroyLibrary` | ❌ 非线程安全 | 必须在主线程调用一次 |
| `FPDF_LoadDocument` | ✅ 线程安全 | 可在不同线程加载不同文档 |
| `FPDF_CloseDocument` | ❌ | 必须在同一线程关闭 |
| `FPDF_LoadPage` | ✅ | 可在不同线程加载不同页 |
| `FPDF_RenderPageBitmap` | ✅ | 可在不同线程渲染不同页面 |
| `FPDFText_LoadPage` | ✅ | 文本提取可并行 |
| 所有 `FPDFAnnot_*` | ❌ | 注释操作应串行 |

### 线程池并行渲染

```c
typedef struct {
    FPDF_DOCUMENT doc;
    int page_index;
    const char* output_dir;
} RenderTask;

DWORD WINAPI RenderPageThread(LPVOID param) {
    RenderTask* task = (RenderTask*)param;
    
    // 每个线程独立加载页面
    FPDF_PAGE page = FPDF_LoadPage(task->doc, task->page_index);
    if (!page) return 1;
    
    double w = FPDF_GetPageWidth(page);
    double h = FPDF_GetPageHeight(page);
    
    FPDF_BITMAP bitmap = FPDFBitmap_Create((int)w, (int)h, 0);
    FPDF_RenderPageBitmap(bitmap, page, 0, 0, (int)w, (int)h, 0, 0);
    
    // 保存...
    char filename[256];
    sprintf(filename, "%s/page_%d.ppm", task->output_dir, task->page_index);
    SavePPM(bitmap, filename);
    
    FPDFBitmap_Destroy(bitmap);
    FPDF_ClosePage(page);
    
    return 0;
}

// 多线程渲染所有页面
void RenderAllPagesParallel(FPDF_DOCUMENT doc) {
    int pages = FPDF_GetPageCount(doc);
    HANDLE* threads = (HANDLE*)malloc(sizeof(HANDLE) * pages);
    RenderTask* tasks = (RenderTask*)malloc(sizeof(RenderTask) * pages);
    
    for (int i = 0; i < pages; i++) {
        tasks[i].doc = doc;
        tasks[i].page_index = i;
        tasks[i].output_dir = "./output";
        threads[i] = CreateThread(NULL, 0, RenderPageThread, &tasks[i], 0, NULL);
    }
    
    WaitForMultipleObjects(pages, threads, TRUE, INFINITE);
    
    for (int i = 0; i < pages; i++) CloseHandle(threads[i]);
    free(threads);
    free(tasks);
}
```

### 注意

- `FPDF_DOCUMENT` 句柄在单线程中加载后，可在多个线程中同时调用 `FPDF_LoadPage`
- 同一页面不能在多个线程中同时渲染
- 页面渲染完成后才能关闭文档

---

## 附录：典型应用场景代码速查

| 场景 | 核心 API 调用 |
|------|--------------|
| 初始化 | `FPDF_InitLibraryWithConfig` |
| 打开文件 | `FPDF_LoadDocument` |
| 打开内存 | `FPDF_LoadMemDocument64` |
| 带密码 | 传入 password 参数即可 |
| 页面总数 | `FPDF_GetPageCount` |
| 渲染到图 | `FPDFBitmap_Create` + `FPDF_RenderPageBitmap` |
| 提取文本 | `FPDFText_LoadPage` + `FPDFText_GetText` |
| 搜索文本 | `FPDFText_FindStart` + `FPDFText_FindNext` |
| 遍历书签 | `FPDFBookmark_GetFirstChild` + `GetNextSibling` |
| 读取注释 | `FPDFPage_GetAnnotCount` + `FPDFPage_GetAnnot` |
| 添加注释 | `FPDFPage_CreateAnnot` + 设置属性 |
| 表单操作 | `FPDFDOC_InitFormFillEnvironment` + 事件函数 |
| 创建页面 | `FPDFPage_New` + 对象操作 + `GenerateContent` |
| 保存 | `FPDF_SaveAsCopy` / `FPDF_SaveWithVersion` |
| 渐进加载 | `FPDFAvail_Create` + `IsDocAvail` + `IsPageAvail` |
| 缩略图 | `FPDFPage_GetThumbnailAsBitmap` |
| 签名验签 | `FPDF_GetSignatureObject` + `GetContents` |
| 无障碍 | `FPDF_StructTree_GetForPage` |
| 页面变换 | `FPDFPage_TransFormWithClip` |
| 导入页面 | `FPDF_ImportPages` |

---

> 本文档由 `modern-pdfium-cmake-starter` 项目维护。
> 最后更新: 2025-06
