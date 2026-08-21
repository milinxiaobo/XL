#include <iostream>

#include "xl/algo/tree.hh"

int main(int args, char* argv[])
{
    std::cout << "\t\x20start...\x20\n";
    {
        std::cout << "\tHello XL.\n";
        xl::algo::Tree<int>::test();
    }
    std::cout << R"(end...)" << std::endl;
    return 0;
}
