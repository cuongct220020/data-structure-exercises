#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "order_dish.h"
#include "utility.h"

void free_dish(DishList *dishList, Dish* searchDish) {
    if (dishList == NULL || searchDish == NULL) {
        printf("[free_dish] Danh sách món ăn hoặc món ăn không hợp lệ.\n");
        return;
    }

    // Nếu searchDish là node đầu tiên
    if (dishList->headDish == searchDish) {
        dishList->headDish = searchDish->next;
        // Nếu cũng là node cuối cùng
        if (dishList->tailDish == searchDish) {
            dishList->tailDish = NULL;
        }
    } else {
        // Tìm node trước searchDish
        Dish* prev = dishList->headDish;
        while (prev != NULL && prev->next != searchDish) {
            prev = prev->next;
        }
        if (prev != NULL) {
            prev->next = searchDish->next;
            // Nếu searchDish là node cuối cùng
            if (dishList->tailDish == searchDish) {
                dishList->tailDish = prev;
            }
        }
    }
    free(searchDish);
}

void free_dish_list(DishList *dishList) {
    if (dishList == NULL) return;

    Dish* currentDish = dishList->headDish;
    while (currentDish != NULL) {
        Dish* nextDish = currentDish->next;
        free(currentDish);
        currentDish = nextDish;
    }
    dishList->headDish = NULL;
    dishList->tailDish = NULL;
}

Dish* search_dish(DishList* dishList, char* maMon, char* tenMon) {
    Dish *currentDish = dishList->headDish;
    while (currentDish != NULL) {
        if (strcmp(currentDish->maMon, maMon) == 0 && strcmp(currentDish->tenMon, tenMon) == 0) {
            return currentDish;
        }
        currentDish = currentDish->next;
    }
    return NULL;
}

Dish* makeNewDish(char* maMon, char* tenMon, 
                    int giaTien, int soLuongDat, 
                    char *thoiGianTaoMon, char* ghiChu) {

    Dish* newDish = (Dish*)malloc(sizeof(Dish));
    if (newDish == NULL) {
        printf("[makeNewDish] Không thể cấp phát động cho món ăn mới.\n");
        return 0;
    }
    // Cập nhật các trường thông tin của món ăn
    strcpy(newDish->maMon, maMon);
    strcpy(newDish->tenMon, tenMon);
    strcpy(newDish->thoiGianTaoMon, thoiGianTaoMon);
    newDish->giaTien = giaTien;
    newDish->soLuongDat = soLuongDat;
    newDish->soLuongTra = 0;
    strcpy(newDish->ghiChu, ghiChu);
    strcpy(newDish->thoiGianCapNhat, thoiGianTaoMon);
    newDish->trangThai = CHUA_LAM;
    newDish->next = NULL;

    return newDish;
}

