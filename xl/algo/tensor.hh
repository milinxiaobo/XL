#pragma once

#include "xl/util/test.hh"

namespace xl::algo {
    struct Shape {
        std::vector<std::size_t> s;
        Shape(std::initializer_list<std::size_t> l) : s(l) {

        }
        std::size_t dims() {
            return s.size();
        }
        std::size_t accumulation() {
            return std::accumulate(s.begin(), s.end(), 1, std::multiplies<std::size_t>());
        }
    };
    struct Stride {
        std::vector<std::size_t> s;
        Stride(std::initializer_list<std::size_t> l) : s(l) {
            for (auto i = std::next(s.rbegin()); i != s.rend(); ++i) {
                (*i) *= (*std::prev(i));
            }
        }
        Stride(const Shape* t) {

        }
        std::size_t dims() {
            return s.size();
        }
        std::size_t accumulation() {
            return std::accumulate(s.begin(), s.end(), 1, std::multiplies<std::size_t>());
        }
        static void test() {
            Stride stride{4,3,2,1};
            xl::util::Test::pv(stride.s);
        }
    };
    struct Layout {};
    struct Tensor {};
}
