cmake_minimum_required(VERSION 3.10)
project(order_management VERSION 0.1.0 LANGUAGES C)

# Thêm thư mục chứa file header
include_directories(include)

# Thêm các file nguồn
set(SOURCES
    src/main.c
    src/order.c
    src/dish.c
    src/utility.c
)

# Tạo executable
add_executable(order_management ${SOURCES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
