#include <algorithm>
#include <fcntl.h>
#include <iostream>
#include <limits.h>
#include <map>
#include <semaphore.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
using namespace std;

void throw_error(int func, int number_of_error, string error_output)
{
    if (func == number_of_error) {
        cerr << error_output << endl;
        exit(-1);
    }
}

void check_semaphore(sem_t *semaphore, sem_t *error, string error_output)
{
    if (semaphore == error) {
        cerr << error_output << endl;
    }
}

void give_to_mmap(string input, string mmap_name, string error_output, sem_t *semaphore, int *state, bool close_file)
{
    int size_ = input.size();
    int file = shm_open(mmap_name.c_str(), O_RDWR | O_CREAT, 0777);
    throw_error(file, -1, error_output);
    ftruncate(file, size_);
    char *mapped = (char *) mmap(NULL, size_, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
    memset(mapped, '\0', size_);
    sprintf(mapped, "%s", input.c_str());
    int getvalue_error = sem_getvalue(semaphore, state);
    throw_error(getvalue_error, -1, "general sem_getvalue");
    if (close_file) {
        munmap(mapped, size_);
        close(file);
    }
}

string get_from_mmap(string mmap_name, string error_message_file, bool close_file)
{
    int file = shm_open(mmap_name.c_str(), O_RDWR | O_CREAT, 0777);
    throw_error(file, -1, error_message_file);
    struct stat stat_buffer;

    if (fstat(file, &stat_buffer) == -1) {
        cout << "-1\n";
        return "Fail";
    }

    int size = stat_buffer.st_size;
    void *mapped = mmap(NULL,
                        size,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED,
                        file,
                        0);

    if (mapped == MAP_FAILED) {
        cout << "MMAP FAIL" << endl;
        return "Fail";
    }
    std::string str(static_cast< const char * >(mapped), stat_buffer.st_size);

    if (close_file) {
        close(file);

        if (munmap(mapped, size) == -1) {
            cout << "MUNMMAP FAIL" << endl;
            return "fail";
        };
    }

    return str;
}

void set_semaphore(sem_t *semaphore, int *state, int value)
{
    while (++(*state) < value + 1) {
        sem_post(semaphore);
    }
    while (--(*state) > value) {
        sem_wait(semaphore);
    }
}
string get_word(string a)
{
    string result;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] == ' ') {
            break;
        }
        result += a[i];
    }
    return result;
}

int main()
{

    map<string, vector<string>> games;
    while (1) {
        string general_sem_name = "general.sem";
        sem_unlink(general_sem_name.c_str());
        sem_t *general_semaphore = sem_open(general_sem_name.c_str(), O_CREAT, 0777, 1);
        int general_state = 1;

        string start_game_sem = "start_game";
        sem_unlink(start_game_sem.c_str());

        sem_t *start_game_semaphore = sem_open(start_game_sem.c_str(), O_CREAT, 0777, 0);
        check_semaphore(start_game_semaphore, SEM_FAILED, "Error in start game semaphore");

        string client_server_general = "client_server_general";
        string server_client_general = "server_client_general";
        //
        int login_count_for_game = 0;
        while (1) {
            bool not_faild = true;
            sem_getvalue(general_semaphore, &general_state);

            if (general_state == 0) {
                string name_and_num = get_from_mmap(client_server_general, "Error in open file", 1);
                string name = get_word(name_and_num);
                string number = name_and_num.substr(name.size());
                cout << "F" << number << endl;
                int64_t num = 0;
                if (number[1] != 'e')
                    num = stoll(number);

                cout << name << " is joined by another player"
                     << "\n";
                string temp_i = number;

                if (number[1] == 'e') {
                    if (games.find(name) != games.end()) {
                        games.erase(name);
                    }
                    give_to_mmap("Delete was successful: Its free now", server_client_general, "Bad in server_client_general", general_semaphore, &general_state, 1);
                }
                else if (games.find(name) != games.end() && games[name].back() != to_string(games[name].size() - 1)) {
                    for (size_t i = 0; i < games[name].size(); ++i) {
                        if (to_string(i) != games[name][i]) {
                            games[name][i] = to_string(i);
                            temp_i = games[name][i];
                            cout << name << games[name][i] << " Player" << endl;
                            break;
                        }
                    }
                    give_to_mmap("Registration was successful: You are a guesser #" + temp_i, server_client_general, "Bad in server_client_general", general_semaphore, &general_state, 1);
                }
                else if (num >= 2 && games.find(name) == games.end()) {
                    games[name].resize(num, "F");
                    games[name][0] = to_string(0);
                    give_to_mmap("Registration was successful: You are a riddler #0", server_client_general, "Bad in server_client_general", general_semaphore, &general_state, 1);
                }
                else {
                    not_faild = false;
                    give_to_mmap("Registration faild: game is full of guessers or number of players is too small", server_client_general, "Bad in server_client_general", general_semaphore, &general_state, 1);
                }

                set_semaphore(general_semaphore, &general_state, 2);

                usleep(25000);
                if (not_faild && games.find(name) != games.end() && games[name].back() == to_string(games[name].size() - 1)) {
                    cout << "THE GAME WILL BEGIN" << endl;
                    break;
                }
            }
            else {
                usleep(500000);
            }
        }
        //
        sem_close(general_semaphore);
        sem_destroy(general_semaphore);
    }


    return 0;
}
