#include <stdio.h>

// Hàm toàn cục
void greet() {
    printf("Hello from a global function!\n");
}

int main() {
    printf("Attempting to use an undefined variable...\n");

    // This line will cause a compilation error because 'undefined_var' is not declared
    printf("Value of undefined_var: %d\n", undefined_var);

    int defined_var = 10;
    printf("Value of defined_var: %d\n", defined_var);

    // Gọi hàm toàn cục
    greet();

    return 0;
}