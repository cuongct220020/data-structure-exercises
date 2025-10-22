#ifndef RB_TREE_HPP
#define RB_TREE_HPP

#include <string>
#include <unordered_map>
#include <functional>

#define TIMEOUT_THRESHOLD 300  // 5 phút

// Màu của nút
enum Color { RED, BLACK };

// Cấu trúc nút cơ bản của cây đỏ đen, tổng quát hóa
template<typename T>
struct RBNode {
    T* left = nullptr;
    T* right = nullptr;
    T* parent = nullptr;
    Color color = RED;
};

// Phiên bản cụ thể: phiên đăng nhập
struct SessionNode : public RBNode<SessionNode> {
    std::string sessionID;
    std::string userID;
    long lastActiveTime;  // Thời gian hoạt động cuối cùng
    long loginTime;

    std::string get_key() const { return sessionID; }
};

struct OnlineUserNode : public RBNode<OnlineUserNode> {
    std::string userID;
    long lastActiveTime;
    std::string activityInfo;

    // // Cây R-B quản lý các phiên đăng nhập của người dùng của người dùng
    // SessionNode* sessionRoot;

    // // Bản đồ ánh xạ người dùng đến cây R-B quản lý toàn bộ phiên đăng nhập
    // std::unordered_map<std::string, SessionNode*> userToSessionsMap; // key = userID
    std::string get_key() const { return userID; }
};

