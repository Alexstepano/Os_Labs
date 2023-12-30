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
bool try_to_solve(vector<int> &alphabet, string &answer_offline, sem_t *semaphore)
{
    bool unsolved = true;
    string guess;
    struct timespec ts, ts_cur;
    int state;
    throw_error(clock_gettime(CLOCK_REALTIME, &ts), -1, "Error in clock_gettime");
    ts.tv_sec += 1200;////
    while (unsolved) {
        usleep(25000);
        sem_getvalue(semaphore, &state);
        throw_error(clock_gettime(CLOCK_REALTIME, &ts_cur), -1, "Error in clock_gettime");
        int bull = 0, cow = 0;
        if (state == 3 || ts_cur.tv_sec >= ts.tv_sec) {
            cout << "Too slow.You lost" << endl;
            return false;
        }
        cout << "input your guess";
        cin >> guess;
        if (guess.size() != 4) {
            cout << "The word have exectly 4 letters" << endl;
            continue;
        }
        if (guess == answer_offline) {
            set_semaphore(semaphore, &state, 4);
            unsolved = false;
        }
        else {
            for (size_t i = 0; i < guess.size(); ++i) {
                if (guess[i] == answer_offline[i]) {
                    ++bull;
                }
                if (alphabet[guess[i] - 'a'] > 0) {
                    ++cow;
                }
            }
            cout << "Cows:" << cow << " "
                 << "Bulls:" << bull;
        }
    }

    cout << "Congrats" << endl;
    return true;
}
void getanswer_form_server(const string &name, int64_t n, vector<int> &alphabet, string &answer_offline)
{
    string sem_name = to_string(n) + name + to_string(0);
    string client_host = sem_name + "_host";
    string host_client = "host_" + sem_name;
    struct timespec ts, ts_cur;
    sem_t *semaphore = sem_open(sem_name.c_str(), O_RDWR | O_CREAT, 0777);
    check_semaphore(semaphore, SEM_FAILED, "Error in child semaphore");
    int state = 1;
    sem_getvalue(semaphore, &state);
    set_semaphore(semaphore, &state, 1);
    throw_error(clock_gettime(CLOCK_REALTIME, &ts), -1, "Error in clock_gettime");
    ts.tv_sec += 300;//
    while (1) {
        sem_getvalue(semaphore, &state);
        usleep(25000);
        throw_error(clock_gettime(CLOCK_REALTIME, &ts_cur), -1, "Error in clock_gettime");
        if (state == 2) {
            answer_offline = get_from_mmap(host_client, "Error to recive message from host", 1);
            for (size_t u = 0; u < answer_offline.size(); ++u) {
                alphabet[answer_offline[u] - 'a'] += 1;
            }
            cout << try_to_solve(alphabet, answer_offline, semaphore) << endl;
            cout << "POLUCHIL -BREAK" << endl;
            break;
        }
        if (ts_cur.tv_sec >= ts.tv_sec) {
            cout << "Sorry - your host can't give you a word,try to find a new game" << endl;
            break;
        }
    }
}

