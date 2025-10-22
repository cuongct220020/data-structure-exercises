#ifndef DOCUMENT_MODEL_H
#define DOCUMENT_MODEL_H

#include "data_structures.h" // Bao gồm các định nghĩa struct Document, ShingleSet, MinHashSignature

// Tạo một đối tượng Document mới.
// Hàm này sẽ sao chép 'original_content' và lấy quyền sở hữu các con trỏ 'shingles' và 'signature'.
// Điều này có nghĩa là người gọi không cần (và không nên) giải phóng 'shingles' và 'signature'
// sau khi truyền chúng vào hàm này, vì Document sẽ chịu trách nhiệm giải phóng chúng sau này.
// Tham số:
//   id: ID duy nhất của tài liệu.
//   original_content: Chuỗi nội dung gốc của tài liệu. Hàm này sẽ tạo một bản sao.
//   shingles: Con trỏ tới ShingleSet đã được tạo cho tài liệu này.
//   signature: Con trỏ tới MinHashSignature đã được tạo cho tài liệu này.
// Trả về: Một cấu trúc Document đã được khởi tạo.
Document create_document_object(int id, const char* original_content,
                                ShingleSet* shingles, MinHashSignature* signature);

// Giải phóng bộ nhớ của một đối tượng Document.
// Hàm này sẽ giải phóng nội dung gốc, ShingleSet, và MinHashSignature của tài liệu.
// Tham số:
//   doc: Con trỏ tới đối tượng Document cần giải phóng.
void free_document_object(Document* doc);

#endif // DOCUMENT_MODEL_H