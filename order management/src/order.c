#include "order_dish.h"
#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


Order* makeNewOrder(int maBan, char* maNV, char* thoiGianTaoDon) {
    Order* newOrder = (Order*)malloc(sizeof(Order));
    if (newOrder == NULL) {
        printf("[makeNewOrder] Không thể cấp phát bộ nhớ cho đơn hàng mới.\n");
        return NULL;
    }

    newOrder->maBan = maBan;
    strcpy(newOrder->maNhanVien, maNV);
    strcpy(newOrder->thoiGianTaoDon, thoiGianTaoDon);
    strcpy(newOrder->thoiGianCapNhat, thoiGianTaoDon);

    // Cấp phát động cho danhSachMon
    newOrder->danhSachMon = (DishList*)malloc(sizeof(DishList));
    if (newOrder->danhSachMon == NULL) {
        printf("[makeNewOrder] Không thể cấp phát bộ nhớ cho danh sách món ăn.\n");
        free(newOrder);
        return NULL;
    }
    newOrder->danhSachMon->headDish = NULL;
    newOrder->danhSachMon->tailDish = NULL;

    newOrder->tongSoMon = 0;
    newOrder->tongSoDiaDat = 0;
    newOrder->tongSoMonTra = 0;
    newOrder->tongSoDiaTra = 0;
    newOrder->tongTien = 0;
    newOrder->trangThai = DANG_PHUC_VU;
    newOrder->next = NULL;

    return newOrder;
}

// 2. Trả về con trỏ tới order hiện có của bàn có <mã bàn> theo đúng thứ tự.
Order* search_order(OrderList *orderList, int maBan) {
    // Kiểm tra điều kiện đầu vào
    if (maBan <= 0) {
        printf("[search_order] Mã bàn không hợp lệ.\n");
        return NULL;
    }
    
    if (orderList == NULL) {
        printf("[search_order] Danh sách đơn hàng không tồn tại\n");
        return NULL;
    }

    Order* currentOrder = orderList->headOrder;
    while (currentOrder != NULL) {
        if (currentOrder->maBan == maBan) {
            if (currentOrder->trangThai == DANG_PHUC_VU) {
                printf("[search_order] Đơn hàng có mã bàn %d đang được phục vụ.\n", maBan);
            } else if (currentOrder->trangThai == DA_THANH_TOAN) {
                printf("[search_order] Đơn hàng có mã bàn %d đã thanh toán.\n", maBan);
            } else if (currentOrder->trangThai == DON_HUY) {
                printf("[search_order] Đơn hàng có mã bàn %d đã bị hủy.\n", maBan);
            }
            return currentOrder; // Trả về con trỏ tới order hiện có của bàn
        }
        currentOrder = currentOrder->next;
    }

    // printf("[search_order] Chưa có đơn hàng nào đang phục vụ cho bàn %d.\n", maBan);
    return NULL; // Không tìm thấy order cho bàn
}