// 4. Gọi món cho bàn có mã bàn, hàm trả về 1 nếu thêm món thành công
int add_dish(OrderList *orderList, 
            char* maNV, int maBan, 
            char* maMon, char* tenMon, 
            int soLuongDat, int giaTien, char* ghiChu) {
    if (maBan <= 0) {
        printf("[add_dish] Mã bàn %d không hợp lệ.\n", maBan);
        return 0; // Thất bại
    }
    if (maNV == NULL) {
        printf("[add_dish] Mã nhân viên %s không hợp lệ.\n", maNV);
        return 0; // Thất bại
    }
    if (maMon == NULL) {
        printf("[add_dish] Mã món ăn %s không hợp lệ.\n", maMon);
        return 0;
    }
    if (tenMon == NULL) {
        printf("[add_dish] Món ăn có mã món ăn %s không có tên món.\n", maMon);
        return 0;
    }
    if (soLuongDat <= 0) {
        printf("[add_dish] Số lượng đĩa đặt %d không hợp lệ", soLuongDat);
        return 0;
    }
    if (giaTien <= 0) {
        printf("[add_dish] Giá tiền %d của món ăn %s không hợp lệ.\n", giaTien, maMon);
        return 0; // Thất bại
    }
    if (orderList == NULL) {
        printf("[add_dish] Danh sách đơn hàng rỗng.\n");
        return 0;
    }

    // Tìm đơn hàng hiện tại
    Order* order = search_order(orderList, maBan);
    
    // Nếu đơn hàng chưa được tạo
    if (order == NULL) {
        printf("[add_dish] Không có đơn hàng cho mã bàn %d. Tạo đơn hàng mới.\n", maBan);
        // Tạo đơn hàng mới
        char timebuf[20];
        char *current_time = get_current_time(timebuf, sizeof(timebuf));
        Order* newOrder = create_order(orderList, maBan, maNV, current_time);
        // Tạo món ăn mới
        Dish* newDish = makeNewDish(maMon, tenMon, giaTien, soLuongDat, current_time, ghiChu);
        
        // Thêm món ăn mới vào danh sách món
        if (newOrder->danhSachMon->headDish == NULL && 
            newOrder->danhSachMon->tailDish == NULL) {
                newOrder->danhSachMon->headDish = newDish;
                newOrder->danhSachMon->tailDish = newDish;
                newOrder->tongSoMon += 1;
                newOrder->tongSoDiaDat += newDish->soLuongDat;
                newOrder->tongTien += newDish->giaTien * newDish->soLuongDat;
        }
        printf("[add_dish] Đã thêm món ăn mới có mã món %s, tên món %s vào danh sách được tạo mới.\n", maMon, tenMon);
        return 1;
    }

    // Nếu đơn hàng đã được tạo
    // 1. Kiểm tra đã tồn tại món ăn trong danh sách món
    Dish* searchDish = search_dish(order->danhSachMon, maMon, tenMon);
    if (searchDish != NULL) { // Nếu đã tồn tại
        searchDish->soLuongDat += soLuongDat;
        searchDish->giaTien = giaTien;
        char timebuf[20];
        char* currentTime = get_current_time(timebuf, sizeof(timebuf));
        strcpy(searchDish->thoiGianCapNhat, currentTime);
        strcpy(searchDish->ghiChu, ghiChu);
        printf("[add_dish] Thêm và cập nhật món ăn có mã món %s, tên món %s thành công.\n", maMon, tenMon);
        return 1;
    } else {
        printf("[add_dish] Món ăn có mã món %s, tên món %s chưa tồn tại trong danh sách món ăn.\n", maMon, tenMon);
    }

    // 2. Nếu món ăn chưa tồn tại trong danh sách món ăn thì thêm vào cuối danh sách món
    char timebuf[20];
    char* currentTime = get_current_time(timebuf, sizeof(timebuf));
    Dish* newDish = makeNewDish(maMon, tenMon, giaTien, soLuongDat, currentTime, ghiChu);

    if (order->danhSachMon->headDish == NULL && 
        order->danhSachMon->tailDish == NULL) {
            order->danhSachMon->headDish = newDish;
            order->danhSachMon->tailDish = newDish;
        } else if (order->danhSachMon->headDish != NULL && 
                order->danhSachMon->tailDish != NULL) {
            order->danhSachMon->tailDish->next = newDish;
            order->danhSachMon->tailDish = newDish;
    }

    order->tongSoMon += 1;
    order->tongSoDiaDat += newDish->soLuongDat;
    order->tongTien += newDish->giaTien * newDish->soLuongDat;

    printf("[add_dish] Thêm món ăn mới có mã món %s, tên món %s thành công.\n", maMon, tenMon);
    return 1;
}

