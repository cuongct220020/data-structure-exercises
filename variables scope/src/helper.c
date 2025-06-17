#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "helper.h"
#include "scopetree.h"

void print_indentation(int level) {
    for (int i = 0; i < level; i++) {
        printf("  "); // 2 khoảng trắng cho mỗi cấp độ
    }
}