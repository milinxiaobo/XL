#pragma once

#include <algorithm>
#include <deque>
#include <vector>

namespace xl::algo {
    template <typename T>
    struct Node {
        T d;
        Node* l = nullptr;
        Node* r = nullptr;
        Node()  = delete;
        Node(T t) : d(t), l(nullptr), r(nullptr) {}
    };
    template <typename T>
    struct Tree {
        Node<T>* r = nullptr;
        Tree() = default;
        ~Tree() {
            _del_node(r);
            r = nullptr;
        }
        void _del_node(Node<T>* n) {
            if (n == nullptr) return;
            _del_node(n->l);
            n->l = nullptr;
            _del_node(n->r);
            n->r = nullptr;
            delete n;
        }
        void from_pre_in_order(const std::vector<T>& p, const std::vector<T>& i) {
            int pl = 0, pr = p.size() - 1;
            int il = 0, ir = i.size() - 1;
            r = _from_pre_in_order(p, i, pl, pr, il, ir);
        }
        Node<T>* _from_pre_in_order(const std::vector<T>& p, const std::vector<T>& i, int pl, int pr, int il, int ir) {
            if (pl > pr || il > ir) return nullptr;
            auto d = p[pl];
            auto n = new Node<T>(d);
            auto j = std::find(i.begin() + il, i.begin() + il + 1, d) - i.begin();
            auto o = j - il;
            n->l = _from_pre_in_order(p, i, pl+1, pl+o, il, j-1);
            n->r = _from_pre_in_order(p, i, pl+o+1, pr, j+1, ir);
            return n;
        }
        void _pre_order_traversal(std::vector<T>& r, const Node<T>* n) {
            if (n == nullptr) return;
            r.push_back(n->d);
            _pre_order_traversal(r, n->l);
            _pre_order_traversal(r, n->r);
        }
        std::vector<T> pre_order_traversal() {
            std::vector<T> o;
            _pre_order_traversal(o, r);
            return o;
        }
        void _in_order_traversal(std::vector<T>& r, const Node<T>* n) {
            if (n == nullptr) return;
            _in_order_traversal(r, n->l);
            r.push_back(n->d);
            _in_order_traversal(r, n->r);
        }
        std::vector<T> in_order_traversal() {
            std::vector<T> o;
            _in_order_traversal(o, r);
            return o;
        }
        std::vector<T> simple_traversal() {
            std::vector<T> o;
            auto c = r;
            while (c != nullptr) {
                if (c->l == nullptr) {
                    o.push_back(c->d);
                    c = c->r;
                }
                else {
                    auto m = c->l;
                    while (m->r != nullptr && m->r != c)
                        m = m->r;
                    if (m->r == nullptr) {
                        m->r = c;
                        c = c->l;
                    } else {
                        m->r = nullptr;
                        o.push_back(c->d);
                        c = c->r;
                    }
                }
            }
            return o;
        }
        std::vector<T> level_order_traversal() {
            std::vector<T> o;
            std::deque<Node<T>*> m;
            m.push_back(r);
            while (!m.empty()) {
                auto s = m.size();
                for (auto i = 0; i < s; ++i) {
                    o.push_back(m[0]->d);
                    m.pop_front();
                    if (t->l != nullptr)
                        m.push_back(t->l);
                    if (t->r != nullptr)
                        m.push_back(t->r);
                }
            }
            return o;
        }
        static void test() {
            auto pv = [](const std::vector<int>& v){
                std::for_each(v.begin(), v.end(), [](int d) { std::cout << d << "\t"; });
                std::cout << std::endl;
            };
            std::vector<int> p = {3, 9, 20, 15, 7};
            std::vector<int> i = {9, 3, 15, 20, 7};
            pv(p);pv(i);

            Tree<int> t;
            t.from_pre_in_order(p, i);
            {
                auto v = t.pre_order_traversal();
                pv(v);
            }
            {
                auto v = t.in_order_traversal();
                pv(v);
            }
            {
                auto v = t.simple_traversal();
                pv(v);
            }
            {
                auto v = t.level_order_traversal();
                pv(v);
            }
        }
    };
}
