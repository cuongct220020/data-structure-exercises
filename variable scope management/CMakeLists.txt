cmake_minimum_required(VERSION 3.10)
project(variables_scope VERSION 0.1.0 LANGUAGES C)

# Thêm thư mục chứa file header cho target
include_directories(include)

# Thêm các file nguồn
set(SOURCES
    src/main.c
    src/scopetree.c
)

# Tạo executable
add_executable(variables_scope ${SOURCES})