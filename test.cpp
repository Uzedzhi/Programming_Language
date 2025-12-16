#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

union en {
    size_t value;
    double lf;
};

void display_binary(unsigned int integer, int n_bits) {
    for (int i = n_bits - 1; i >= 0; i--) {
        if ((i + 1) % 8 == 0)
            putchar(' ');
        if ((integer >> i) & 1) {
            putchar('1');
        } else {
            putchar('0');
        }
    }
    printf("\n");
}

int main(void) {

    char str[] = "■";
    char *buf = "привет";
    wchar_t *bufrus = (wchar_t *) buf;
    fprintf(stderr, "%s %d, %c, %c, %c, %c", bufrus, wcslen(bufrus), bufrus[0], bufrus[1], bufrus[2], bufrus[3]);
    return 0;
}