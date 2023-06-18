#pragma once

#include <string.h>
#include "vec.h"

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

	vec3<T> operator*(const vec3<T> &vec) const {
		const mat4 &m = *this;

		return vec3<T>(
			vec[0] * m[0][0] + vec[1] * m[0][1] + vec[2] * m[0][2] + m[0][3],
			vec[0] * m[1][0] + vec[1] * m[1][1] + vec[2] * m[1][2] + m[1][3],
			vec[0] * m[2][0] + vec[1] * m[2][1] + vec[2] * m[2][2] + m[2][3]);
	}

	mat4 &operator=(const mat4 &mat) {
		memcpy(data, mat.data, sizeof(data));
		return *this;
	}

	mat4 operator*(T scalar) const {
		const mat4 &m = *this;

		return mat4(
			m[0][0] * scalar, m[0][1] * scalar, m[0][2] * scalar, m[0][3] * scalar,
			m[1][0] * scalar, m[1][1] * scalar, m[1][2] * scalar, m[1][3] * scalar,
			m[2][0] * scalar, m[2][1] * scalar, m[2][2] * scalar, m[2][3] * scalar,
			m[3][0] * scalar, m[3][1] * scalar, m[3][2] * scalar, m[3][3] * scalar);
	}

	friend mat4 operator*(T scalar, const mat4 &mat) {
		return mat * scalar;
	}

	mat4 &operator*=(T scalar) {
		*this = *this * scalar;
		return *this;
	}

	mat4 operator*(const mat4 &mat) const {
		const mat4 &m = *this;

		return mat4(
			m[0][0] * mat[0][0] + m[0][1] * mat[1][0] + m[0][2] * mat[2][0] + m[0][3] * mat[3][0],
			m[0][0] * mat[0][1] + m[0][1] * mat[1][1] + m[0][2] * mat[2][1] + m[0][3] * mat[3][1],
			m[0][0] * mat[0][2] + m[0][1] * mat[1][2] + m[0][2] * mat[2][2] + m[0][3] * mat[3][2],
			m[0][0] * mat[0][3] + m[0][1] * mat[1][3] + m[0][2] * mat[2][3] + m[0][3] * mat[3][3],

			m[1][0] * mat[0][0] + m[1][1] * mat[1][0] + m[1][2] * mat[2][0] + m[1][3] * mat[3][0],
			m[1][0] * mat[0][1] + m[1][1] * mat[1][1] + m[1][2] * mat[2][1] + m[1][3] * mat[3][1],
			m[1][0] * mat[0][2] + m[1][1] * mat[1][2] + m[1][2] * mat[2][2] + m[1][3] * mat[3][2],
			m[1][0] * mat[0][3] + m[1][1] * mat[1][3] + m[1][2] * mat[2][3] + m[1][3] * mat[3][3],

			m[2][0] * mat[0][0] + m[2][1] * mat[1][0] + m[2][2] * mat[2][0] + m[2][3] * mat[3][0],
			m[2][0] * mat[0][1] + m[2][1] * mat[1][1] + m[2][2] * mat[2][1] + m[2][3] * mat[3][1],
			m[2][0] * mat[0][2] + m[2][1] * mat[1][2] + m[2][2] * mat[2][2] + m[2][3] * mat[3][2],
			m[2][0] * mat[0][3] + m[2][1] * mat[1][3] + m[2][2] * mat[2][3] + m[2][3] * mat[3][3],

			m[3][0] * mat[0][0] + m[3][1] * mat[1][0] + m[3][2] * mat[2][0] + m[3][3] * mat[3][0],
			m[3][0] * mat[0][1] + m[3][1] * mat[1][1] + m[3][2] * mat[2][1] + m[3][3] * mat[3][1],
			m[3][0] * mat[0][2] + m[3][1] * mat[1][2] + m[3][2] * mat[2][2] + m[3][3] * mat[3][2],
			m[3][0] * mat[0][3] + m[3][1] * mat[1][3] + m[3][2] * mat[2][3] + m[3][3] * mat[3][3]);
	}

	mat4 &operator*=(const mat4 &mat) {
		*this = *this * mat;
		return *this;
	}

	mat4 transpose() const {
		const mat4 &m = *this;

		return mat4(
			m[0][0], m[1][0], m[2][0], m[3][0],
			m[0][1], m[1][1], m[2][1], m[3][1],
			m[0][2], m[1][2], m[2][2], m[3][2],
			m[0][3], m[1][3], m[2][3], m[3][3]);
	}

	mat4 inverse() const {
		const mat4 &m = *this;

		T A2323 = m[2][2] * m[3][3] - m[2][3] * m[3][2];
		T A1323 = m[2][1] * m[3][3] - m[2][3] * m[3][1];
		T A1223 = m[2][1] * m[3][2] - m[2][2] * m[3][1];
		T A0323 = m[2][0] * m[3][3] - m[2][3] * m[3][0];
		T A0223 = m[2][0] * m[3][2] - m[2][2] * m[3][0];
		T A0123 = m[2][0] * m[3][1] - m[2][1] * m[3][0];
		T A2313 = m[1][2] * m[3][3] - m[1][3] * m[3][2];
		T A1313 = m[1][1] * m[3][3] - m[1][3] * m[3][1];
		T A1213 = m[1][1] * m[3][2] - m[1][2] * m[3][1];
		T A2312 = m[1][2] * m[2][3] - m[1][3] * m[2][2];
		T A1312 = m[1][1] * m[2][3] - m[1][3] * m[2][1];
		T A1212 = m[1][1] * m[2][2] - m[1][2] * m[2][1];
		T A0313 = m[1][0] * m[3][3] - m[1][3] * m[3][0];
		T A0213 = m[1][0] * m[3][2] - m[1][2] * m[3][0];
		T A0312 = m[1][0] * m[2][3] - m[1][3] * m[2][0];
		T A0212 = m[1][0] * m[2][2] - m[1][2] * m[2][0];
		T A0113 = m[1][0] * m[3][1] - m[1][1] * m[3][0];
		T A0112 = m[1][0] * m[2][1] - m[1][1] * m[2][0];

		T det =
			m[0][0] * (m[1][1] * A2323 - m[1][2] * A1323 + m[1][3] * A1223) -
			m[0][1] * (m[1][0] * A2323 - m[1][2] * A0323 + m[1][3] * A0223) +
			m[0][2] * (m[1][0] * A1323 - m[1][1] * A0323 + m[1][3] * A0123) -
			m[0][3] * (m[1][0] * A1223 - m[1][1] * A0223 + m[1][2] * A0123);

		det = T(1) / det;

		return mat4(
			det *  (m[1][1] * A2323 - m[1][2] * A1323 + m[1][3] * A1223),
			det * -(m[0][1] * A2323 - m[0][2] * A1323 + m[0][3] * A1223),
			det *  (m[0][1] * A2313 - m[0][2] * A1313 + m[0][3] * A1213),
			det * -(m[0][1] * A2312 - m[0][2] * A1312 + m[0][3] * A1212),
			det * -(m[1][0] * A2323 - m[1][2] * A0323 + m[1][3] * A0223),
			det *  (m[0][0] * A2323 - m[0][2] * A0323 + m[0][3] * A0223),
			det * -(m[0][0] * A2313 - m[0][2] * A0313 + m[0][3] * A0213),
			det *  (m[0][0] * A2312 - m[0][2] * A0312 + m[0][3] * A0212),
			det *  (m[1][0] * A1323 - m[1][1] * A0323 + m[1][3] * A0123),
			det * -(m[0][0] * A1323 - m[0][1] * A0323 + m[0][3] * A0123),
			det *  (m[0][0] * A1313 - m[0][1] * A0313 + m[0][3] * A0113),
			det * -(m[0][0] * A1312 - m[0][1] * A0312 + m[0][3] * A0112),
			det * -(m[1][0] * A1223 - m[1][1] * A0223 + m[1][2] * A0123),
			det *  (m[0][0] * A1223 - m[0][1] * A0223 + m[0][2] * A0123),
			det * -(m[0][0] * A1213 - m[0][1] * A0213 + m[0][2] * A0113),
			det *  (m[0][0] * A1212 - m[0][1] * A0212 + m[0][2] * A0112));
	}
};
