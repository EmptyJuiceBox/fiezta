#pragma once

#include <string.h>

template <typename T>
struct vec3 {
	T data[3];

	vec3() : data{T(0),T(0),T(0)} {}

	vec3(T x) : data{x,T(0),T(0)} {}

	vec3(T x, T y) : data{x,y,T(0)} {}

	vec3(T x, T y, T z) : data{x,y,z} {}

	vec3(const vec3 &vec) : vec3(vec.data) {}

	vec3(const T *vec) {
		memcpy(data, vec, sizeof(data));
	}

	const T &operator[](size_t i) const {
		return data[i];
	}

	T &operator[](size_t i) {
		return data[i];
	}

	vec3 &operator=(const vec3 &vec) {
		memcpy(data, vec.data, sizeof(data));
		return *this;
	}

	vec3 operator+(const vec3 &vec) const {
		return vec3(data[0] + vec[0], data[1] + vec[1], data[2] + vec[2]);
	}

	vec3 &operator+=(const vec3 &vec) {
		data[0] += vec[0];
		data[1] += vec[1];
		data[2] += vec[2];
		return *this;
	}

	vec3 operator-(const vec3 &vec) const {
		return vec3(data[0] - vec[0], data[1] - vec[1], data[2] - vec[2]);
	}

	vec3 &operator-=(const vec3 &vec) {
		data[0] -= vec[0];
		data[1] -= vec[1];
		data[2] -= vec[2];
		return *this;
	}

	vec3 operator*(const vec3 &vec) const {
		return vec3(data[0] * vec[0], data[1] * vec[1], data[2] * vec[2]);
	}

	vec3 &operator*=(const vec3 &vec) {
		data[0] *= vec[0];
		data[1] *= vec[1];
		data[2] *= vec[2];
		return *this;
	}

	vec3 operator*(T scalar) const {
		return vec3(data[0] * scalar, data[1] * scalar, data[2] * scalar);
	}

	friend vec3 operator*(T scalar, const vec3& vec) {
		return vec3(vec[0] * scalar, vec[1] * scalar, vec[2] * scalar);
	}

	vec3 &operator*=(T scalar) {
		data[0] *= scalar;
		data[1] *= scalar;
		data[2] *= scalar;
		return *this;
	}

	T dot(const vec3 &vec) const {
		return data[0] * vec[0] + data[1] * vec[1] + data[2] * vec[2];
	}

	vec3 cross(const vec3 &vec) const {
		return vec3(
			data[1] * vec[2] - data[2] * vec[1],
			data[2] * vec[0] - data[0] * vec[2],
			data[0] * vec[1] - data[1] * vec[0]);
	}
};
