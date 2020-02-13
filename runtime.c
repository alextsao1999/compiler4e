//
// Created by Alex on 2020/2/13.
//

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define true 1
#define false 0
typedef char bool;
extern int start_main();

void println(char *str) {
    if (str)
        puts(str);
    puts("\n");
}

void print_int(int value) {
    printf("%d", value);
}

int main() {

    return start_main();
}
