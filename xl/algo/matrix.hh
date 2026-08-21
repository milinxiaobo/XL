#pragma once

namespace xl::algo {
    namespace cuda {
        struct Dim {};
        struct CUDA {
            static CUDA context() {}
            static void test() {
                std::cout << "CUDA test" << std::endl;
            }
        };
    }
}
