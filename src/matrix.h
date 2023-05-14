#pragma once

#include <string.h>

template <typename T>
struct matrix {
	T data[16];

	matrix() : data{
		T(1), T(0), T(0), T(0),
		T(0), T(1), T(0), T(0),
		T(0), T(0), T(1), T(0),
		T(0), T(0), T(0), T(1)} {}

	matrix(const matrix<T> &mat) : matrix(mat.data) {}

	matrix(const T *mat) {
		memcpy(data, mat, sizeof(data));
	}

	const T *operator[](size_t i) const {
		return &data[i * 4];
	}

	T *operator[](size_t i) {
		return &data[i * 4];
	}
};
