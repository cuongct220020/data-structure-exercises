#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "order_dish.h"
#include "utility.h"

int main() {
    if (!freopen("../input/order_input1.txt", "r", stdin)) {
        printf("[main] Không thể mở file input/Order_input1.txt\n");
        return 1;
    }

    OrderList* orderList = init_order_list();
    if (orderList == NULL) {
        printf("[main] Không thể tạo danh sách đơn hàng.\n");
        return 1;
    }

    char* line = NULL;
    size_t len = 0;
    int ret;

    while ((ret = get_next_valid_line(&line, &len, stdin)) != 0) {
        if (ret == -1) continue;
        if (strcmp(line, "? create_order") == 0) {
            printf("=============================== TẠO ĐƠN HÀNG ===============================\n");
            while ((ret = get_next_valid_line(&line, &len, stdin)) > 0) {
                char maNV[20], date[16], time[16];
                int maBan;
                //printf("DEBUG: line = [%s]\n", line);
                int matched = sscanf(line, "%s %d %s %s", maNV, &maBan, date, time);
                //printf("DEBUG: matched = %d\n", matched);
                if (matched == 4) {
                    char thoiGian[32];
                    snprintf(thoiGian, sizeof(thoiGian), "%s %s", date, time);
                    //printf("DEBUG: call create_order\n");
                    create_order(orderList, maBan, maNV, thoiGian);
                    printf("\n");
                } else {
                    printf("Định dạng dòng create_order không hơp lệ: %s\n", line);
                }
            }
            printf("\n");
        } else if (strcmp(line, "? add_dish") == 0) {
            printf("================================== GỌI MÓN ==================================\n");
            while ((ret = get_next_valid_line(&line, &len, stdin)) > 0) {
                char maNV[20], maMon[20], tenMon[MAX_NAME], ghiChu[MAX_NOTE];
                int maBan, soLuong, giaTien;
                int matched = sscanf(line, "%s %d %s \"%[^\"]\" %d %d \"%[^\"]\"",
                    maNV, &maBan, maMon, tenMon, &soLuong, &giaTien, ghiChu);
                if (matched == 7) {
                    add_dish(orderList, maNV, maBan, maMon, tenMon, soLuong, giaTien, ghiChu);
                    printf("\n");
                } else {
                    matched = sscanf(line, "%s %d %s \"%[^\"]\" %d %d \"\"",
                        maNV, &maBan, maMon, tenMon, &soLuong, &giaTien);
                    if (matched == 6) {
                        ghiChu[0] = '\0';
                        add_dish(orderList, maNV, maBan, maMon, tenMon, soLuong, giaTien, ghiChu);
                        printf("\n");
                    } else {
                        printf("Định dạng dòng add_dish không hợp lệ: %s\n", line);
                    }
                }
            }
            printf("\n");
        } else if (strcmp(line, "? update_dish") == 0) {
            printf("================================== TRẢ MÓN ==================================\n");
            while ((ret = get_next_valid_line(&line, &len, stdin)) > 0) {
                char maNV[20], maMon[20] = "", tenMon[MAX_NAME];
                int maBan, soLuong;
                int matched = sscanf(line, "%s %d %s \"%[^\"]\" %d",
                    maNV, &maBan, maMon, tenMon, &soLuong);
                if (matched == 5) {
                    update_dish(orderList, maBan, maMon, tenMon, soLuong);
                    printf("\n");
                } else {
                    // Trường hợp chỉ có tên món (dòng cuối file)
                    matched = sscanf(line, "%s %d \"%[^\"]\" %d",
                        maNV, &maBan, tenMon, &soLuong);
                    if (matched == 4) {
                        update_dish(orderList, maBan, "", tenMon, soLuong);
                        printf("\n");
                    } else {
                        printf("Định dạng dòng update_dish không hợp lệ\n");
                    }
                }
            }
            printf("\n");
        } else if (strcmp(line, "? cancel_dish") == 0) {
            printf("================================== HUỶ MÓN ==================================\n");
            while ((ret = get_next_valid_line(&line, &len, stdin)) > 0) {
                char maNV[20], maMon[20] = "", tenMon[MAX_NAME], ghiChu[MAX_NOTE];
                int maBan;
                int matched = sscanf(line, "%s %d %s \"%[^\"]\" \"%[^\"]\"",
                    maNV, &maBan, maMon, tenMon, ghiChu);
                if (matched == 5) {
                    cancel_dish(orderList, maBan, maMon, tenMon, ghiChu);
                    printf("\n");
                } else {
                    // Trường hợp chỉ có tên món (dòng cuối file)
                    matched = sscanf(line, "%s %d \"%[^\"]\" \"%[^\"]\"",
                        maNV, &maBan, tenMon, ghiChu);
                    if (matched == 4) {
                        cancel_dish(orderList, maBan, "", tenMon, ghiChu);
                        printf("\n");
                    } else {
                        printf("Định dạng cancel_dish không hợp lệ.\n");
                    }
                }
            }
            printf("\n");
        } else if (strncmp(line, "? print_order", 13) == 0) {
            printf("================================== IN ĐƠN HÀNG ==================================\n");
            int maBan;
            if (sscanf(line, "? print_order %d", &maBan) == 1) {
                print_order(orderList, maBan);
                printf("\n");
            } else {
                printf("Định dạng dòng print_order không hợp lệ.\n");
            }
        } else if (strcmp(line, "? cancel_order") == 0) {
            printf("================================== HUỶ ĐƠN HÀNG ==================================\n");
            while (get_next_valid_line(&line, &len, stdin)) {
                int maBan;
                if (sscanf(line, "%d", &maBan) == 1) {
                    cancel_order(orderList, maBan);
                    printf("\n");
                } else {
                    printf("Định dạng dòng cancel_order không hợp lệ.\n");
                }
            }
        } else if (strcmp(line, "? create_bill") == 0) {
            printf("================================== TẠO HOÁ ĐƠN ==================================\n");
            while ((ret = get_next_valid_line(&line, &len, stdin)) > 0) {
                char maNV[20]; int maBan;
                if (sscanf(line, "%s %d", maNV, &maBan) == 2) {
                    create_bill(orderList, maBan);
                    printf("\n");
                } else if (sscanf(line, "%d", &maBan) == 1) {
                    create_bill(orderList, maBan);
                    printf("\n");
                } else {
                    printf("Định dạng dòng create_bill không hợp lệ.\n");
                }
            }
        }
    }

    if (line) free(line);
    free_order_list(orderList);
    return 0;
}