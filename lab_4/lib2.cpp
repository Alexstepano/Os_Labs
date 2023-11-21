#include <cmath>
#include <iomanip>
#include <iostream>
extern "C" float Square(float A, float B);
extern "C" float Pi(int k)
{
    float pi = 1;

    for (int64_t i = 1; i <= k; ++i) {
        pi *= (static_cast<float>(4 * i * i) / static_cast<float>(4 * i * i - 1));
    }
    return 2 * pi;
}
float Square(float A, float B)
{
    return 0.5 * A * B;
}
