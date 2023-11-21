#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
const char *FIRST_MEMMAP_NAME = "file1";
const char *SECOND_MEMMAP_NAME = "file2";
const char *SEMAPH_NAME = "sem";
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
void error_check(int inserted_val, char *type_of_error)
{
    if (inserted_val == -1) {
        perror(type_of_error);
        exit(-1);
    }
}
int sem_getter(sem_t *semaphor)
{
    int val;
    sem_getvalue(semaphor, &val);
    return val;
}
void sem_setter(sem_t *semaphor, int val)
{
    while (sem_getter(semaphor) < val) {
        sem_post(semaphor);
    }
    while (sem_getter(semaphor) > val) {
        sem_wait(semaphor);
    }
}
int main()
{


    char *filename;
    size_t len;
    filename = read_string(len);
    int file_desc_1 =
        open(filename, O_WRONLY | O_APPEND | O_CREAT, 0777);// explain 0777!

    error_check(file_desc_1, "error file_desc");
    filename = read_string(len);
    int file_desc_2 =
        open(filename, O_WRONLY | O_APPEND | O_CREAT, 0777);// explain 0777!
    error_check(file_desc_2, "error file_desc");
    free(filename);
    int FIRST_MEMMAP_FILE, SECOND_MEMMAP_FILE;
    FIRST_MEMMAP_FILE = shm_open(FIRST_MEMMAP_NAME, O_RDWR | O_CREAT, 0777);
    error_check(FIRST_MEMMAP_FILE, "error: shared memory file_desc");
    SECOND_MEMMAP_FILE = shm_open(SECOND_MEMMAP_NAME, O_RDWR | O_CREAT, 0777);
    error_check(SECOND_MEMMAP_FILE, "error: shared memory file_desc");
    sem_unlink(SEMAPH_NAME);
    sem_t *semaphore = sem_open(SEMAPH_NAME, O_CREAT, 0777, 2);
    pid_t c_pid = fork();
    error_check(c_pid, "fork");
    if (c_pid > 0) {


        pid_t c_a_pid = fork();
        error_check(c_a_pid, "fork");

        if (c_a_pid > 0) {
            close(file_desc_1);
            close(file_desc_2);
            if (sem_getter(semaphore) == 2) {
                char *first_mmap = (char *) mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, FIRST_MEMMAP_FILE, 0);

                char *second_mmap = (char *) mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, SECOND_MEMMAP_FILE, 0);

                if (first_mmap == MAP_FAILED) {
                    perror("Error with creating first_map");
                    return -1;
                }

                if (second_mmap == MAP_FAILED) {
                    perror("Error with creating second_map");
                    return -1;
                }
                int f_pos = 0;
                int s_pos = 0;
                int f_len = 0;
                int s_len = 0;
                char *string;
                int flag = 0;
                size_t len_s = 0;
                while ((string = read_string(len_s)) != nullptr) {
                    if (!flag) {
                        f_len += len_s + 1;
                        error_check(ftruncate(FIRST_MEMMAP_FILE, f_len), "Ftruncate error");
                        for (int char_ind = 0; char_ind < len_s; ++char_ind) {
                            first_mmap[f_pos] = string[char_ind];
                            ++f_pos;
                        }
                        first_mmap[f_pos] = '\0';
                        ++f_pos;
                    }
                    else {
                        s_len += len_s + 1;
                        error_check(ftruncate(SECOND_MEMMAP_FILE, s_len), "Ftruncate error");
                        for (int char_ind = 0; char_ind < len_s; ++char_ind) {
                            second_mmap[s_pos] = string[char_ind];
                            ++s_pos;
                        }
                        second_mmap[s_pos] = '\0';
                        ++s_pos;
                    }
                    flag = (flag + 1) % 2;
                }
                sem_setter(semaphore, 1);
                struct stat first_buf, second_buf;
                fstat(FIRST_MEMMAP_FILE, &first_buf);
                fstat(SECOND_MEMMAP_FILE, &second_buf);
                munmap(first_mmap, first_buf.st_size);
                munmap(second_mmap, second_buf.st_size);
            }
            close(SECOND_MEMMAP_FILE);
            close(FIRST_MEMMAP_FILE);
        }
        else {
            close(file_desc_1);
            close(FIRST_MEMMAP_FILE);
            error_check(dup2(file_desc_2, STDOUT_FILENO), "error dup2");
            close(file_desc_2);
            close(SECOND_MEMMAP_FILE);


            error_check(execl("./son.out", SEMAPH_NAME, SECOND_MEMMAP_NAME, NULL), "error execl");
            return 0;
        }
    }
    else {
        close(file_desc_2);
        close(SECOND_MEMMAP_FILE);
        error_check(dup2(file_desc_1, STDOUT_FILENO), "error dup2");
        close(file_desc_1);
        close(FIRST_MEMMAP_FILE);
        error_check(execl("./son.out", SEMAPH_NAME, FIRST_MEMMAP_NAME, NULL), "error execl");
        return 0;
    }
    sem_close(semaphore);
    sem_destroy(semaphore);
    return 0;
}
