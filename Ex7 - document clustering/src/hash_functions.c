#include "hash_functions.h"
#include <stdlib.h> // Cho malloc, free, rand, srand
#include <string.h> // Cho strlen (nếu hàm hash cần biết độ dài chuỗi)
#include <time.h>   // Cho time (để khởi tạo seed cho srand)

// Hàm hash đơn giản (phiên bản djb2)
// Một hàm hash không quá phức tạp nhưng khá phân bố tốt cho chuỗi ký tự.
uint32_t simple_hash(const char* str) {
    uint32_t hash = 5381; // Một số nguyên tố lớn làm giá trị khởi tạo
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

// Hàm hash với seed (phiên bản djb2 biến đổi)
// Thêm seed vào giá trị khởi tạo để tạo ra các hàm hash khác nhau.
uint32_t hash_with_seed(const char* str, uint32_t seed) {
    uint32_t hash = seed; // Khởi tạo hash bằng giá trị seed
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

// Tạo một mảng các seed cho nhiều hàm hash
// Sử dụng rand() để tạo các seed. Lưu ý: rand() không tạo số ngẫu nhiên mạnh về mặt mật mã.
// Đối với MinHash, sự khác biệt và phân bố tốt giữa các hash function là quan trọng hơn.
uint32_t* generate_hash_seeds(int num_hashes) {
    if (num_hashes <= 0) return NULL;

    uint32_t* seeds = (uint32_t*)malloc(num_hashes * sizeof(uint32_t));
    if (seeds == NULL) {
        return NULL; // Lỗi cấp phát bộ nhớ
    }

    // Khởi tạo bộ tạo số ngẫu nhiên chỉ một lần
    // static là để đảm bảo srand chỉ chạy 1 lần duy nhất trong suốt vòng đời chương trình.
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned int)time(NULL)); // Sử dụng thời gian hiện tại làm seed
        seeded = true;
    }

    // Tạo các seed. Cố gắng làm cho chúng khác biệt và có giá trị lớn.
    for (int i = 0; i < num_hashes; i++) {
        // Kết hợp rand() nhiều lần để tạo ra giá trị lớn hơn
        // và cộng thêm 'i' để đảm bảo sự khác biệt nếu rand() lặp lại giá trị
        seeds[i] = (uint32_t)rand() ^ ((uint32_t)rand() << 16) ^ (uint32_t)i;
        // Đảm bảo seed không bằng 0 để tránh trường hợp seed mặc định hoặc có vấn đề trong một số hàm hash
        if (seeds[i] == 0) seeds[i] = 1; 
    }

    return seeds;
}

// Hàm hash MurmurHash2 (phiên bản 32-bit)
// Đây là một hàm hash nhanh và phân bố tốt, thích hợp cho các ứng dụng như bảng băm.
// Mã nguồn dựa trên phiên bản MurmurHash2 của Austin Appleby, điều chỉnh cho C và char*
uint32_t murmur_hash(const char* key, int len, uint32_t seed) {
    // Các hằng số MurmurHash
    const uint32_t m = 0x5bd1e995;
    const int r = 24;

    // Khởi tạo giá trị hash với seed và độ dài dữ liệu
    uint32_t h = seed ^ (uint32_t)len;

    // Trộn 4 byte một lúc vào giá trị hash
    const unsigned char* data = (const unsigned char*)key;

    while (len >= 4) {
        // Đọc 4 byte dưới dạng một số nguyên 32-bit.
        // Cẩn thận với vấn đề căn chỉnh bộ nhớ (alignment) trên một số kiến trúc.
        // Để an toàn hơn, có thể đọc từng byte và ghép lại.
        uint32_t k = *(uint32_t*)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    // Xử lý các byte còn lại (tối đa 3 byte)
    switch (len) {
        case 3: h ^= data[2] << 16;
        case 2: h ^= data[1] << 8;
        case 1: h ^= data[0];
            h *= m;
    };

    // Bước trộn cuối cùng để phân tán bit (avalanche effect)
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}