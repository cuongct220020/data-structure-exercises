#include <stdio.h>

/* Kết quả in ra màn hình
    Inside main: global_var = 5, main_var = 50
    Inside myFunction: global_var = 10, func_var = 20, param1 = 100
    Inside local scope 1: global_var = 10, func_var = 25, local_var_1 = 30
    Inside local scope 2: global_var = 15, local_var_2 = 40
    Back in myFunction: global_var = 10, func_var = 20
    Back in main: global_var = 5, main_var = 50
*/

int global_var = 10; // Biến toàn cục

void myFunction(int param1) { // Phạm vi hàm
    int func_var = 20;

    printf("Inside myFunction: global_var = %d, func_var = %d, param1 = %d\n", global_var, func_var, param1);

    { // Phạm vi cục bộ 1
        int local_var_1 = 30;
        int func_var = 25; // Shadowing func_var
        printf("Inside local scope 1: global_var = %d, func_var = %d, local_var_1 = %d\n", global_var, func_var, local_var_1);

        { // Phạm vi cục bộ 2
            int local_var_2 = 40;
            int global_var = 15; // Shadowing global_var
            printf("Inside local scope 2: global_var = %d, local_var_2 = %d\n", global_var, local_var_2);
        } // Kết thúc phạm vi cục bộ 2
    } // Kết thúc phạm vi cục bộ 1

    printf("Back in myFunction: global_var = %d, func_var = %d\n", global_var, func_var); // func_var trở lại 20
}

int main() { // Phạm vi hàm main
    int main_var = 50;
    int global_var = 5; // Shadowing global_var

    printf("Inside main: global_var = %d, main_var = %d\n", global_var, main_var);

    myFunction(100);

    printf("Back in main: global_var = %d, main_var = %d\n", global_var, main_var);

    return 0;
}