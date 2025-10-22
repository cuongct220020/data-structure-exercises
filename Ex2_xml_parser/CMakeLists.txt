cmake_minimum_required(VERSION 3.10)
project(xml VERSION 0.1.0 LANGUAGES C)

# Thêm thư mục chứa file header
include_directories(include)

# Thêm các file nguồn
set(SOURCES
    src/main.c
    src/tagstack.c
    src/xmlparse.c
)

# Tạo executable
add_executable(xml_parse ${SOURCES})