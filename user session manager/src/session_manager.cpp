#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "rb_tree.hpp"

// Hàm này dùng để kiểm tra xem người dùng có đang online hay không
bool is_user_has_any_session(OnlineUserNode*& root, const std::string& userId) {
    if (!root) return false;
    if (root->userID == userId)
        return true;
    return is_user_has_any_session(root->left, userId) || 
            is_user_has_any_session(root->right, userId);
}


// Hàm này dùng đối với các dòng input không phải login, logout
bool is_valid_session(SessionNode*& root, const std::string& sessionId, const std::string& userId, long currentTime, FILE *out) {
    // Th0: Tham số truyền vào không hợp lệ
    if (sessionId.empty() || userId.empty()) {
        fprintf(out, "[is_valid_session] Tham số không hợp lệ: SessionId='%s', UserId='%s'\n",
                sessionId.c_str(), userId.c_str());
        return false;
    }

    // Th1: Tìm session
    SessionNode* session = bst_search(root, sessionId);
    if (!session) {
        fprintf(out, "[is_valid_session] SessionId='%s' không tồn tại trong hệ thống.\n", sessionId.c_str());
        return false;
    }

    // Th2: Kiểm tra hết hạn
    if (currentTime - session->lastActiveTime > TIMEOUT_THRESHOLD) {
        fprintf(out, "[is_valid_session] SessionId='%s' đã hết hạn.\n", sessionId.c_str());
        return false;
    }

    // Th3: Kiểm tra session đúng người dùng
    if (session->userID != userId) {
        fprintf(out, "[is_valid_session] SessionId='%s' không thuộc về UserId='%s'.\n",
                sessionId.c_str(), userId.c_str());
        return false;
    }

    // Hợp lệ
    fprintf(out, "[is_valid_session] SessionId='%s' là hợp lệ cho UserId='%s'.\n",
            sessionId.c_str(), userId.c_str());
    return true;
}

// Thêm một session mới vào cây R-B Session
void insert_session(SessionNode*& root, const std::string& SessionId, const std::string& UserId, long loginTime, FILE *out) {    
    // Tham số truyền vào không hợp lệ
    if (SessionId.empty() || UserId.empty() || loginTime < 0) {
        fprintf(out, "[insert_session] Tham số không hợp lệ: SessionId='%s', UserId='%s', loginTime=%ld\n",
                SessionId.c_str(), UserId.c_str(), loginTime);
        return;
    }

    // Tạo sesssion mới
    SessionNode *new_node = new SessionNode();
    new_node->color = RED;  // Mặc định là RED khi chèn vào
    new_node->sessionID = SessionId;
    new_node->userID = UserId;
    new_node->lastActiveTime = loginTime;
    new_node->loginTime = loginTime;
    new_node->left = nullptr;
    new_node->right = nullptr;
    new_node->parent = nullptr;
    

    if (root == nullptr) {
        root = new_node;
        new_node->color = BLACK;
        fprintf(out, "[insert_session] Chèn root sessionId='%s' cho user='%s'.\n", SessionId.c_str(), UserId.c_str());
    } else {
        rb_insert(root, new_node);
    }
}

// Thêm một người dùng online mới vào cây R-B OnlineUser
void insert_online_user(OnlineUserNode*& root, const std::string& UserId, long lastActiveTime, const std::string action, FILE *out) {
    // Tham số truyền vào không hợp lệ
    if (UserId.empty() || lastActiveTime < 0 || action.empty()) {
        fprintf(out, "[insert_online_user] Tham số không hợp lệ.\n");
        return;
    }


    // Tạo nút mới
    OnlineUserNode* new_node = new OnlineUserNode();  // Dùng new
    new_node->userID = UserId;
    new_node->lastActiveTime = lastActiveTime;
    new_node->activityInfo = action;
    new_node->left = nullptr;
    new_node->right = nullptr;
    new_node->parent = nullptr;

    // Chèn nút mới vào cây R-B OnlineUser
    rb_insert(root, new_node);
}

