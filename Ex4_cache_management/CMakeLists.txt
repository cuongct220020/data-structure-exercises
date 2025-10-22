cmake_minimum_required(VERSION 3.10)
project(cache_management VERSION 0.1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Thêm thư mục chứa file header
include_directories(include)

# Thêm các file nguồn
set(SOURCES
    src/main.cpp
    src/lru_mru_mfu_cache.cpp
)

# Tạo executable
add_executable(cache_management ${SOURCES})
