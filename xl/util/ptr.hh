#pragma once
#include <utility>
#include "xl/util/mem.hh"

namespace xl::util {
	template <typename T>
	struct ptr {
		T* m_data;
		std::size_t* m_size;
		ptr() : m_size(new std::size_t) {
			*m_size = 0;
		}
		~ptr() {
			*m_size -= 1;
			if (*m_size == 0) {
                m_data->~T();
				delete m_size;
				delete m_data;
			}
		}
		ptr(const ptr& a) {
			this->m_data = a.m_data;
			this->m_size = a.m_size;
			*this->m_size += 1;
		}
		ptr& operator=(ptr a) {
			ptr tmp;
			tmp.m_data = a.m_data;
			tmp.m_size = a.m_size;
			*tmp.m_size += 1;
			return tmp;
		}
		T& operator*() {
			return *m_data;
		}
		T* operator->() {
			return m_data;
		}
	};
	template <typename T, typename... Args>
	ptr<T> make_ptr(Args&&... args) {
        void* raw = nils::util::mem_pool::instance().alloc(sizeof(T));
		ptr<T> tmp;
        tmp.m_data = new(raw)T(std::forward<Args>(args)...);
		*tmp.m_size += 1;
		return tmp;
	}
}
