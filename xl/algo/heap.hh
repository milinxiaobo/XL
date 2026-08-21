#pragma once

#include "xl/util/test.hh"

namespace xl::algo {
    template <typename T>
    struct Heap {
        std::size_t s;
        std::vector<T> d;
        Heap(std::size_t s) : s(s) {
            d.resize(s);
        }
        void swap(T& a, T& b) {
            auto t = a;
            a = b;
            b = t;
        }
        void create(const std::vector<T>& a) {
            for (auto i = 0; i < s; ++i) {
                d.at(i) = a.at(i);
            }
            // [0,1,2,3,4,5,6,7,8,9,10,11,12]
            for (int i = s / 2; i >= 0; --i) {
                rf(i);
            }
        }
        void rf(int i) {
            auto l = 2 * i + 1;
            auto r = 2 * i + 2;
            auto c = i;
            if (l < s && d.at(c) > d.at(l)) c = l;
            if (r < s && d.at(c) > d.at(r)) c = r;
            if (c != i) {
                swap(d.at(c), d.at(i));
                rf(c);
            }
        }
        void topk(T v) {
            if (d[0] < v) {
                d[0] = v;
                rf(0);
            }
        }
        static void test() {
            // std::vector<T> a{12,11,10,9,8,7,6,5,4,3,2,1,0};
            std::vector<T> a{0,1,2,3,4,5,6,7,8,9,10,11,12};
            Heap<T> h(7);
            h.create(a);
            xl::util::Test::pv(h.d);
            for (int i = 7; i < a.size(); ++i) {
                h.topk(a[i]);
            }
            xl::util::Test::pv(h.d);
        }
    };
}
