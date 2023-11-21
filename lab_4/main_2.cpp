#include <cmath>
#include <dlfcn.h>
#include <iomanip>
#include <iostream>
int main()
{
    int lib_num = 0;
    std::cout << "LIB_NUM 1 or 2" << std::endl;
    std::cin >> lib_num;
    if (lib_num != 1 && lib_num != 2) {
        perror("not a lib numbear");
        return -1;
    }
    int command = 0;
    const char *libs_list[2] = {"./lib1.so", "./lib2.so"};
    void *lib_header;
    --lib_num;
    lib_header = dlopen(libs_list[lib_num], RTLD_LAZY);
    if (lib_header == NULL) {
        perror(dlerror());
        return -1;
    }
    float (*Square)(float A, float B);
    float (*Pi)(int k);
    Square = (float (*)(float, float)) dlsym(lib_header, "Square");
    if (Square == NULL) {
        perror(dlerror());
        return -1;
    }
    Pi = (float (*)(int)) dlsym(lib_header, "Pi");
    if (Pi == NULL) {
        perror(dlerror());
        return -1;
    }
    while (std::cin >> command) {
        if (command == 1) {
            std::cout << "insert A and B" << std::endl;
            float a, b;
            std::cin >> a >> b;
            std::cout << std::setprecision(10) << Square(a, b) << std::endl;
        }
        else if (command == 2) {
            std::cout << "Insert K" << std::endl;
            int k;
            std::cin >> k;
            std::cout << std::setprecision(10) << Pi(k) << std::endl;
        }
        else if (command == 0) {

            if (dlclose(lib_header) != 0) {
                perror(dlerror());
                return -1;
            }
            lib_num = (lib_num + 1) % 2;
            lib_header = dlopen(libs_list[lib_num], RTLD_LAZY);
            if (lib_header == NULL) {
                perror(dlerror());
                return -1;
            }
            Square = (float (*)(float, float)) dlsym(lib_header, "Square");
            if (Square == NULL) {
                perror(dlerror());
                return -1;
            }
            Pi = (float (*)(int)) dlsym(lib_header, "Pi");
            if (Pi == NULL) {
                perror(dlerror());
                return -1;
            }
            std::cout << "switched libs" << std::endl;
        }
    }
    if (dlclose(lib_header) != 0) {
        perror(dlerror());
        return -1;
    }

    return 0;
}