// 5. Hàm cập nhật món ăn, trả món cho khách. Hàm này trả về 1 nếu thành công, 0 nếu thất bại. 
int update_dish(OrderList *orderList, int maBan, char* maMon, char *tenMon, int soLuongTra) {
    if (maBan <= 0) {
        printf("[update_dish] Mã bàn %d không hợp lệ.\n", maBan);
        return 0; // Thất bại
    }
    if (maMon == NULL) {
        printf("[update_dish] Mã món ăn không hợp lệ.\n");
        return 0;
    }
    if (tenMon == NULL) {
        printf("[update_dish] Tên món ăn không hợp lệ.\n");
        return 0;
    }
    if (soLuongTra <= 0) {
        printf("[update_dish] Số lượng trả %d không hợp lệ.\n", soLuongTra);
        return 0; // Thất bại
    }

    Order* order = search_order(orderList, maBan);
    if (order == NULL) {
        printf("[update_dish] Không có đơn hàng cho mã bàn %d.\n", maBan);
        return 0; // Thất bại
    }
    
    // Nếu đơn hàng đã tồn tại
    // 1. Tìm kiếm món ăn
    Dish *searchDish = search_dish(order->danhSachMon, maMon, tenMon);

    char timeBuf[20];
    char* currentTime = get_current_time(timeBuf, sizeof(timeBuf));
    // 2. Kiểm tra món ăn đã tồn tại hay chưa
    if (searchDish != NULL) {
        printf("[update_dish] Ma Mon: %s, Ten Mon %s\n", searchDish->maMon, searchDish->tenMon);
        if (searchDish->soLuongDat < soLuongTra) {
            printf("[update_dish] Số lượng trả %d nhiều hơn số lượng đặt %d\n", soLuongTra, searchDish->soLuongDat);
            searchDish->soLuongTra = searchDish->soLuongDat; // Trả hết số lượng đã đặt
            searchDish->trangThai = DA_LAM_XONG; // Cập nhật trạng thái món ăn
            strcpy(searchDish->thoiGianCapNhat, currentTime);
            printf("[update_dish] Trả hết món ăn có mã %s, tên %s. Cập nhật trạng thái món ăn thành đã làm xong.\n", 
                   searchDish->maMon, searchDish->tenMon);
            // Cập nhật trạng thái đơn hàng
            return 1;
        }
        strcpy(searchDish->thoiGianCapNhat, currentTime);
        searchDish->soLuongTra += soLuongTra;
        if (searchDish->soLuongDat == soLuongTra) {
            // Nếu số lượng trả bằng số lượng đặt thì cập nhật trạng thái món ăn
            searchDish->trangThai = DA_LAM_XONG;
        } else if (searchDish->soLuongDat > soLuongTra) {
            // Nếu số lượng trả nhỏ hơn số lượng đặt thì cập nhật trạng thái món ăn
            searchDish->trangThai = DANG_LAM;
        }
    } else {
        printf("[update_dish] Món ăn chưa tồn tại trong danh sách món ăn. Cập nhật thất bại.\n");
        return 0;
    }

    // Cập nhật trạng thái của đơn hàng
    order->tongSoMonTra += 1;
    order->tongSoDiaTra += soLuongTra;
    strcpy(order->thoiGianCapNhat, currentTime);
    
    return 1;
}

// 6. Huỷ món
int cancel_dish(OrderList* orderList, int maBan, char* maMon, char* tenMon, char* ghiChu) {
    if (maBan <= 0) {
        printf("[cancel_dish] Mã bàn %d không hợp lệ.\n", maBan);
        return 0; 
    }
    if (maMon == NULL) {
        printf("[cancel_dish] Mã món không hợp lệ.\n");
        return 0;
    }

    if (orderList == NULL) {
        printf("[cancel_dish] Danh sách đơn hàng không tồn tại.\n");
        return 0;
    }

    Order* order = search_order(orderList, maBan);
    if (order == NULL) {
        printf("[cancel_dish] Không tồn tại đơn hàng cho mã bàn %d.\n", maBan);
        return 0;
    }

    // Nếu đơn hàng tồn tại
    // 1. Tìm kiếm món ăn trong danh sách món
    Dish* searchDish = search_dish(order->danhSachMon, maMon, tenMon);
    if (searchDish == NULL) {
        printf("[cancel_dish] Món ăn có mã %s không tồn tại trong danh sách món ăn của bàn %d.\n", maMon, maBan);
        return 0; // Thất bại
    }
    
    // 2. Kiểm tra trạng thái món ăn, nếu món ăn đã làm xong hoặc đang làm thì không thể huỷ
    if (searchDish->trangThai == DA_LAM_XONG || 
        searchDish->trangThai == DANG_LAM) {
        printf("[cancel_dish] Không thể huỷ món ăn đang làm hoặc đã làm xong.\n");
        return 0; // Thất bại
    }
    
    // Cập nhật trạng thái món ăn
    searchDish->trangThai = DA_HUY;
    searchDish->soLuongTra = 0; // Số lượng trả về là 0
    strcpy(searchDish->ghiChu, ghiChu);

    char timeBuf[20];
    char* currentTime = get_current_time(timeBuf, sizeof(timeBuf));
    strcpy(searchDish->thoiGianCapNhat, currentTime);

    printf("[cancel_dish] Đã huỷ món ăn có mã %s trong đơn hàng của bàn %d.\n", maMon, maBan);

    // Cập nhật trạng thái đơn hàng
    order->tongSoMon -= 1; // Giảm tổng số món
    order->tongSoDiaDat -= searchDish->soLuongDat; // Giảm tổng số đĩa đặt
    order->tongSoDiaTra -= searchDish->soLuongTra; // Giảm tổng số đĩa trả
    order->tongTien -= searchDish->giaTien * searchDish->soLuongDat; // Giảm tổng tiền
    strcpy(order->thoiGianCapNhat, currentTime);

    // Giải phóng bộ nhớ của món ăn
    //free_dish(order->danhSachMon, searchDish);
    printf("[cancel_dish] Đã huỷ món ăn và cập nhật đơn hàng mã %d thành công.\n", maBan);
    return 1; // Thành công
}