template<typename Node>
void left_rotate(Node*& root, Node* x) {
    Node* y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;

    y->parent = x->parent;
    if (!x->parent) {
        root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

template<typename Node>
void right_rotate(Node*& root, Node* x) {
    Node* y = x->left;
    x->left = y->right;
    if (y->right) y->right->parent = x;

    y->parent = x->parent;
    if (!x->parent) {
        root = y;
    } else if (x == x->parent->right) {
        x->parent->right = y;
    } else {
        x->parent->left = y;
    }
    y->right = x;
    x->parent = y;
}

template<typename Node>
void fix_insert(Node*& root, Node* z) {
    while (z->parent && z->parent->color == RED) {
        if (z->parent == z->parent->parent->left) {
            Node* y = z->parent->parent->right;
            if (y && y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    left_rotate(root, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                right_rotate(root, z->parent->parent);
            }
        } else {
            Node* y = z->parent->parent->left;
            if (y && y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    right_rotate(root, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                left_rotate(root, z->parent->parent);
            }
        }
    }
    root->color = BLACK;
}

template <typename Node>
void fix_delete(Node*& root, Node* x) {
    while (x != root && x && x->parent && x->color == BLACK) {
        if (x == x->parent->left) {
            Node* w = x->parent->right;
            if (w && w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                left_rotate(root, x->parent);
                w = x->parent->right;
            }
            if ((!w || !w->left || w->left->color == BLACK) &&
                (!w || !w->right || w->right->color == BLACK)) {
                if (w) w->color = RED;
                x = x->parent;
            } else {
                if (!w || !w->right || w->right->color == BLACK) {
                    if (w && w->left) w->left->color = BLACK;
                    if (w) w->color = RED;
                    if (w) right_rotate(root, w);
                    w = x->parent->right;
                }
                if (w) w->color = x->parent->color;
                x->parent->color = BLACK;
                if (w && w->right) w->right->color = BLACK;
                left_rotate(root, x->parent);
                x = root;
            }
        } else {
            Node* w = x->parent->left;
            if (w && w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                right_rotate(root, x->parent);
                w = x->parent->left;
            }
            if ((!w || !w->left || w->left->color == BLACK) &&
                (!w || !w->right || w->right->color == BLACK)) {
                if (w) w->color = RED;
                x = x->parent;
            } else {
                if (!w || !w->left || w->left->color == BLACK) {
                    if (w && w->right) w->right->color = BLACK;
                    if (w) w->color = RED;
                    if (w) left_rotate(root, w);
                    w = x->parent->left;
                }
                if (w) w->color = x->parent->color;
                x->parent->color = BLACK;
                if (w && w->left) w->left->color = BLACK;
                right_rotate(root, x->parent);
                x = root;
            }
        }
    }
    if (x) x->color = BLACK;
}

template<typename Node>
void rb_insert(Node*& root, Node* z) {
    Node* y = nullptr;
    Node* x = root;

    // Bước 1: Tìm vị trí thích hợp (như BST)
    while (x != nullptr) {
        y = x;
        if (z->get_key() < x->get_key())
            x = x->left;
        else
            x = x->right;
    }

    z->parent = y;
    if (y == nullptr) {
        root = z;  // Cây rỗng
    } else if (z->get_key() < y->get_key()) {
        y->left = z;
    } else {
        y->right = z;
    }

    z->left = nullptr;
    z->right = nullptr;
    z->color = RED;

    // Bước 2: Sửa lại cây R-B để cân bằng
    fix_insert(root, z);
}

// Tìm node nhỏ nhất
template<typename Node>
Node* treeMinimum(Node* x) {
    while (x->left != NULL) x = x->left;
    return x;
}

// Thay thế 1 node u bằng v
template<typename Node>
void rbTransplant(Node **root, Node *u, Node *v) {
    if (!u->parent) *root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    if (v) v->parent = u->parent;
}

template<typename Node>
void rb_delete(Node*& root, Node* z) {
    Node* y = z;
    Node* x = nullptr;
    Color y_original_color = y->color;

    // Trường hợp 1: Không có con trái
    if (z->left == nullptr) {
        x = z->right;
        rbTransplant(&root, z, z->right);
    }
    // Trường hợp 2: Không có con phải
    else if (z->right == nullptr) {
        x = z->left;
        rbTransplant(&root, z, z->left);
    }
    // Trường hợp 3: Có cả 2 con → tìm successor
    else {
        y = treeMinimum(z->right);
        y_original_color = y->color;
        x = y->right;

        if (y->parent == z) {
            if (x) x->parent = y;
        } else {
            rbTransplant(&root, y, y->right);
            y->right = z->right;
            if (y->right) y->right->parent = y;
        }

        rbTransplant(&root, z, y);
        y->left = z->left;
        if (y->left) y->left->parent = y;
        y->color = z->color;
    }

    if (y_original_color == BLACK && x) {
        fix_delete(root, x);
    }
}

// Tìm node có key == khóa cho trước (dựa trên comparator với Node::key)
// Yêu cầu Node phải có một trường key kiểu std::string (hoặc tương đương)
template<typename Node>
Node* bst_search(Node*& root, const std::string& key) {
    if (!root) return nullptr;

    if (key == root->get_key()) return root;

    if (key < root->get_key())
        return bst_search(root->left, key);
    else
        return bst_search(root->right, key);
}

// Duyệt cây theo thứ tự inorder và áp dụng hàm func cho mỗi node
template<typename Node, typename Func>
void inorder_traversal(const Node* root, Func func) {
    if (!root) return;
    inorder_traversal(root->left, func);
    func(root);
    inorder_traversal(root->right, func);
}


// ===== Các hàm quản lý phiên đăng nhập và người dùng online ===== //
bool is_user_has_any_session(OnlineUserNode*& root, const std::string& userId);

bool is_valid_session(SessionNode*& root, const std::string& sessionId, const std::string& userId, long currentTime, FILE *out);

void insert_session(SessionNode*& root, const std::string& SessionId, const std::string& UserId, long loginTime, FILE *out);

void insert_online_user(OnlineUserNode*& root, const std::string& UserId, long lastActiveTime, const std::string action, FILE *out);

void delete_session(SessionNode*& root, const std::string& SessionId, const std::string& UserId, FILE *out);

void delete_online_user(OnlineUserNode*& root, const std::string& UserId, FILE *out);

// Hàm xoá tất cả session của một người dùng nếu người dùng logout khỏi toàn bộ thiết bị
void delete_all_sessions_of_user(SessionNode*& root, const std::string& userId, FILE *out);

// Hàm in ra session đang hoạt động, dùng kết hợp với inorder_traversal
void print_active_session_node(SessionNode* node, FILE* out);

// Forward declaration for print_online_user_to_file
void print_online_user_node(OnlineUserNode* node, FILE* out);

#endif  // RB_TREE_HPP
