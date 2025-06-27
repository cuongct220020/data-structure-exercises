#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <functional>

#include "rb_tree.hpp"

/**
 * Chuyển timeStr thành timestamp (giây từ đầu ngày)
 * Hỗ trợ cả định dạng:
 *   - "08:00:00" → 28800
 *   - "2025-05-08T09:00:00Z" → 32400 (tức 09:00:00)
 */
long convert_to_timestamp(const std::string& timeStr) {
    int hh, mm, ss;

    const char* cstr = timeStr.c_str();

    if (strchr(cstr, 'T')) {
        // ISO 8601: "2025-05-08T09:00:00Z"
        if (sscanf(cstr, "%*d-%*d-%*dT%2d:%2d:%2dZ", &hh, &mm, &ss) == 3) {
            return (long)hh * 3600 + mm * 60 + ss;
        }
    } else {
        // Giản lược: "08:00:00"
        if (sscanf(cstr, "%2d:%2d:%2d", &hh, &mm, &ss) == 3) {
            return (long)hh * 3600 + mm * 60 + ss;
        }
    }

    // Nếu không parse được
    return -1;
}

// void process_single_device(FILE *in, SessionNode **root1, FILE *out) {
//     char *line = NULL;
//     size_t len = 0;

//     while (getline(&line, &len, in) != -1) {
//         // bỏ newline
//         if (line[strlen(line)-1]=='\n') line[strlen(line)-1]=0;

//         // tách
//         char *timeStr = strtok(line, " ");
//         char *userId  = strtok(NULL, " ");
//         char *action  = strtok(NULL, "\0");  // phần còn lại

//         long ts = convert_to_timestamp(timeStr);
//         if (ts<0) continue;

//         if (strcmp(action, "login")==0) {
//             *root1 = insert_session1(*root1, userId, ts, action, out);
//         }
//         else if (strcmp(action, "logout")==0) {
//             *root1 = delete_session1(*root1, userId, out);
//         }
//         else {
//             // update lastActiveTime và activityInfo
//             SessionNode *node = search_session1(*root1, userId, out);
//             if (node) {
//                 node->lastActiveTime = ts;
//                 strncpy(node->sessionData, action, sizeof(node->sessionData)-1);
//                 fprintf(out, "[update_session1] %s hoạt động '%s' tại %ld\n",
//                         userId, action, ts);
//             }
//         }
//     }

//     free(line);
// }

void process_multi_device(FILE *in, SessionNode*& sessionRoot, OnlineUserNode*& onlineUserRoot, FILE *out) {
    char *line = NULL;
    size_t len = 0;

    while (getline(&line, &len, in) != -1) {
        if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = '\0';

        // Parse format: timestamp userId sessionId activity ...
        char *timeStr = strtok(line, " ");
        char *userId = strtok(NULL, " ");
        char *sessionId = strtok(NULL, " ");
        char *activity = strtok(NULL, "\0");

        if (!timeStr || !userId || !sessionId || !activity) {
            fprintf(out, "[process_multi_device] Dòng đầu vào không hợp lệ: %s\n", line);
            continue;
        }

        std::string timestamp = timeStr;
        std::string user_id = userId;
        std::string session_id = sessionId;
        std::string action = activity;
        long ts = convert_to_timestamp(timestamp);
        if (ts < 0) {
            fprintf(out, "[process_multi_device] Timestamp %s không hợp lệ.\n", timestamp.c_str());
            continue;
        }

        // === Handle activity types ===
        if (action == "login failed") { // Bỏ qua các login thất bại
            fprintf(out, "[process_multi_device] User %s đăng nhập không thành công với session %s, bỏ qua user này.\n", user_id.c_str(), session_id.c_str());
        } else if (action == "login success") { // Xử lý login thành công
            fprintf(out, "[process_multi_device] User %s đăng nhập thành công với session %s.\n", user_id.c_str(), session_id.c_str());
            
            if (!bst_search(sessionRoot, session_id)) {
                // Thêm session mới vào cây R-B Session
                insert_session(sessionRoot, session_id, user_id, ts, out);

                // Thêm user mới vào cây R-B OnlineUser
                insert_online_user(onlineUserRoot, user_id, ts, action, out);
            } else {
                fprintf(out, "[process_multi_device] Session %s đã tồn tại.\n", session_id.c_str());
            }
        } else if (action == "logout") { // Xử lý logout
            fprintf(out, "[process_multi_device] User %s đã logout với session %s.\n", user_id.c_str(), session_id.c_str());
            // Xóa session
            delete_session(sessionRoot, session_id, user_id, out);

            // Nếu user không còn session nào → xóa khỏi online user tree
            if (!is_user_has_any_session(onlineUserRoot, user_id)) {
                fprintf(out, "[process_multi_device] User %s không còn session nào, xóa khỏi online user tree.\n", user_id.c_str());
                delete_online_user(onlineUserRoot, user_id, out);
            }
        } else { // Với các thao tác khác (như hoạt động thường nhật)
            // Kiểm tra xem user có đang online không
            OnlineUserNode* onlineUser = bst_search(onlineUserRoot, user_id);
            if (!onlineUser) {
                fprintf(out, "[process_multi_device] User %s không online, không thể cập nhật hoạt động.\n", user_id.c_str());
                continue;
            }

            // Cập nhật hoạt động thường nhật
            onlineUser->lastActiveTime = ts;
            onlineUser->activityInfo = action; // Cập nhật thông tin hoạt động
            fprintf(out, "[process_multi_device] Đã cập nhật thông tin thường nhật của user %s: %s\n", user_id.c_str(), action.c_str());
        }
    }
    free(line);
}

