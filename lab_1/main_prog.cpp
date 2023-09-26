#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
char *read_string(size_t &string_len)
{
    string_len = 0;
    size_t string_cap = 1;
    char *string = (char *) malloc(sizeof(char));
    char c = getchar();
    if (c == '\n') c = getchar();
    if (c == EOF) return nullptr;
    while (c != '\n') {
        string[string_len] = c;
        ++string_len;
        if (string_len >= string_cap) {
            string_cap *= 2;
            string = (char *) realloc(string, string_cap * sizeof(char));
        }
        c = getchar();
        if (c == EOF) return nullptr;
    }
    string[string_len] = '\0';
    return string;
}
int main()
{


    char *filename;
    size_t len;
    filename = read_string(len);
    int file_desc_1 =
        open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644);// explain 0777!

    if (file_desc_1 == -1) {
        perror("error file_desc");
        return -1;
    }
    filename = read_string(len);
    int file_desc_2 =
        open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644);// explain 0777!
    if (file_desc_2 == -1) {
        perror("error file_desc");
        return -1;
    }
    free(filename);
    int first_pipe_des[2];
    int sec_pipe_des[2];
    if (pipe(first_pipe_des) == -1) {
        perror("pipe");
        return -1;
    }
    if (pipe(sec_pipe_des) == -1) {
        perror("pipe");
        return -1;
    }

    pid_t c_pid = fork();
    if (c_pid == -1) {
        perror("fork");
        return -1;
    }
    else if (c_pid > 0) {
        close(first_pipe_des[0]);


        pid_t c_a_pid = fork();
        if (c_a_pid == -1) {
            perror("fork");
            return -1;
        }
        else if (c_a_pid > 0) {
            close(sec_pipe_des[0]);
            close(file_desc_1);
            close(file_desc_2);
            char *string;
            int flag = 0;
            size_t len_s = 0;
            char p = 'a';
            while ((string = read_string(len_s)) != nullptr) {
                if (!flag) {
                    write(first_pipe_des[1], &p, 1);
                    write(first_pipe_des[1], &len_s, sizeof(len_s));
                    write(first_pipe_des[1], string, sizeof(char) * len_s);
                }
                else {
                    write(sec_pipe_des[1], &p, 1);
                    write(sec_pipe_des[1], &len_s, sizeof(len_s));
                    write(sec_pipe_des[1], string, sizeof(char) * len_s);
                }
                flag = (flag + 1) % 2;
                free(string);
            }
            p = 'e';
            write(first_pipe_des[1], &p, 1);
            write(sec_pipe_des[1], &p, 1);
            close(first_pipe_des[1]);
            close(sec_pipe_des[1]);
        }
        else {
            close(sec_pipe_des[1]);
            close(first_pipe_des[1]);
            close(first_pipe_des[0]);
            close(file_desc_1);
            if (dup2(file_desc_2, STDOUT_FILENO) == -1) {
                perror("error dup2");
                return -1;
            }
            close(file_desc_2);

            if (dup2(sec_pipe_des[0], STDIN_FILENO) == -1) {
                perror("error dup2");
                return -1;
            }
            close(sec_pipe_des[0]);


            execl("./son.out", NULL);
            return 0;
        }
    }
    else {
        close(first_pipe_des[1]);
        close(sec_pipe_des[0]);
        close(sec_pipe_des[1]);
        close(file_desc_2);
        if (dup2(file_desc_1, STDOUT_FILENO) == -1) {
            perror("error dup2");
            return -1;
        }
        close(file_desc_1);
        if (dup2(first_pipe_des[0], STDIN_FILENO) == -1) {
            perror("error dup2");
            return -1;
        }
        close(first_pipe_des[0]);

        execl("./son.out", NULL);
        return 0;
    }
    return 0;
}
