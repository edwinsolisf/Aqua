#ifndef STM_MATH_H
#define STM_MATH_H

#include "math_internal.h"
#include "units.h"

namespace stm
{
	template<Number T>
	inline constexpr auto real(const T& value)
	{
		if constexpr (Real<T>)
			return value;
		else if constexpr (Complex<T>)
			return value.real();
	}

	template<Number T>
	inline constexpr auto imag(const T& value)
	{
		if constexpr (Real<T>)
			return static_cast<T>(0);
		else if constexpr (Complex<T>)
			return value.imag();
	}

	template<Number T>
	inline constexpr auto conjugate(const T& value)
	{
		if constexpr (Real<T>)
			return value;
		else if constexpr (Complex<T>)
			return T{ real(value) , -imag(value) };
	}

	template<Number T>
	inline constexpr T sqrt(const T& value)
	{
		if constexpr (Real<T>)
		{
			if (std::is_constant_evaluated())
				return const_sqrt(value);
			else
				return static_cast<T>(std::sqrt(value));
		}
		else if constexpr (Complex<T>)
		{
			using U = T::value_type;
			const U a = real(value), b = imag(value);
			if (std::is_constant_evaluated())
			{
				const U r = const_sqrt(a * a + b * b);
				return T{ const_sqrt((r + a) / static_cast<U>(2)), const_sqrt((r - a) / static_cast<U>(2)) };
			}
			else
			{
				const U r = std::sqrt(a * a + b * b);
				return T{ std::sqrt((r + real(value)) / static_cast<U>(2)), std::sqrt(r - real(value) / static_cast<U>(2)) };
			}
		}
	}

	template<Number T>
	inline constexpr auto norm(const T& value) noexcept
	{
		if constexpr (Real<T>)
			return value * value;
		else if constexpr (Complex<T>)
			return real(value) * real(value) + imag(value) * imag(value);
		//else
			//static_assert(false);
	}

	template<Number T>
	inline constexpr auto abs(const T& value) noexcept
	{
		if constexpr (Real<T>)
			return value >= T{0} ? value : -value;
		else
			return stm::sqrt(norm(value));
	}

	template<Real T>
	inline constexpr T tan(const T& x) noexcept
	{
		if (std::is_constant_evaluated())
			return const_tan(x);
		else
			return static_cast<T>(std::tan(x));
	}

	template<Real T, Angle_unit_type Unit>
	inline constexpr T tan(const angle_unit<T, Unit>& x) noexcept
	{
		return stm::tan(radian<T>(x).value);
	}

	template<Real T>
	inline constexpr T sin(const T& x) noexcept
	{
		if (std::is_constant_evaluated())
			return const_sin(x);
		else
			return static_cast<T>(std::sin(x));
	}

	template<Real T, Angle_unit_type Unit>
	inline constexpr T sin(const angle_unit<T, Unit>& x) noexcept
	{
		return stm::sin(radian<T>(x).value);
	}

	template<Real T>
	inline constexpr T cos(const T& x) noexcept
	{
		if (std::is_constant_evaluated())
			return const_cos(x);
		else
			return static_cast<T>(std::cos(x));
	}

	template<Real T, Angle_unit_type Unit>
	inline constexpr T cos(const angle_unit<T, Unit>& x) noexcept
	{
		return stm::cos(radian<T>(x).value);
	}

	template<Real T>
	inline constexpr T atan2(const T& y, const T& x) noexcept
	{
		if (std::is_constant_evaluated())
			return const_atan2(y, x);
		else
			return static_cast<T>(std::atan2(y, x));
	}

	template<Real T>
	inline constexpr T asin(const T& x) noexcept
	{
		if (std::is_constant_evaluated())
			return atan2(x, sqrt(T{ 1 } - (x * x)));
		else
			return static_cast<T>(std::asin(x));
	}

	template<Real T>
	inline constexpr T acos(const T& x) noexcept
	{
		if (std::is_constant_evaluated())
			return atan2(sqrt(T{ 1 } - (x * x)), x);
		else
			return static_cast<T>(std::acos(x));
	}

	template<Float T>
	inline constexpr T trunc(const T& value) noexcept
	{
		if (std::is_constant_evaluated())
			return static_cast<T>(static_cast<long long int>(value));
		else
			return std::trunc(value);
	}

	template<Float T>
	inline constexpr T fmod(const T& x, const T& y) noexcept
	{
		if (std::is_constant_evaluated())
			return x - stm::trunc(x / y) * y;
		else
			return std::fmod(x, y);
	}

}

#endif /* STM_MATH_H */