// 2. In ra chi tiết các order của mã bàn nếu mà bàn đó đang được phục vụ
void print_order(OrderList* orderList, int maBan) {
    // Kiểm tra điều kiện đầu vào
    if (maBan <= 0) {
        printf("[print_order] Mã bàn không hợp lệ\n");
        return;
    }
    if (orderList == NULL) {
        printf("[print_order] Danh sách đơn hàng không tồn tại\n");
        return;
    }

    // Tìm kiếm order theo mã bàn
    Order* order = search_order(orderList, maBan);
    if (order == NULL) {
        printf("[print_order] Không có đơn hàng cho mã bàn %d.\n", maBan);
        return;
    }

    if (order->trangThai == DANG_PHUC_VU) {
        printf("[print_order] Chi tiết đơn hàng cho mã bàn %d\n", maBan);
        printf("\tMã nhân viên: %s\n", order->maNhanVien);
        printf("\tThời gian cập nhật: %s\n", order->thoiGianCapNhat);
        printf("\tTổng số món: %d, Tổng số đĩa đặt: %d\n", order->tongSoMon, order->tongSoDiaDat);
        printf("\tTổng số món trả: %d, Tổng số đĩa trả: %d\n", order->tongSoMonTra, order->tongSoDiaTra);
        printf("\tDanh sách món:\n");

        Dish* currentDish = order->danhSachMon->headDish;
        if (currentDish == NULL) {
            printf("[print_order] Không có món nào trong đơn hàng này.\n");
            return;
        }
        printf("\tSTT | MaMon |      Ten mon       | SL Dat | SL Tra |  GiaTien |   Trang thai  |   Ghi chu\n");
        printf("\t----|-------|--------------------|--------|--------|----------|---------------|-------------------\n");
        int stt = 1;
        while (currentDish != NULL) {
            printf("\t%-3d | %-5s | %-18s | %6d | %6d | %8d | %-13s | %s\n",
                stt++, currentDish->maMon, currentDish->tenMon,
                currentDish->soLuongDat, currentDish->soLuongTra,
                currentDish->giaTien,
                (currentDish->trangThai == CHUA_LAM) ? "Chua lam" :
                (currentDish->trangThai == DANG_LAM) ? "Dang lam" :
                (currentDish->trangThai == DA_LAM_XONG) ? "Da lam xong" : "Da huy",
                (currentDish->ghiChu[0]) ? currentDish->ghiChu : "");
            currentDish = currentDish->next;
        }
    } else if (order->trangThai == DA_THANH_TOAN) {
        printf("[print_order] Đơn hàng cho mã bàn %d đã được được thanh toán\n", maBan);
    } else if (order->trangThai == DON_HUY) {
        printf("[print_order] Đơn hàng cho mã bàn %d đã bị huỷ\n", maBan);
    }
}

// 3. Tạo ra một đơn hàng mới chưa có mã bàn hoặc đơn hàng cho mã bàn đó đã thanh toán xong, trả về đơn hàng vừa tạo
Order* create_order(OrderList* orderList, int maBan, char* maNV, char* thoiGianTaoDon) {
    printf("[create_order] Tạo đơn hàng cho mã bàn %d, mã nhân viên %s, thời gian tạo đơn %s\n", 
           maBan, maNV, thoiGianTaoDon);
    // Kiểm tra điều kiện đầu vào
    if (maBan <= 0) {
        printf("[create_order] Mã bàn không hợp lệ.\n");
        return NULL;
    }
    if (maNV == NULL) {
        printf("[create_order] Mã nhân viên không hợp lệ.\n");
        return NULL;
    }
    if (thoiGianTaoDon == NULL) {
        printf("[create_order] Thời gian không hợp lệ.\n");
        return NULL;
    }
    if (orderList == NULL) {
        printf("[create_order] Danh sách đơn hàng không tồn tại.\n");
        return NULL;
    }

    // Kiểm tra xem đã có order cho bàn này chưa
    Order* order = search_order(orderList, maBan);
    if (order == NULL) {
        // Tạo một đơn hàng mới
        Order *newOrder = makeNewOrder(maBan, maNV, thoiGianTaoDon);

        // Thêm đơn hàng mới vào danh sách đơn hàng
        if (orderList->headOrder == NULL && orderList->tailOrder == NULL) {
            orderList->headOrder = newOrder;
            orderList->tailOrder = newOrder;
        } else if (orderList->headOrder != NULL && orderList->tailOrder != NULL) {
            orderList->tailOrder->next = newOrder;
            orderList->tailOrder = newOrder;
        }
        
        // Khởi tạo các trường của đơn hàng mới
        printf("[create_order] Đã tạo đơn hàng mới thành công cho mã bàn %d.\n", maBan);
        return newOrder;
    
    } else if (order->trangThai == DA_THANH_TOAN || order->trangThai == DON_HUY) {
        if (order->trangThai == DA_THANH_TOAN) {
            printf("[create_order] Đơn hàng cho mã bàn %d đã thanh toán. Tạo đơn hàng mới.\n", maBan);
        } else {
            printf("[create_order] Đơn hàng cho mã bàn %d đã bị hủy. Tạo đơn hàng mới.\n", maBan);
        }
        // Cập nhật thông tin đơn hàng mới
        strcpy(order->maNhanVien, maNV);
        order->trangThai = DANG_PHUC_VU;
        order->tongSoMon = 0;
        order->tongSoDiaDat = 0;
        order->tongSoMonTra = 0;
        order->tongSoDiaTra = 0;
        order->tongTien = 0;
        strcpy(order->thoiGianTaoDon, thoiGianTaoDon);
        strcpy(order->thoiGianCapNhat, thoiGianTaoDon);
        
        // Giải phóng danh sách món cũ
        if (order->danhSachMon->headDish != NULL && 
            order->danhSachMon->tailDish != NULL) {
            
            free_dish_list(order->danhSachMon);
            order->danhSachMon->headDish = NULL;
            order->danhSachMon->tailDish = NULL;
            printf("[create_order] Đã giải phóng danh sách món cũ cho mã bàn %d.\n", maBan);
        } else if (order->danhSachMon->headDish == NULL && 
                    order->danhSachMon->tailDish == NULL) {
            printf("[create_order] Danh sách món ăn của đơn hàng có mã bàn %d đã rỗng.\n", maBan);
        }
        printf("[create_order] Đã tạo lại đơn hàng mới cho mã bàn %d.\n", maBan);
    
    } else if (order->trangThai == DANG_PHUC_VU) {
        printf("[create_order] Đơn hàng cho mã bàn %d đã tồn tại và đang được phục vụ.\n", maBan);
    }

    return order;
}

