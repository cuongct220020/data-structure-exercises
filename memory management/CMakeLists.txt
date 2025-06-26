cmake_minimum_required(VERSION 3.10)
project(memory_management VERSION 0.1.0 LANGUAGES C)

# Thêm thư mục chứa file header
include_directories(include)

# Thêm các file nguồn
set(SOURCES
    src/main.c
    src/mem_alloc.c
    src/buddy_alloc.c
)

# Tạo executable
add_executable(memory_management ${SOURCES})