void giveanswer_to_server(const string &name, int64_t n, vector<int> &alphabet, string &answer_offline, int64_t real_number)
{
    vector< string > sem_name(real_number);
    vector< sem_t * > semaphore(real_number);
    vector<int> state(real_number + 1);
    vector< string > client_host(real_number);
    vector< string > host_client(real_number);
    struct timespec ts, ts_cur;
    for (int64_t i = 1; i < real_number; ++i) {
        sem_name[i] = to_string(i) + name + to_string(0);
        client_host[i] = sem_name[i] + "_host";
        host_client[i] = "host_" + sem_name[i];
        semaphore[i] = sem_open(sem_name[i].c_str(), O_RDWR | O_CREAT, 0777, 1);
        check_semaphore(semaphore[i], SEM_FAILED, "Error in child semaphore");
        state[i] = 2;
        sem_getvalue(semaphore[i], &state[i]);

        set_semaphore(semaphore[i], &state[i], 2);
    }
    cout << "FFFF" << endl;

    while (1) {
        cin >> answer_offline;
        if (answer_offline.size() != 4) {
            cout << "Error:big or small - only 4 letters" << endl;
            continue;
        }
        bool flag = false;
        for (int64_t i = 0; i < answer_offline.size(); ++i) {
            if (!(answer_offline[i] >= 'a' && answer_offline[i] <= 'z')) {
                cout << "Error:Unnalowed character " << endl;
                flag = true;
                break;
            }
        }
        if (flag) {
            continue;
        }
        else {
            break;
        }
    }
    throw_error(clock_gettime(CLOCK_REALTIME, &ts), -1, "Error in clock_gettime");
    ts.tv_sec += 300;//
    while (1) {
        sem_getvalue(semaphore[real_number - 1], &state[real_number - 1]);
        throw_error(clock_gettime(CLOCK_REALTIME, &ts_cur), -1, "Error in clock_gettime");
        usleep(25000);
        if (state[real_number - 1] == 1 || ts_cur.tv_sec >= ts.tv_sec) {
            cout << "VSAL -BREAK" << endl;
            for (int64_t i = 1; i < real_number; ++i) {
                give_to_mmap(answer_offline, host_client[i], "Error to transpot word to player", semaphore[i], &state[i], 1);
                sem_getvalue(semaphore[i], &state[i]);
                set_semaphore(semaphore[i], &state[i], 2);
            }


            break;
        }
    }
    int vinning_posititon = 0;
    bool solved = false;
    throw_error(clock_gettime(CLOCK_REALTIME, &ts), -1, "Error in clock_gettime");
    ts.tv_sec += 1200;//
    while (1) {
        usleep(25000);
        throw_error(clock_gettime(CLOCK_REALTIME, &ts_cur), -1, "Error in clock_gettime");
        if (ts_cur.tv_sec >= ts.tv_sec) {
            solved = true;
            vinning_posititon = 0;
        }
        for (int64_t i = 1; i < real_number; ++i) {
            usleep(25000);
            if (solved) {
                set_semaphore(semaphore[i], &state[i], 3);
            }
            sem_getvalue(semaphore[i], &state[i]);
            if (state[i] == 4) {
                vinning_posititon = i;
                cout << "Player" << i << "Guessed it first" << endl;
                solved = true;
            }
        }
        for (int64_t i = 1; i < vinning_posititon; ++i) {
            if (solved) {
                set_semaphore(semaphore[i], &state[i], 3);
            }
        }
        if (solved) { break; }
    }
}


int main()
{
    // Создание общего семафора
    string general_sem_name = "general.sem";
    sem_t *general_semaphore = sem_open(general_sem_name.c_str(), O_RDWR | O_CREAT, 0777);
    int general_state;
    string client_server_general = "client_server_general";
    string server_client_general = "server_client_general";
    string name;
    string real_name;
    int64_t real_number;
    int64_t n = 0;
    while (1) {
        sem_getvalue(general_semaphore, &general_state);
        usleep(20000);
        if (general_state == 1) {
            printf("Enter the name of the game and the number of players - if the game has already been created, put the number of players at 1: ");
            int64_t n = 0;
            string num;
            cin >> name >> num;
            bool bad_inp = false;
            for (size_t i = 0; i < num.size(); ++i) {
                if (!(num[i] >= '0' && num[i] <= '9')) {
                    bad_inp = true;
                    break;
                }
            }
            if (bad_inp) {
                continue;
            }
            if (num[0] != 'e') {
                n = stoll(num);
            }

            real_name = name;
            real_number = n;
            name += " ";
            name += num;
            give_to_mmap(name, client_server_general, "general shm_open from child", general_semaphore, &general_state, 1);
            set_semaphore(general_semaphore, &general_state, 0);
            usleep(25000);
        }
        else if (general_state == 2) {
            string is_verificate = get_from_mmap(server_client_general, "Error in server_client_general", 1);
            cout << is_verificate << "\n";
            set_semaphore(general_semaphore, &general_state, 1);
            if ("Registration faild: game is full of guessers or number of players is too small" == is_verificate || is_verificate == "Delete was successful: Its free now") {
                continue;
            }
            auto it = find(is_verificate.begin(), is_verificate.end(), '#');
            if (it != is_verificate.end()) ++it;
            while (it != is_verificate.end()) {
                n = n * 10 + ((*it) - '0');
                ++it;
            }
            cout << n << endl;
            break;
        }
    }

    string answer_offline = "";
    vector<int> alphabet(26, 0);
    if (n != 0) {
        getanswer_form_server(real_name, n, alphabet, answer_offline);
    }
    else {
        giveanswer_to_server(real_name, n, alphabet, answer_offline, real_number);
    }
    general_sem_name = "general.sem";
    general_semaphore = sem_open(general_sem_name.c_str(), O_RDWR | O_CREAT, 0777);
    while (1) {
        sem_getvalue(general_semaphore, &general_state);
        usleep(20000);
        if (general_state == 1) {

            int64_t n = 0;

            name = real_name + " e";
            cout << name << endl;
            give_to_mmap(name, client_server_general, "general shm_open from child", general_semaphore, &general_state, 1);
            set_semaphore(general_semaphore, &general_state, 0);
            usleep(25000);
        }
        else if (general_state == 2) {
            string is_verificate = get_from_mmap(server_client_general, "Error in server_client_general", 1);
            cout << is_verificate << "\n";
            set_semaphore(general_semaphore, &general_state, 1);

            break;
        }
    }
    return 0;
}
