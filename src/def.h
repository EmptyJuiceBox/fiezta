#pragma once

#include <groufix.h>

#define dassert(expr) do { \
	if (!(expr)) { \
		std::cerr << "Asertion failed: " #expr << '\n'; \
		std::cerr << "  At " << __FILE__ << ":" << __LINE__ << "(" << __func__ << ")\n"; \
		abort(); \
	} \
} while (0)

template<typename T>
const T *ref(const T &val) {
	return &val;
}
