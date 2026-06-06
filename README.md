# modern-pdfium

[English](#english) | [中文](#chinese)

---

## <a name="chinese"></a>中文介绍

**modern-pdfium** 是一个面向现代 C++（C++20）的 [PDFium](https://pdfium.googlesource.com/pdfium/) 封装库。它用 RAII 惯用法、强类型枚举和命名空间将 PDFium 的 C 语言 API 包装为易用且安全的 C++ 接口。

### 特性 / 已封装的模块

| 模块 | 头文件 | 说明 |
|------|--------|------|
| **核心** | `core.h` | 库的初始化/销毁（RAII）、错误码、沙箱策略、版本信息 |
| **文档** | `document.h` | 文档打开/关闭、元数据、页数、权限 |
| **页面** | `page.h` | 页面渲染、尺寸、旋转 |
| **位图** | `bitmap.h` | 位图创建与操作 |
| **文本** | `text.h` | 文本提取、搜索、Web 链接 |
| **注释** | `annotation.h` | 注释类型、属性、外观 |
| **书签** | `bookmark.h` | 书签遍历、标题、动作 |
| **附件** | `attachment.h` | 文件附件提取 |
| **签名** | `signature.h` | 数字签名信息 |
| **表单** | `form.h` | AcroForm / XFA 表单填充与事件 |
| **图像** | `image.h` | 图像对象提取与信息 |
| **字体** | `font.h` | 字体信息与操作 |
| **路径** | `path.h` | 路径对象、线段、裁剪路径 |
| **结构树** | `struct_tree.h` | 结构树、元素、属性（无障碍 / 标签 PDF） |
| **链接** | `link.h` | 文档链接与跳转目标 |
| **动作** | `action.h` | 动作类型与参数 |
| **保存** | `save.h` | 增量保存与完整保存 |
| **渐进式加载** | `dataavail.h` | 渐进式可用性查询与加载 |

### 构建

```bash
mkdir build && cd build
cmake .. -DPDFIUM_ROOT=/path/to/pdfium
cmake --build .
```

可选：启用示例构建：

```bash
cmake .. -DPDFIUM_ROOT=/path/to/pdfium -DBUILD_EXAMPLES=ON
```

### 依赖

- C++20 兼容的编译器（MSVC 2022+、Clang 16+）
- PDFium 二进制分发（含头文件 `fpdfview.h` 和库文件）
- 仅在 **Windows** 上进行过测试

### 许可

本项目基于 MIT 许可证发布，详见 [LICENSE](LICENSE)。

使用或分发时**必须保留**原始版权声明。

---

## <a name="english"></a>English

**modern-pdfium** is a modern C++ (C++20) wrapper for [PDFium](https://pdfium.googlesource.com/pdfium/). It wraps PDFium's C API into ergonomic and type-safe C++ interfaces using RAII idioms, strongly-typed enums, and namespaces.

### Features / Wrapped Modules

| Module | Header | Description |
|--------|--------|-------------|
| **Core** | `core.h` | Library init/destroy (RAII), error codes, sandbox policy, version |
| **Document** | `document.h` | Document open/close, metadata, page count, permissions |
| **Page** | `page.h` | Page rendering, size, rotation |
| **Bitmap** | `bitmap.h` | Bitmap creation and manipulation |
| **Text** | `text.h` | Text extraction, search, web links |
| **Annotation** | `annotation.h` | Annotation types, properties, appearance |
| **Bookmark** | `bookmark.h` | Bookmark traversal, titles, actions |
| **Attachment** | `attachment.h` | File attachment extraction |
| **Signature** | `signature.h` | Digital signature information |
| **Form** | `form.h` | AcroForm / XFA form filling and events |
| **Image** | `image.h` | Image object extraction and info |
| **Font** | `font.h` | Font information and operations |
| **Path** | `path.h` | Path objects, segments, clip paths |
| **Struct Tree** | `struct_tree.h` | Structure tree, elements, attributes (accessibility / tagged PDF) |
| **Link** | `link.h` | Document links and destinations |
| **Action** | `action.h` | Action types and parameters |
| **Save** | `save.h` | Incremental and full save |
| **Data Avail** | `dataavail.h` | Progressive availability query and loading |

### Build

```bash
mkdir build && cd build
cmake .. -DPDFIUM_ROOT=/path/to/pdfium
cmake --build .
```

Optionally build examples:

```bash
cmake .. -DPDFIUM_ROOT=/path/to/pdfium -DBUILD_EXAMPLES=ON
```

### Requirements

- C++20 compatible compiler (MSVC 2022+, Clang 16+)
- PDFium binary distribution (including `fpdfview.h` headers and libraries)
- Tested on **Windows only**

### License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.

**The original copyright notice must be retained** in all copies or substantial portions of the Software.