// Hàm xóa một session dựa trên SessionId và UserId
void delete_session(SessionNode*& root, const std::string& SessionId, const std::string& UserId, FILE *out) {
    if (root == nullptr) {
        fprintf(out, "[delete_session] Cây rỗng, không thể xóa session.\n");
        return;
    }
    // Kiểm tra giá trị đầu vào
    if (SessionId.empty() || UserId.empty()) {
        fprintf(out, "[delete_session] Tham số không hợp lệ: SessionId='%s', UserId='%s'\n", SessionId.c_str(), UserId.c_str());
        return;
    }

    // Tìm kiếm session cần xóa
    SessionNode* node = bst_search(root, SessionId);
    if (!node) {
        fprintf(out, "[delete_session] Không tìm thấy sessionId='%s' trong cây.\n", SessionId.c_str());
        return;
    }
    // Kiểm tra xem session có thuộc về người dùng không
    if (node->userID != UserId) {
        fprintf(out, "[delete_session] SessionId='%s' không thuộc về UserId='%s'.\n", SessionId.c_str(), UserId.c_str());
        return;
    }

    // Xóa session
    rb_delete(root, node);
    delete node;  // Giải phóng bộ nhớ
    fprintf(out, "[delete_session] Đã xóa sessionId='%s' của UserId='%s'.\n",
            SessionId.c_str(), UserId.c_str());
}

// Hàm xóa một người dùng online dựa trên UserId
void delete_online_user(OnlineUserNode*& root, const std::string& userId, FILE *out) {
    if (root == nullptr) {
        fprintf(out, "[delete_online_user] Cây rỗng, không thể xóa user.\n");
        return;
    }

    if (userId.empty()) {
        fprintf(out, "[delete_online_user] UserId không hợp lệ\n");
        return;
    }

    OnlineUserNode* node = bst_search(root, userId);
    if (!node) {
        fprintf(out, "[delete_online_user] Không tìm thấy user '%s' trong cây online\n", userId.c_str());
        return;
    }

    rb_delete(root, node);
    delete node;  // Giải phóng bộ nhớ
    fprintf(out, "[delete_online_user] Đã xóa user '%s' khỏi cây online\n", userId.c_str());
}

// Hàm xoá tất cả session của một người dùng nếu người dùng logout khỏi toàn bộ thiết bị
// void delete_all_sessions_of_user(SessionNode*& root, const std::string& userId, FILE *out) {
//     if (root == nullptr) {
//         fprintf(out, "[delete_all_sessions_of_user] Cây rỗng, không thể xóa session.\n");
//         return;
//     }

//     // Duyệt qua cây và xóa tất cả session của người dùng
//     SessionNode* current = root;
//     while (current) {
//         if (current->userID == userId) {
//             SessionNode* deleted_node = current;
//             current = current->right;  // Di chuyển sang phải để tiếp tục tìm kiếm
//             delete_session(root, deleted_node->sessionID, userId, out);
//         } else if (userId > current->userID) {
//             current = current->right;  // Di chuyển sang phải
//         } else {
//             current = current->left;   // Di chuyển sang trái
//         }
//     }
// }

// Hàm xoá tất cả session của một người dùng nếu người dùng logout khỏi toàn bộ thiết bị
void delete_all_sessions_of_user(const std::string& userId, FILE *out) {
    // Khối quản lý toàn bộ session của user
    
}

// Hàm in ra session đang hoạt động, dùng kết hợp với inorder_traversal
void print_active_session_node(SessionNode* node, FILE* out) {
    fprintf(out, "SessionID: %s | User: %s | Login: %ld | LastActive: %ld\n",
            node->sessionID.c_str(), node->userID.c_str(),
            node->loginTime, node->lastActiveTime);
}

// Hàm in ra thông tin người dùng đang online, dùng kết hợp với inorder_traversal
void print_online_user_node(OnlineUserNode* node, FILE* out) {
    fprintf(out, "User: %s | Last Active: %ld | Activity: %s\n", 
            node->userID.c_str(), node->lastActiveTime, node->activityInfo.c_str());
}