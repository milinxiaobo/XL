#pragma once
#include <cstdlib>
#include <atomic>
#include <unordered_map>

namespace xl::util {
    struct mem_address {
        std::size_t address;
        void* load() {
            return reinterpret_cast<void*>(address);
        }
        void save(void* ptr) {
            address = reinterpret_cast<std::size_t>(ptr);
        }
        template <typename T>
        void save(T* ptr) {
            address = reinterpret_cast<std::size_t>(ptr);
        }
        template <typename T>
        T* load() {
            return reinterpret_cast<T*>(address);
        }
        operator std::size_t() {
            return address;
        }
        mem_address() = default;
        mem_address(void* ptr) {
            address = reinterpret_cast<std::size_t>(ptr);
        }
        mem_address(std::size_t addr) {
            address = addr;
        }
    };
    struct mem_obj {
        mem_address offset;
        std::size_t size;
        mem_obj(mem_address offset, std::size_t size)
            : offset(offset), size(size) {
        }
    };
    struct mem_pool {
        static mem_pool& instance() {
            static mem_pool temp;
            return temp;
        }
        std::atomic<std::size_t> m_counter;
        std::size_t m_size;
        std::size_t m_alignment;
        void* m_data;
        std::vector<std::size_t> m_coll;
        std::vector<mem_obj> m_free_mem_obj_vec;
        std::vector<mem_obj> m_used_mem_obj_vec;
        std::unordered_map<std::size_t, mem_obj> m_map_address;
        mem_pool() {
            m_size = 1024;
            m_alignment = 64;
            m_data = (void*)std::malloc(m_size);
            m_coll = std::vector<std::size_t>(m_size, 0);
        }
        void* alloc(std::size_t size) {
            m_counter++;
            std::cout << "m_counter:" << m_counter << "\n";
            auto temp = std::aligned_alloc(m_alignment, size);
            m_map_address.emplace(mem_address(temp), mem_obj(mem_address(temp), size));
            return temp;
            // return malloc(size);
        }
        void move(void* ptr) {

        }
        std::size_t alloc_index(std::size_t size) {
            auto index = ++m_counter;
            auto address = std::aligned_alloc(m_alignment, size);
            m_map_address.emplace(mem_address(index), mem_obj(mem_address(address), size));
            return index;
        }
        void* get_address(std::size_t index) {
            return m_map_address.at(index).offset.load();
        }
    };
    template<typename T>
    struct object {
        T& operator*() {
            // return *reinterpret_cast<T*>(mem_pool::instance().get());
        }
    };
}
