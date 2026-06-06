# modern-pdfium

[English](#english) | [中文](#chinese)

---

## <a name="chinese"></a>中文介绍

> ⚠ **仅支持 Windows x64 平台**

## 快速开始

```bash
# 1. 构建 PDFium（首次需要）
git clone https://github.com/Ding-yixia/pdfium_cmake_support.git
cd pdfium_cmake_support
python build.py --examples
cd ..

# 2. 构建封装库 + 示例
git clone https://github.com/Ding-yixia/modern-pdfium-cmake-starter.git
cd modern-pdfium-cmake-starter
mkdir build && cd build
cmake .. -DPDFIUM_ROOT=../pdfium_cmake_support -DBUILD_EXAMPLES=ON -G Ninja
cmake --build . --parallel

# 3. 运行示例
./examples/pdfium_basic ../pdfium_cmake_support/test.pdf
./examples/pdfium_text ../pdfium_cmake_support/test.pdf
```

## 示例程序

| 示例 | 文件 | 说明 |
|------|------|------|
| 📄 basic | `examples/basic.cpp` | 基础文档操作：加载、信息、渲染 |
| 🔤 text | `examples/text.cpp` | 文本提取：全文、区域、字符详情 |
| 🔖 bookmark | `examples/bookmark.cpp` | 书签遍历：递归导航、目标页码 |
| 🖍 annotation | `examples/annotation.cpp` | 注释枚举：类型、位置、颜色 |
| 🔍 search | `examples/search.cpp` | 文本搜索：关键字查找、URL 提取 |
| 🖼 render | `examples/render.cpp` | 多页面渲染：缩放、缩略图 |

## 详细文档

➡ **[docs/API.md](docs/API.md)** — 完整 API 参考、使用示例、高级用法

## 封装模块一览

| 模块 | 头文件 | 类 | FPDF 函数数 |
|------|--------|-----|-----------|
| 核心库 | `core.h` | `Library` | ~10 |
| 文档 | `document.h` | `Document` | ~30 |
| 页面 | `page.h` | `Page` | ~30 |
| 位图 | `bitmap.h` | `Bitmap` | ~10 |
| 文本 | `text.h` | `TextPage`, `TextFind`, `WebLinks` | ~30 |
| 注释 | `annotation.h` | `Annotation` | ~50 |
| 书签 | `bookmark.h` | `Bookmark`, `Dest`, `Action` | ~15 |
| 表单 | `form.h` | `FormHandle` | ~20 |
| 图像 | `image.h` | `ImageObject` | ~15 |
| 字体 | `font.h` | `Font` | ~15 |
| 路径 | `path.h` | `PathObject`, `PathSegment` | ~25 |
| 结构树 | `struct_tree.h` | `StructTree`, `StructElement` | ~30 |
| 附件 | `attachment.h` | `Attachment` | ~12 |
| 签名 | `signature.h` | `Signature` | ~8 |
| 保存 | `save.h` | `Save` | ~3 |
| 渐进加载 | `dataavail.h` | `Avail`, `PageObject` | ~15 |
| 链接 | `link.h` | `Link` | ~10 |

## 构建选项

```bash
cmake .. -DPDFIUM_ROOT=path/to/pdfium  -DBUILD_EXAMPLES=ON  -G Ninja
```

| 选项 | 默认 | 说明 |
|------|------|------|
| `PDFIUM_ROOT` | 空 | PDFium 构建产物路径 |
| `BUILD_EXAMPLES` | OFF | 构建示例程序 |

## 项目结构

```
├── include/pdfium/    18 个头文件
├── src/               16 个实现文件
├── examples/           6 个示例程序
├── docs/               API 文档
├── CMakeLists.txt      构建配置
├── LICENSE             MIT 协议
└── README.md           本文件
```

## 许可

MIT License © 2025 Ding-yixia

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

---

## ☕ 支持 / Support

> **「落红不是无情物，化作春泥更护花」** —— *龚自珍《己亥杂诗》*
>
> *"Falling petals are not heartless things; they turn into spring soil to nurture future blooms."*
>
> **「随风潜入夜，润物细无声」** —— *杜甫《春夜喜雨》*
>
> *"Stealing in with the night breeze, it moistens all things silently."*
>
> **「采得百花成蜜后，为谁辛苦为谁甜」** —— *罗隐《蜂》*
>
> *"After gathering nectar from a hundred flowers, for whom is the toil, for whom the sweetness?"*
>
> **"We are like dwarfs sitting on the shoulders of giants"** — *Bernard of Chartres*
>
> **"The lamp is not for itself; it gives light to others"** — *Rumi*
>
> **"How far that little candle throws his beams! So shines a good deed in a naughty world"** — *William Shakespeare, The Merchant of Venice*
>
> **"It is more blessed to give than to receive"** — *Acts 20:35*
>
> **"Each of you should give what you have decided in your heart to give, not reluctantly or under compulsion, for God loves a cheerful giver"** — *2 Corinthians 9:7*
>
> **"The best way to find yourself is to lose yourself in the service of others"** — *Mahatma Gandhi*
>
> **"継続は力なり"** — *日本の諺*
>
> *"Persistence is power" — Japanese proverb*
>
> ---
>
> 感谢您对此项目的关注。代码因分享而美丽，因互助而强大。
> 如果本项目对您有所帮助，欢迎请作者喝杯咖啡 ☕
>
> *Thank you for your interest. Code becomes beautiful through sharing, strong through community.*
> *If this project has helped you, feel free to buy me a coffee ☕*

| 中文收款码 |
|:---------:|
| ![support_me_work.png](image/support_me_work.png) |

### 🌍 世界语言感谢 / Thank You in Many Languages

| 语言 | 文字 | 发音 |
|------|------|------|
| 🇨🇳 **中文** | 感谢您的支持！ | Xièxiè nín de zhīchí! |
| 🇬🇧 **English** | Thank you for your support! | — |
| 🇫🇷 **Français** | Merci pour votre soutien ! | Mehr-see poor vo-tr soo-tee-ehn |
| 🇪🇸 **Español** | ¡Gracias por su apoyo! | Grah-see-as por su ah-poh-yo |
| 🇷🇺 **Русский** | Спасибо за вашу поддержку! | Spa-see-ba za va-shoo pad-derzh-koo |
| 🇩🇪 **Deutsch** | Vielen Dank für Ihre Unterstützung! | Fee-len Dank fewr Ee-re un-ter-shtet-zung |
| 🇯🇵 **日本語** | ご支援ありがとうございます | Go-shien a-ri-ga-tou go-za-i-masu |
| 🇰🇷 **한국어** | 지원해 주셔서 감사합니다 | Ji-won-hae ju-shyeo-seo gam-sa-ham-ni-da |
| 🇦🇪 **العربية** | شكرا لدعمكم | Shuk-ran li-da-mikum |
| 🇵🇹 **Português** | Obrigado pelo seu apoio! | Oh-bree-gah-doo peh-loo seh-oo ah-poy-yoo |
| 🇮🇹 **Italiano** | Grazie per il vostro supporto! | Graht-see-eh per eel voh-stroh soo-por-toh |
| 🇳🇱 **Nederlands** | Bedankt voor uw steun! | Be-dankt voor ew stun |
| 🇸🇪 **Svenska** | Tack för ditt stöd! | Tak fur dit stöd |
| 🇵🇱 **Polski** | Dziękuję za Państwa wsparcie! | Djen-koo-yeh za Pan-stva fspar-cheh |
| 🇹🇷 **Türkçe** | Desteğiniz için teşekkürler! | Des-teh-ee-niz ee-chehn teh-shek-kewr-ler |
| 🇻🇳 **Tiếng Việt** | Cảm ơn sự ủng hộ của bạn! | Gam un su ung ho kua ban |
| 🇮🇳 **हिन्दी** | आपके समर्थन के लिए धन्यवाद | Aap-ke sam-ar-than ke li-ye dhan-yavad |
| 🇹🇭 **ไทย** | ขอบคุณสำหรับการสนับสนุนของคุณ | Khob-khun sam-rap kan-sa-nap-sa-nun khong-khun |

> *愿知识之光普照人间，代码之力筑梦未来。*
> *May the light of knowledge illuminate the world, and the power of code build our dreams.*
> *Que la lumière du savoir éclaire le monde, et la puissance du code construise nos rêves.*
