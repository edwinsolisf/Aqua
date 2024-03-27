#ifndef STM_QUATERNION_H
#define STM_QUATERNION_H

#include "common.h"

namespace stm
{
	template<Real T>
	class quaternion
	{
	public:
		using value_type = T;
		using underlying_num_type = underlying_num_t<T>;

		constexpr quaternion() noexcept = default;
		constexpr quaternion(const quaternion&) noexcept = default;
		constexpr quaternion(quaternion&&) noexcept = default;
		~quaternion() noexcept = default;

		constexpr quaternion(std::span<T, 4> data) noexcept
			: r{data[0]}, i{data[1]}, j{data[2]}, k{data[3]} {}
		constexpr quaternion(T r, T i, T j, T k) noexcept
			: r{r}, i{i}, j{j}, k{k} {}

		friend constexpr quaternion operator*(const quaternion& lhs, const quaternion& rhs) noexcept
		{
			return quaternion{
				lhs.r * rhs.r - lhs.i * rhs.i - lhs.j * rhs.j - lhs.k * rhs.k,
				lhs.r * rhs.i + lhs.i * rhs.r + lhs.j * rhs.k - lhs.k * rhs.j,
				lhs.r * rhs.j - lhs.i * rhs.k + lhs.j * rhs.r + lhs.k * rhs.i,
				lhs.r * rhs.k + lhs.i * rhs.j - lhs.j * rhs.i + lhs.k * rhs.r
			};
		}

		constexpr T real() const noexcept { return r; }
		constexpr std::span<T, 3> imaginary() const noexcept { return { &data_[1] }; }

		constexpr quaternion conjugate() const noexcept
		{
			return { r, -i , -j , -k };
		}

		constexpr T magnitude() const noexcept
		{
			return std::sqrt(r * r + i * i + j * j + k * k);
		}

		constexpr quaternion inverse() const noexcept
		{
			return conjugate() / magnitude();
		}

		T& r = data_[0];
		T& i = data_[1];
		T& j = data_[2];
		T& k = data_[3];

		constexpr T& operator[](std::size_t i) noexcept { return data_[i]; }
		constexpr const T& operator[](std::size_t i) const noexcept { return data_[i]; }

	private:
		std::array<T, 4> data_;
	};

	using quatf = quaternion<float>;
}

#endif /* STM_QUATERNION_H */