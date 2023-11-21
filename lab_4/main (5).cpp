#include <cmath>
#include <iomanip>
#include <iostream>
extern "C" float Square(float A, float B);
extern "C" float Pi(int k);
int main()
{
    int command = 0;
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
        else {
            std::cout << "UNALLOWED COMMAND" << std::endl;
        }
    }
    return 0;
}
