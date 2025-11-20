#include <iostream>

int main() {
#if __cplusplus >= 201703L
    std::cout << "C++17 or later\n";
#else
    std::cout << "Older than C++17\n";
#endif
}
