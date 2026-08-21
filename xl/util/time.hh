#pragma once

#include <chrono>

namespace xl::util {

	template <typename T>
	struct time_unit {
		constexpr static char unit[] = "not support";
	};

	template <>
	struct time_unit<std::chrono::milliseconds> {
		constexpr static char unit[] = "ms";
	};

	template <>
	struct time_unit<std::chrono::microseconds> {
		constexpr static char unit[] = "us";
	};

	template <>
	struct time_unit<std::chrono::nanoseconds> {
		constexpr static char unit[] = "ns";
	};

	template <typename T>
	struct time {
		using btype = std::chrono::steady_clock;
		using ttype = std::chrono::time_point<btype>;

		time() {
			start();
		}

		~time() {
			stop();
		}

		void start() {
			m_last = btype::now();
		}

		void stop() {
			std::cout
				<< "\t"
				<< std::chrono::duration_cast<T>(btype::now() - m_last).count()
				<< time_unit<T>::unit
				<< "\n";
		}

		ttype m_last;
	};

	using time_ms = time<std::chrono::milliseconds>;
	using time_us = time<std::chrono::microseconds>;
	using time_ns = time<std::chrono::nanoseconds>;
}
