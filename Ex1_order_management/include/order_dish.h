#ifndef ORDER_H
#define ORDER_H

#define MAX_NAME 50
#define MAX_NOTE 100

// Trạng thái đơn hàng
typedef enum {
    DANG_PHUC_VU,
    DA_THANH_TOAN,
    DON_HUY
} TrangThaiDonHang;

// Trạng thái món ăn
typedef enum {
    CHUA_LAM,
    DANG_LAM,
    DA_LAM_XONG,
    DA_HUY
} TrangThaiMonAn;

// Cấu trúc món ăn
typedef struct Dish {
    char maMon[20];
    char tenMon[MAX_NAME];
    int soLuongDat;
    int soLuongTra;
    int giaTien;
    char thoiGianTaoMon[20];
    char thoiGianCapNhat[20];
    char ghiChu[MAX_NOTE];
    TrangThaiMonAn trangThai;
    struct Dish* next;
} Dish;

// Cấu trúc danh sách món ăn
typedef struct DishList {
    Dish* headDish;
    Dish* tailDish;
} DishList;

// Cấu trúc đơn hàng
typedef struct Order {
    int maBan;
    char maNhanVien[20];
    int tongSoMon;     // Tổng số món mà khách hàng đã đặt
    int tongSoDiaDat; // Tổng số đĩa mà khách hàng đã đặt
    int tongSoMonTra; // Tổng số món mà nhà bếp đã làm xong
    int tongSoDiaTra; // Tổng số đĩa mà nhà bếp đã làm xong
    long long tongTien;
    char thoiGianTaoDon[20];
    char thoiGianCapNhat[20];
    DishList* danhSachMon;
    TrangThaiDonHang trangThai;
    struct Order* next;
} Order;

// Cấu trúc danh sách đơn hàng
typedef struct {
    Order* headOrder;
    Order* tailOrder;
} OrderList;


// Hàm tạo đối tượng đơn hàng
Order* makeNewOrder(int maBan, char* maNV, char* thoiGianTaoDon);

// Hàm tìm kiếm và hiển thị
Order* search_order(OrderList *orderList, int maBan);
void print_order(OrderList* orderList, int maBan);

// Hàm tạo mới đơn hàng
Order* create_order(OrderList* orderList, int maBan, char* maNV, char* thoiGian);

// Hàm quản lý đơn hàng - trả về 1 nếu thành công, 0 nếu thất bại
int cancel_order(OrderList *orderList, int maBan);

// Hàm xuất hóa đơn
void create_bill(OrderList *orderList, int maBan);

// Hàm quản lý bộ nhớ
OrderList* init_order_list();
void free_order_list(OrderList* orderList);


// Hàm tạo đối tượng món ăn
Dish* makeNewDish(char* maMon, char* tenMon, int giaTien, 
                int soLuongDat, char *thoiGianTaoMon, char* ghiChu);

// Hàm tìm kiếm món ăn
Dish* search_dish(DishList *dishList, char* maMon, char *tenMon);

// Hàm quản lý món ăn - trả về 1 nếu thành công, 0 nếu thất bại
int add_dish(OrderList *orderList, char* maNV, int maBan, 
            char* maMon, char* tenMon, int soLuong, int giaTien, char* ghiChu);
int update_dish(OrderList *orderList, int maBan, char* maMon, char* tenMon, int soLuongTra);
int cancel_dish(OrderList* orderList, int maBan, char* maMon, char* tenMon, char* ghiChu);

// Hàm giải phòng món ăn khỏi danh sách
void free_dish(DishList *dishList, Dish* searchDish);
void free_dish_list(DishList *dishList);

#endif // ORDER_H