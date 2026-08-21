#pragma once

namespace xl::util {
    struct Test {
        template <typename T>
        static void pv(const std::vector<T>& v) {
            std::for_each(v.begin(), v.end(), [](int d) { std::cout << d << "\t"; });
            std::cout << std::endl;
            std::cout << std::flush;
        }
    };
}
