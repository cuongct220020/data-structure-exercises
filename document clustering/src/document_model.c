#include "document_model.h"
#include <stdlib.h> // Để sử dụng malloc, free, strdup
#include <string.h> // Để sử dụng strdup

// Bao gồm các header của các module khác nếu cần hàm giải phóng của chúng
// (ví dụ: free_shingle_set, free_minhash_signature)
#include "shingle_generator.h" // Chứa free_shingle_set
#include "minhash.h"           // Chứa free_minhash_signature

// Triển khai hàm tạo Document
Document create_document_object(int id, const char* original_content,
                                ShingleSet* shingles, MinHashSignature* signature) {
    Document doc; // Tạo struct trên stack

    doc.id = id;

    // Sao chép nội dung gốc. Cần kiểm tra NULL cho original_content
    if (original_content) {
        doc.content = strdup(original_content);
        if (!doc.content) {
            // Xử lý lỗi cấp phát bộ nhớ.
            // Trong trường hợp này, vì hàm trả về Document by value,
            // không thể trả về NULL. Có thể in lỗi và gán NULL,
            // hoặc thoát chương trình tùy chính sách xử lý lỗi.
            fprintf(stderr, "Lỗi: Không thể cấp phát bộ nhớ cho nội dung gốc của tài liệu ID %d.\n", id);
            doc.content = NULL;
        }
    } else {
        doc.content = NULL;
    }

    // Gán con trỏ shingles và signature.
    // Hàm này 'nhận' quyền sở hữu các con trỏ này, nên không cần sao chép chúng.
    // Người gọi chịu trách nhiệm đảm bảo các con trỏ này hợp lệ.
    doc.shingles = shingles;
    doc.signature = signature;

    return doc; // Trả về struct Document by value
}

// Triển khai hàm giải phóng Document
void free_document_object(Document* doc) {
    if (!doc) {
        return; // Không làm gì nếu con trỏ là NULL
    }

    // Giải phóng nội dung gốc nếu nó đã được cấp phát
    if (doc->content) {
        free(doc->content);
        doc->content = NULL; // Tránh lỗi double-free
    }

    // Giải phóng ShingleSet nếu nó tồn tại
    if (doc->shingles) {
        free_shingle_set(doc->shingles); // Hàm này phải giải phóng cả struct ShingleSet và các chuỗi bên trong nó
        doc->shingles = NULL;
    }

    // Giải phóng MinHashSignature nếu nó tồn tại
    if (doc->signature) {
        free_minhash_signature(doc->signature); // Hàm này phải giải phóng cả struct MinHashSignature và mảng values
        doc->signature = NULL;
    }

    // Không cần free(doc) ở đây vì 'doc' được truyền dưới dạng con trỏ tới một phần tử trong mảng 'documents',
    // và mảng 'documents' sẽ được giải phóng một lần sau khi tất cả các phần tử đã được dọn dẹp.
}