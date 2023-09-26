#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
void reverse_str(char str[], size_t length)
{
    char tempo;
    for (size_t i = 0; i < length; ++i) {
        tempo = str[length - 1];
        str[length - 1] = str[i];
        str[i] = tempo;
        --length;
    }
}

int main()
{

    char a = ' ';
    size_t s_len;
    read(0, &a, sizeof(char));
    while (a != 'e') {
        read(0, &s_len, sizeof(s_len));
        char string[s_len];
        read(0, &string, sizeof(char) * s_len);
        reverse_str(string, s_len);
        write(1, string, sizeof(char) * s_len);
        write(1, "\n", 1);

        read(0, &a, sizeof(char));
    }

    return 0;
}
