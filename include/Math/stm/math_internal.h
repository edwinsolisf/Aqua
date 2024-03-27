#ifndef STM_MATH_INTERNAL_H
#define STM_MATH_INTERNAL_H

#include "common.h"
#include "constant.h"

namespace stm
{
	namespace intern
	{
		static constexpr long double sqrt_newtonraphson(long double x, long double curr, long double prev) noexcept
		{
			return curr == prev? curr : sqrt_newtonraphson(x, 0.5 * (curr + x / curr), curr);
		}

		static constexpr long double const_sqrt_impl(long double value) noexcept
		{
			return value >= 0 && value < std::numeric_limits<long double>::infinity()
				? sqrt_newtonraphson(value, value, 0)
				: std::numeric_limits<long double>::quiet_NaN();
		}

		static constexpr long double atan_iter_fraction(long double x, long double curr, long double prev, long long iter,
														long double m_0, long double m_1, long double n_0, long double n_1) noexcept
		{
			const long double a_1 = 2.L * iter + 1.L;
			const long double b_0 = static_cast<long double>(iter * iter);
			const long double top = a_1 * m_1 + b_0 * x * x * m_0;
			const long double bottom = a_1 * n_1 + b_0 * x * x * n_0;
			return curr == prev ? curr : atan_iter_fraction(x, top / bottom, curr, iter + 1, m_1, top, n_1, bottom);
		}

		static constexpr long double const_atan_impl(long double val) noexcept
		{
			return atan_iter_fraction(val, val, 0.L, 2, val, val * 3.L, 1.L, 3.L + val * val);
		}

		static constexpr long double const_atan2_impl(long double y, long double x) noexcept
		{
			if (x == 0.L)
			{
				if (y > 0.L)
					return pi / 2.0L;
				else if (y < 0.L)
					return -pi / 2.0L;
				else
					return (1.L / y < 0) ? ((1.L / x < 0) ? -pi : -0.L) : ((1.L / x < 0) ? pi : 0.L);
			}
			else if (x < 0.L)
			{
				if (y >= 0.L)
					return const_atan_impl(y / x) + pi;
				else
					return const_atan_impl(y / x) - pi;
			}
			else
				return const_atan_impl(y / x);
		}

		static constexpr long double tan_iter_fraction(long double x, long double curr, long double prev, long long iter,
														long double m_0, long double m_1, long double n_0, long double n_1) noexcept
		{
			const long double a_1 = 2.L * iter + 1.L;
			const long double top = a_1 * m_1 - x * x * m_0;
			const long double bottom = a_1 * n_1 - x * x * n_0;
			return curr == prev ? curr : tan_iter_fraction(x, top / bottom, curr, iter + 1, m_1, top, n_1, bottom);
		}

		static constexpr long double const_tan_impl(long double x) noexcept
		{
			return tan_iter_fraction(x, x, 0.L, 2, x, x * 3.L, 1.L, 3.L - x * x);
		}
	}


	template<Real T>
	inline constexpr T const_sqrt(const T& value) noexcept
	{
		return static_cast<T>(intern::const_sqrt_impl(static_cast<long double>(value)));
	}

	template<Real T>
	inline constexpr T const_atan2(const T& y, const T& x) noexcept
	{
		return static_cast<T>(intern::const_atan2_impl(static_cast<long double>(y), static_cast<long double>(x)));
	}

	template<Real T>
	inline constexpr T const_tan(const T& x) noexcept
	{
		return static_cast<T>(intern::const_tan_impl(static_cast<long double>(x)));
	}

	template<Real T>
	inline constexpr T const_sin(const T& x) noexcept
	{
		const long double t = intern::const_tan_impl(static_cast<long double>(x) / 2.L);
		return static_cast<T>((t * 2.L) / (1.L + t * t));
	}

	template<Real T>
	inline constexpr T const_cos(const T& x) noexcept
	{
		const long double t = intern::const_tan_impl(static_cast<long double>(x) / 2.L);
		return static_cast<T>((1.L - t * t) / (1.L + t * t));
	}
}

#endif /* STM_MATH_INTERNAL_H */