// Huỷ order (chỉ huỷ nếu tổng số món trả và tổng số đĩa trả bằng 0)
int cancel_order(OrderList *orderList, int maBan) {
    if (maBan <= 0) {
        printf("[cancel_order] Mã bàn không hợp lệ.\n");
        return 0;
    }
    if (orderList == NULL) {
        printf("[cancel_order] Danh sách đơn hàng không tồn tại.\n");
        return 0;
    }
    
    Order* order = search_order(orderList, maBan);
    if (order == NULL) {
        printf("[cancel_order] Không tìm thấy đơn hàng cho mã bàn %d.\n", maBan);
        return 0;
    }
    
    if (order->trangThai == DANG_PHUC_VU) {
        // Chỉ huỷ đơn hàng nếu tổng số món trả và tổng số đĩa trả bằng 0
        if (order->tongSoMonTra == 0 && order->tongSoDiaTra == 0) {
            printf("[cancel_order] Đơn hàng cho mã bàn %d đang được phục vụ. Hủy đơn hàng.\n", maBan);
            // Cập nhậ trạng thái của đơn hàng
            order->trangThai = DON_HUY;
            strcpy(order->thoiGianCapNhat, get_current_time(order->thoiGianCapNhat, sizeof(order->thoiGianCapNhat)));
            // Giải phóng danh sách món ăn
            if (order->danhSachMon->headDish != NULL && order->danhSachMon->tailDish != NULL) {
                free_dish_list(order->danhSachMon);
                order->danhSachMon->headDish = NULL;
                order->danhSachMon->tailDish = NULL;
                printf("[cancel_order] Đã giải phóng danh sách món ăn cho mã bàn %d.\n", maBan);
            } else {
                printf("[cancel_order] Danh sách món ăn của đơn hàng có mã bàn %d đã rỗng.\n", maBan);
            }
            printf("[cancel_order] Đơn hàng cho mã bàn %d đã được hủy thành công.\n", maBan);
        } else {
            printf("[cancel_order] Không thể hủy đơn hàng cho mã bàn %d vì đã có món ăn được làm xong hoặc đã trả.\n", maBan);
            return 0;
        }
    } else if (order->trangThai == DA_THANH_TOAN) {
        printf("[cancel_order] Đơn hàng cho mã bàn %d đã thanh toán. Không thể hủy.\n", maBan);
        return 0;
    } else if (order->trangThai == DON_HUY) {
        printf("[cancel_order] Đơn hàng cho mã bàn %d đã bị hủy trước đó.\n", maBan);
        return 0;
    }

    return 1; // Thành công
}


