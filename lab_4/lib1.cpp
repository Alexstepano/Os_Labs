#include <cmath>
#include <iomanip>
#include <iostream>
extern "C" float Square(float A, float B);
extern "C" float Pi(int k);
float Pi(int k)
{
    float pi = 0;
    int sigh = 1, denominator = 1;
    for (int i = 0; i < k; ++i) {
        pi += sigh * (4.0 / static_cast<float>(denominator));
        sigh *= -1;
        denominator += 2;
    }
    return pi;
}
float Square(float A, float B)
{
    return A * B;
}
