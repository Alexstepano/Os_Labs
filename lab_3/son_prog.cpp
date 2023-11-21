#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
const int MAX_SIZE_STR = 200;
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
int main(int argc, char *argv[])
{
    char *semaph_name = argv[0];
    char *mmapfile_name = argv[1];
    int mmapfile;
    sem_t *semaphore = sem_open(semaph_name, O_RDWR | O_CREAT, 0777);
    int flag = 1;
    while (flag) {
        if (sem_getter(semaphore) == 2) {
            continue;
        }
        mmapfile = shm_open(mmapfile_name, O_RDWR | O_CREAT, 0777);
        error_check(mmapfile, "error: shared memory file_desc");
        struct stat buff;
        fstat(mmapfile, &buff);
        int size = buff.st_size;
        char *m_map = (char *) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, mmapfile, 0);
        char string[MAX_SIZE_STR];
        int str_size = 0;
        for (int ind = 0; ind < size; ++ind) {
            char symb = m_map[ind];
            if (symb == '\0') {
                reverse_str(string, str_size);
                write(fileno(stdout), string, sizeof(char) * str_size);
                write(fileno(stdout), "\n", sizeof(char));
                str_size = 0;
                continue;
            }
            string[str_size] = m_map[ind];
            ++str_size;
        }
        sem_unlink(semaph_name);
        munmap(m_map, size);
        close(mmapfile);
        flag = 0;
    }

    return 0;
}
