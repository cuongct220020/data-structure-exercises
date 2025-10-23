# Data Structure Exercises
> **IT3011 - Data Structures & Algorithm - Hanoi University of Science and Technology**

Practical implementations of common data structures (linked lists, trees, hash tables) applied to real-world problems using **C/C++** and **CMake**.

## 📋 Exercises

| # | Topic | Data Structure | Description |
|---|-------|----------------|-------------|
| 1 | Order Management | Singly Linked List | Business logic for order processing |
| 2 | XML Parser | Tree & Stack | Build XML tree and validate tags |
| 3 | Memory Management | Singly Linked List | OS memory allocation (First/Best/Next/Worst Fit) |
| 4 | Cache Management | Doubly Linked List & Hash Table | LRU, MRU, MFU algorithms |
| 5 | Prefix Tree | Trie | Spell checker and autocomplete |
| 6 | Variable Scope | Tree | Compiler scope validation |
| 7 | Document Clustering | MinHash + LSH | Find similar documents |
| 8 | User Session | Red-Black Tree | Web server session management |

## 🚀 Quick Start

### Prerequisites
- Git
- CMake (3.10+)
- C/C++ Compiler (GCC/Clang/MSVC)

### Build & Run

```bash
# Clone repository
git clone https://github.com/cuongct220020/data-structure-exercises.git
cd data-structure-exercises

# Choose and run any exercise (example: ex1)
cd ex1-order-management
mkdir build && cd build
cmake .. && cmake --build .
./main
```

### Build Template
Replace `exN-exercise-name` with any exercise folder:

```bash
cd exN-exercise-name
mkdir build && cd build
cmake .. && cmake --build .
./main
```

## 🧹 Clean Build

```bash
rm -rf build
```

## 📝 License
Educational purpose - IT3011 Course

## 👨‍💻 Author
[@cuongct220020](https://github.com/cuongct220020)
