#include <iostream>

void printIfPositive(int i)
{
    std::cout << "Surround and Unwrap me!";
}

namespace {
    void example() {
        printIfPositive(3);
    }
}