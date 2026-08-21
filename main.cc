#include <iostream>

#include "xl/include.hh"
#include "xl/algo/heap.hh"
#include "xl/algo/tensor.hh"
#include "xl/algo/tree.hh"

int main(int args, char* argv[])
{
    std::cout << "\t\x20start...\x20\n";
    {
        std::cout << "\tHello XL.\n";
        xl::algo::Tree<int>::test();
        xl::algo::Stride::test();
        xl::algo::Heap<int>::test();
    }
    std::cout << R"(end...)" << std::endl;
    return 0;
}
