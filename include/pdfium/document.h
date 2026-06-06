#pragma once
#include <memory>
#include <string>
#include <vector>
#include <string_view>
#include "types.h"
#include "bookmark.h"

namespace pdfium {

class FormHandle;
class Attachment;
class Signature;

class Document {
public:
    // Load from file
    static std::unique_ptr<Document> LoadFromFile(const std::string& path, const std::string& password = "");
    // Load from memory
    static std::unique_ptr<Document> LoadFromMemory(const void* data, size_t size, const std::string& password = "");
    // Load from custom file access
    static std::unique_ptr<Document> LoadFromCustom(FPDF_FILEACCESS* file_access, const std::string& password = "");
    // Create new empty document
    static std::unique_ptr<Document> Create();

    ~Document();
    Document(const Document&) = delete;
    Document& operator=(const Document&) = delete;

    // Move operations
    Document(Document&& other) noexcept;
    Document& operator=(Document&& other) noexcept;

    // Page operations
    int GetPageCount() const;
    std::unique_ptr<Page> LoadPage(int index);
    void DeletePage(int index);
    std::unique_ptr<Page> NewPage(int index, double width, double height);

    // Document info
    int GetFileVersion() const;
    unsigned long GetPermissions() const;
    unsigned long GetUserPermissions() const;
    int GetSecurityHandlerRevision() const;

    // Metadata
    std::string GetMetaText(const std::string& tag) const;
    std::string GetPageLabel(int page_index) const;

    // Viewer preferences
    bool GetPrintScaling() const;
    int GetNumCopies() const;
    int GetDuplexType() const;

    // Bookmarks
    Bookmark GetFirstChild(const Bookmark* bookmark = nullptr) const;
    Bookmark FindBookmark(const std::wstring& title) const;

    // Named destinations
    int CountNamedDests() const;

    // Form type
    int GetFormType() const;
    
    // Form fill environment
    std::unique_ptr<FormHandle> InitFormFillEnvironment();
    
    // Attachments
    int GetAttachmentCount() const;
    std::unique_ptr<Attachment> GetAttachment(int index) const;
    std::unique_ptr<Attachment> AddAttachment(const std::wstring& name);
    bool DeleteAttachment(int index);
    
    // Signatures
    int GetSignatureCount() const;
    std::unique_ptr<Signature> GetSignatureObject(int index) const;
    
    // JavaScript actions
    int GetJavaScriptActionCount() const;
    
    // Import pages from another document
    bool ImportPages(const Document& src_doc, const std::string& page_range = "", int index = -1);
    bool ImportPagesByIndices(const Document& src_doc, const std::vector<int>& page_indices, int index = -1);
    
    // Catalog
    bool IsTagged() const;
    std::string GetLanguage() const;
    void SetLanguage(const std::string& language);

    // XFA
    int GetXFAPacketCount() const;
    std::string GetXFAPacketName(int index) const;
    std::vector<uint8_t> GetXFAPacketContent(int index) const;

    // Raw handle access
    FPDF_DOCUMENT Handle() const { return doc_; }
    
    // Check if document is valid
    explicit operator bool() const { return doc_ != nullptr; }

private:
    Document(FPDF_DOCUMENT doc);
    FPDF_DOCUMENT doc_;
};

} // namespace pdfium
