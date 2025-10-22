cmake_minimum_required(VERSION 3.10)
project(Document_Clustering_MinHash_LSH)

# Include directories
include_directories(include)

# Source files
file(GLOB SOURCES "src/*.c")

# Create executable
add_executable(document_clustering ${SOURCES})