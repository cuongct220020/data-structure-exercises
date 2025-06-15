#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "order_dish.h"
#include "utility.h"

char* get_current_time(char* buffer, int bufferSize) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(buffer, bufferSize, "%Y-%m-%d %H:%M:%S", t);
    return buffer;
}

int get_next_valid_line(char **line, size_t *len, FILE *fp) {
    while (getline(line, len, fp) != -1) {
        (*line)[strcspn(*line, "\r\n")] = 0;
        if (strlen(*line) == 0 || (*line)[0] == '/') continue;
        if ((*line)[0] == '#') return -1;
        return 1; // dòng hợp lệ
    }
    return 0; // EOF
}

// Khởi tạo danh sách đơn hàng
OrderList* init_order_list() {
    OrderList* orderList = (OrderList*)malloc(sizeof(OrderList));
    if (orderList == NULL) {
        printf("Khong the cap phat bo nho cho danh sach don hang.\n");
        return NULL;
    }
    
    orderList->headOrder = NULL;
    orderList->tailOrder = NULL;
    
    return orderList;
}

// Giải phóng bộ nhớ
void free_order_list(OrderList* orderList) {
    if (orderList == NULL) return;
    Order* currentOrder = orderList->headOrder;
    while (currentOrder != NULL) {
        Order* nextOrder = currentOrder->next;
        if (currentOrder->danhSachMon) {
            free_dish_list(currentOrder->danhSachMon);
            free(currentOrder->danhSachMon);
        }
        free(currentOrder);
        currentOrder = nextOrder;
    }
    free(orderList);
}

void create_bill(OrderList *orderList, int maBan) {
    if (maBan <= 0) {
        printf("[create_bill] Mã bàn không hợp lệ.\n");
        return;
    }
    if (orderList == NULL) {
        printf("[create_bill] Danh sách đơn hàng không tồn tại.\n");
        return;
    }

    Order* order = search_order(orderList, maBan);
    if (order == NULL) {
        printf("[create_bill] Không tìm thấy đơn hàng cho mã bàn %d.\n", maBan);
        return;
    }

    if (order->trangThai == DA_THANH_TOAN || order->trangThai == DON_HUY) {
        printf("[create_bill] Đơn hàng cho mã bàn %d đã thanh toán hoặc đã bị hủy. Không thể tạo hoá đơn.\n", maBan);
        return;
    }

    // Đảm bảo thư mục output tồn tại ở cùng mức với build, input
    struct stat st = {0};
    if (stat("../output", &st) == -1) {
        if (mkdir("../output", 0755) != 0) {
            printf("[create_bill] Không thể tạo thư mục ../output\n");
            return;
        }
    }

    // Tạo tên file hóa đơn ở ../output
    char filename[128];
    snprintf(filename, sizeof(filename), "../output/bill_%02d.txt", maBan);

    FILE* fp = fopen(filename, "w");
    if (!fp) {
        printf("[create_bill] Không thể tạo file hóa đơn %s\n", filename);
        return;
    }

    // In thông tin hóa đơn
    fprintf(fp, "==================== HOÁ ĐƠN THANH TOÁN ====================\n");
    fprintf(fp, "Mã bàn %d\n", order->maBan);
    fprintf(fp, "Mã nhân viên %s\n", order->maNhanVien);
    fprintf(fp, "Thời gian tạo đơn: %s\n", order->thoiGianTaoDon);
    fprintf(fp, "Thời gian cập nhật: %s\n", order->thoiGianCapNhat);
    fprintf(fp, "Tổng số món đặt: %d\n", order->tongSoMon);
    fprintf(fp, "Tổng số đĩa đặt: %d\n", order->tongSoDiaDat);
    fprintf(fp, "---------------------------------------------"
                "------------------------------\n");
    fprintf(fp, "STT | Ma mon | Ten mon                   | SL | Gia     | Thanh tien   | Ghi chu\n");
    fprintf(fp, "----|--------|---------------------------|----|---------|--------------|--------------------------\n");

    int stt = 1;
    long long tongTien = 0;
    Dish* dish = order->danhSachMon->headDish;
    while (dish != NULL) {
        long long thanhTien = 0;
        char ghiChuHienThi[256] = "";
        if (dish->trangThai != DA_HUY) {
            thanhTien = dish->giaTien * dish->soLuongDat;
            tongTien += thanhTien;
            if (dish->ghiChu[0]) {
                snprintf(ghiChuHienThi, sizeof(ghiChuHienThi), "%s", dish->ghiChu);
            }
        } else {
            if (dish->ghiChu[0]) {
                snprintf(ghiChuHienThi, sizeof(ghiChuHienThi), "Đã huỷ - %s", dish->ghiChu);
            } else {
                snprintf(ghiChuHienThi, sizeof(ghiChuHienThi), "Đã huỷ");
            }
        }
        fprintf(fp, "%-3d | %-6s | %-25s | %-2d | %-7d | %-12lld | %s\n",
                stt++, dish->maMon, dish->tenMon, dish->soLuongDat, dish->giaTien, thanhTien,
                ghiChuHienThi);
        dish = dish->next;
    }

    fprintf(fp, "---------------------------------------------\n");
    fprintf(fp, "Tong tien: %lld\n", tongTien);
    fprintf(fp, "=============================================\n");

    fclose(fp);
    printf("[create_bill] Đã tạo hóa đơn cho bàn %d tại file %s\n", maBan, filename);
}