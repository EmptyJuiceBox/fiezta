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

	mat4 operator*(T scalar) const {
		return mat4(
			data[0][0] * scalar, data[0][1] * scalar, data[0][2] * scalar, data[0][3] * scalar,
			data[1][0] * scalar, data[1][1] * scalar, data[1][2] * scalar, data[1][3] * scalar,
			data[2][0] * scalar, data[2][1] * scalar, data[2][2] * scalar, data[2][3] * scalar,
			data[3][0] * scalar, data[3][1] * scalar, data[3][2] * scalar, data[3][3] * scalar);
	}

	friend mat4 operator*(T scalar, const mat4 &mat) {
		return mat * scalar;
	}

	mat4 &operator*=(T scalar) {
		*this = *this * scalar;
		return *this;
	}

	mat4 operator*(const mat4 &mat) const {
		return mat4(
			data[0][0] * mat[0][0] + data[0][1] * mat[1][0] + data[0][2] * mat[2][0] + data[0][3] * mat[3][0],
			data[0][0] * mat[0][1] + data[0][1] * mat[1][1] + data[0][2] * mat[2][1] + data[0][3] * mat[3][1],
			data[0][0] * mat[0][2] + data[0][1] * mat[1][2] + data[0][2] * mat[2][2] + data[0][3] * mat[3][2],
			data[0][0] * mat[0][3] + data[0][1] * mat[1][3] + data[0][2] * mat[2][3] + data[0][3] * mat[3][3],

			data[1][0] * mat[0][0] + data[1][1] * mat[1][0] + data[1][2] * mat[2][0] + data[1][3] * mat[3][0],
			data[1][0] * mat[0][1] + data[1][1] * mat[1][1] + data[1][2] * mat[2][1] + data[1][3] * mat[3][1],
			data[1][0] * mat[0][2] + data[1][1] * mat[1][2] + data[1][2] * mat[2][2] + data[1][3] * mat[3][2],
			data[1][0] * mat[0][3] + data[1][1] * mat[1][3] + data[1][2] * mat[2][3] + data[1][3] * mat[3][3],

			data[2][0] * mat[0][0] + data[2][1] * mat[1][0] + data[2][2] * mat[2][0] + data[2][3] * mat[3][0],
			data[2][0] * mat[0][1] + data[2][1] * mat[1][1] + data[2][2] * mat[2][1] + data[2][3] * mat[3][1],
			data[2][0] * mat[0][2] + data[2][1] * mat[1][2] + data[2][2] * mat[2][2] + data[2][3] * mat[3][2],
			data[2][0] * mat[0][3] + data[2][1] * mat[1][3] + data[2][2] * mat[2][3] + data[2][3] * mat[3][3],

			data[3][0] * mat[0][0] + data[3][1] * mat[1][0] + data[3][2] * mat[2][0] + data[3][3] * mat[3][0],
			data[3][0] * mat[0][1] + data[3][1] * mat[1][1] + data[3][2] * mat[2][1] + data[3][3] * mat[3][1],
			data[3][0] * mat[0][2] + data[3][1] * mat[1][2] + data[3][2] * mat[2][2] + data[3][3] * mat[3][2],
			data[3][0] * mat[0][3] + data[3][1] * mat[1][3] + data[3][2] * mat[2][3] + data[3][3] * mat[3][3]);
	}

	mat4 &operator*=(const mat4 &mat) {
		*this = *this * mat;
		return *this;
	}

	mat4 transpose() const {
		return mat4(
			data[0][0], data[1][0], data[2][0], data[3][0],
			data[0][1], data[1][1], data[2][1], data[3][1],
			data[0][2], data[1][2], data[2][2], data[3][3],
			data[0][3], data[1][3], data[2][3], data[3][3]);
	}

	mat4 inverse() const {
		T A2323 = data[2][2] * data[3][3] - data[2][3] * data[3][2];
		T A1323 = data[2][1] * data[3][3] - data[2][3] * data[3][1];
		T A1223 = data[2][1] * data[3][2] - data[2][2] * data[3][1];
		T A0323 = data[2][0] * data[3][3] - data[2][3] * data[3][0];
		T A0223 = data[2][0] * data[3][2] - data[2][2] * data[3][0];
		T A0123 = data[2][0] * data[3][1] - data[2][1] * data[3][0];
		T A2313 = data[1][2] * data[3][3] - data[1][3] * data[3][2];
		T A1313 = data[1][1] * data[3][3] - data[1][3] * data[3][1];
		T A1213 = data[1][1] * data[3][2] - data[1][2] * data[3][1];
		T A2312 = data[1][2] * data[2][3] - data[1][3] * data[2][2];
		T A1312 = data[1][1] * data[2][3] - data[1][3] * data[2][1];
		T A1212 = data[1][1] * data[2][2] - data[1][2] * data[2][1];
		T A0313 = data[1][0] * data[3][3] - data[1][3] * data[3][0];
		T A0213 = data[1][0] * data[3][2] - data[1][2] * data[3][0];
		T A0312 = data[1][0] * data[2][3] - data[1][3] * data[2][0];
		T A0212 = data[1][0] * data[2][2] - data[1][2] * data[2][0];
		T A0113 = data[1][0] * data[3][1] - data[1][1] * data[3][0];
		T A0112 = data[1][0] * data[2][1] - data[1][1] * data[2][0];

		T det =
			data[0][0] * (data[1][1] * A2323 - data[1][2] * A1323 + data[1][3] * A1223) -
			data[0][1] * (data[1][0] * A2323 - data[1][2] * A0323 + data[1][3] * A0223) +
			data[0][2] * (data[1][0] * A1323 - data[1][1] * A0323 + data[1][3] * A0123) -
			data[0][3] * (data[1][0] * A1223 - data[1][1] * A0223 + data[1][2] * A0123);

		det = T(1) / det;

		return mat4(
			det *   (data[1][1] * A2323 - data[1][2] * A1323 + data[1][3] * A1223),
			det * - (data[0][1] * A2323 - data[0][2] * A1323 + data[0][3] * A1223),
			det *   (data[0][1] * A2313 - data[0][2] * A1313 + data[0][3] * A1213),
			det * - (data[0][1] * A2312 - data[0][2] * A1312 + data[0][3] * A1212),
			det * - (data[1][0] * A2323 - data[1][2] * A0323 + data[1][3] * A0223),
			det *   (data[0][0] * A2323 - data[0][2] * A0323 + data[0][3] * A0223),
			det * - (data[0][0] * A2313 - data[0][2] * A0313 + data[0][3] * A0213),
			det *   (data[0][0] * A2312 - data[0][2] * A0312 + data[0][3] * A0212),
			det *   (data[1][0] * A1323 - data[1][1] * A0323 + data[1][3] * A0123),
			det * - (data[0][0] * A1323 - data[0][1] * A0323 + data[0][3] * A0123),
			det *   (data[0][0] * A1313 - data[0][1] * A0313 + data[0][3] * A0113),
			det * - (data[0][0] * A1312 - data[0][1] * A0312 + data[0][3] * A0112),
			det * - (data[1][0] * A1223 - data[1][1] * A0223 + data[1][2] * A0123),
			det *   (data[0][0] * A1223 - data[0][1] * A0223 + data[0][2] * A0123),
			det * - (data[0][0] * A1213 - data[0][1] * A0213 + data[0][2] * A0113),
			det *   (data[0][0] * A1212 - data[0][1] * A0212 + data[0][2] * A0112));
	}
};
