#pragma once

#include <random>
#include <type_traits>

namespace xl::util {

	template <typename T>
	struct random {
		using type = typename std::conditional<
			std::is_integral<T>::value,
			std::uniform_int_distribution<>,
			std::uniform_real_distribution<>
		>::type;

		random(T begin, T end) {
			std::random_device rd;
			m_gen = std::mt19937(rd());
			m_dis = type(begin, end);
		}

		T operator()() {
			return m_dis(m_gen);
		}

		type m_dis;
		std::mt19937 m_gen;
	};

}
