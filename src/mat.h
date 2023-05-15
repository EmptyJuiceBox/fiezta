#pragma once

#include <string.h>
#include <iostream>

template <typename T>
struct mat4 {
	T data[16];

	mat4() : data{
		T(1), T(0), T(0), T(0),
		T(0), T(1), T(0), T(0),
		T(0), T(0), T(1), T(0),
		T(0), T(0), T(0), T(1)} {}

	mat4(
		T m00, T m01, T m02, T m03,
		T m10, T m11, T m12, T m13,
		T m20, T m21, T m22, T m23,
		T m30, T m31, T m32, T m33
	) : data{
		m00, m01, m02, m03,
		m10, m11, m12, m13,
		m20, m21, m22, m23,
		m30, m31, m32, m33} {}

	mat4(const mat4 &mat) : mat4(mat.data) {}

	mat4(const T *mat) {
		memcpy(data, mat, sizeof(data));
	}

	const T *operator[](size_t i) const {
		return &data[i * 4];
	}

	T *operator[](size_t i) {
		return &data[i * 4];
	}

	mat4 &operator=(const mat4 &mat) {
		memcpy(data, mat.data, sizeof(data));
		return *this;
	}
};
