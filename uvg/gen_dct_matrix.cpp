#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>

/*
 * Writes the 8x8 DCT matrix to stdout.
 */
int main() {
    const float n = 8;
    const auto rootOneOverN = 1.0 / sqrt(n);
    const auto rootTwoOverN = sqrt(2.0 / n);

    std::cout << "{ ";

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == 0) {
                std::cout << rootOneOverN;
            }
            else {
                const auto multiplier = (2*j + 1)*i / (2*n);
                std::cout << (rootTwoOverN * cos(multiplier * M_PI));
            }
            if (i != n-1 || j != n-1) {
                std::cout << ", ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << "}" << std::endl;

    return 0;
}