// Wrapper functions for inorder_traversal
void print_session_wrapper(const SessionNode* node, FILE* out) {
    print_active_session_node(const_cast<SessionNode*>(node), out);
}

void print_online_user_wrapper(const OnlineUserNode* node, FILE* out) {
    print_online_user_node(const_cast<OnlineUserNode*>(node), out);
}

int main(void) {
    // ---- Test TH1: chỉ 1 thiết bị ----
    // Mở file xuất log cho TH1
    // FILE *out1 = fopen("../out/output1.txt", "w");
    // if (!out1) {
    //     fprintf(stderr, "[main] Không thể mở output.txt\n");
    //     return 1;
    // }

    // // Khởi tạo hai cây
    // SessionNode *root1 = NULL;
    
    
    // // FILE *in1 = fopen("../data/user_activity_logs.txt", "r");
    // // if (!in1) {
    // //     fprintf(stderr, "[main] Không thể mở user_activity_logs.txt\n");
    // // } else {
    // //     fprintf(out1, "=== Bắt đầu xử lý TH1 (1 thiết bị mỗi user) ===\n");
    // //     process_single_device(in1, &root1, out1);
    // //     fprintf(out1, "\n-- Active sessions TH1 --\n");
    // //     print_active_sessions(root1, out1);
    // //     fclose(in1);
    // //     fclose(out1);
    // // }

    // ============================== Test TH2: đa thiết bị ============================== //
    FILE *out2 = fopen("../out/output2.txt", "w");
    if (!out2) {
        fprintf(stderr, "[main] Không thể mở output.txt\n");
        return 1;
    }

    SessionNode *session_root2 = NULL;
    OnlineUserNode *online_user_root2 = NULL;

    FILE *in2 = fopen("../data/user_sessions_multidevices_log.txt", "r");
    if (!in2) {
        fprintf(out2, "[main] Không thể mở user_sessions_multidevices_log.txt\n");
    } else {
        fprintf(out2, "=== Bắt đầu xử lý TH2 (đa thiết bị) ===\n\n");
        process_multi_device(in2, session_root2, online_user_root2, out2);
        fprintf(out2, "\n========== Active sessions TH2 ==========\n");
        const SessionNode* const_session_root2 = session_root2;
        inorder_traversal(const_session_root2, [out2](const SessionNode* node) {
            print_active_session_node(const_cast<SessionNode*>(node), out2);
        });
        fprintf(out2, "\n========== All Online users TH2 ==========\n");
        const OnlineUserNode* const_online_user_root2 = online_user_root2;
        inorder_traversal(const_online_user_root2, [out2](const OnlineUserNode* node) {
            print_online_user_node(const_cast<OnlineUserNode*>(node), out2);
        });
        fclose(in2);
        fclose(out2);
    }

    return 0;
}