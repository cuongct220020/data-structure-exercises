cmake_minimum_required(VERSION 3.10)
project(cache_management VERSION 0.1.0 LANGUAGES C)

# Thêm thư mục chứa file header
include_directories(include)

# Thêm các file nguồn
set(SOURCES
    src/main.c
    src/lru_cache.c
)

# Tạo executable
add_executable(cache_management ${SOURCES})