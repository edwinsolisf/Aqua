#ifndef STM_COMPLEX_H
#define STM_COMPLEX_H

#include "common.h"
#include "math.h"

namespace stm
{
	template<Real T>
	class complex
	{
	public:
		constexpr complex() noexcept = default;
		constexpr complex(const complex&) noexcept = default;
		constexpr complex(complex&&) noexcept = default;
		constexpr complex& operator=(const complex&) noexcept = default;
		constexpr complex& operator=(complex&&) noexcept = default;
		~complex() noexcept = default;

		constexpr complex(const std::complex<T>& number) noexcept
			:r(number.real()), i(number.imag())
		{}

		constexpr complex(const T& real, const T& imag = 0) noexcept
			:r{real} , i{imag}
		{}

		constexpr T& real() noexcept { return r; }
		constexpr T& imag() noexcept { return i; }
		constexpr const T& real() const noexcept { return r; }
		constexpr const T& imag() const noexcept { return i; }

		constexpr T* data() noexcept { return &r; }
		constexpr const T* data() const noexcept { return &r; }

		constexpr friend complex operator+(const complex& lhs, const T& rhs) noexcept
		{
			return { lhs.r + rhs , lhs.i };
		}

		constexpr friend complex operator-(const complex& lhs, const T& rhs) noexcept
		{
			return { lhs.r - rhs , lhs.i };
		}

		constexpr friend complex operator*(const complex& lhs, const T& rhs) noexcept
		{
			return { lhs.r * rhs , lhs.i * rhs };
		}

		constexpr friend complex operator/(const complex& lhs, const T& rhs) noexcept
		{
			return { lhs.r / rhs , lhs.i / rhs };
		}

		constexpr friend complex operator+(const complex& lhs, const complex& rhs) noexcept
		{
			return { lhs.r + rhs.r , lhs.i + rhs.i };
		}

		constexpr friend complex operator-(const complex& lhs, const complex& rhs) noexcept
		{
			return { lhs.r - rhs.r , lhs.i - rhs.i };
		}

		constexpr friend complex operator*(const complex& lhs, const complex& rhs) noexcept
		{
			return { lhs.r * rhs.r - lhs.i * rhs.i , lhs.r * rhs.i  + lhs.i * rhs.r };
		}

		constexpr friend complex operator+(const T& lhs, const complex& rhs) noexcept
		{
			return { lhs + rhs.r , rhs.i };
		}

		constexpr friend complex operator-(const T& lhs, const complex& rhs) noexcept
		{
			return { lhs - rhs.r , -rhs.i };
		}

		constexpr friend complex operator*(const T& lhs, const complex& rhs) noexcept
		{
			return { lhs * rhs.r , lhs * rhs.i };
		}

		constexpr friend complex operator/(const T& lhs, const complex& rhs) noexcept(Float<T>)
		{
			auto norm = rhs.norm();
			if constexpr (Integer<T>)
				if (norm == static_cast<T>(0)) intern::int_zero_division_except();
			return { lhs * rhs.r / norm , lhs * -rhs.i / norm };
		}

		constexpr friend complex operator/(const complex& lhs, const complex& rhs) noexcept(Float<T>)
		{
			auto norm = rhs.norm();
			if constexpr (Integer<T>)
				if (norm == static_cast<T>(0)) intern::int_zero_division_except();
			return { (lhs.r * rhs.r + lhs.i * rhs.i) / norm , (lhs.i * rhs.r - lhs.r * rhs.i) / norm };
		}

		constexpr complex& operator+=(const T& rhs) noexcept
		{
			r += rhs;
			i += rhs;
			return *this;
		}

		constexpr complex& operator-=(const T& rhs) noexcept
		{
			r -= rhs;
			i -= rhs;
			return *this;
		}

		constexpr complex& operator*=(const T& rhs) noexcept
		{
			r *= rhs;
			i *= rhs;
			return *this;
		}

		constexpr complex& operator/=(const T& rhs) noexcept(Float<T>)
		{
			if constexpr (Integer<T>)
				if (rhs == static_cast<T>(0)) intern::int_zero_division_except();
			r /= rhs;
			i /= rhs;
			return *this;
		}

		constexpr complex& operator+=(const complex& rhs) noexcept
		{
			r += rhs.r;
			i += rhs.i;
			return *this;
		}

		constexpr complex& operator-=(const complex & rhs) noexcept
		{
			r -= rhs.r;
			i -= rhs.i;
			return *this;
		}

		constexpr complex& operator*=(const complex & rhs) noexcept
		{
			auto tmp = r * rhs.r - i * rhs.i;
			i = r * rhs.i + i * rhs.r;
			r = tmp;
			return *this;
		}

		constexpr complex& operator/=(const complex & rhs) noexcept(Float<T>)
		{
			T norm = rhs.norm();
			if constexpr (Integer<T>)
				if (norm == static_cast<T>(0)) intern::int_zero_division_except();

			auto tmp = r * rhs.r + i * rhs.i;
			i = (-r * rhs.i + i * rhs.r) / norm;
			r = tmp / norm;
			return *this;
		}

		constexpr T norm() const noexcept { return r * r + i * i; }

		constexpr T abs() const noexcept { return stm::sqrt(this->norm()); }

		constexpr complex conj() const noexcept { return { r , -i }; }

		constexpr complex unit() const noexcept(Float<T>)
		{
			auto abs = this->abs();
			if constexpr (Integer<T>)
				if (abs == static_cast<T>(0)) intern::int_zero_division_except();
			return *this / this->abs();
		}

		constexpr T arg() const noexcept { return stm::atan2(i, r); }

		template<Number U>
		constexpr complex<U> cast() const noexcept
		{
			using Uu = underlying_num_t<U>;
			return { static_cast<Uu>(r) , static_cast<Uu>(i) };
		}

		using value_type = T;

		friend std::ostream& operator<<<T>(std::ostream& stream, const stm::complex<T>& value);

	private:
		T r, i;
	};
}

#endif /* STM_COMPLEX_H */