#ifndef STM_NUMERIC_H
#define STM_NUMERIC_H

#include "common.h"

namespace stm
{
	template<Integer T>
	inline constexpr T gcd(T a, T b) noexcept
	{
		for (T temp = a; b != 0; temp = a)
		{
			a = b;
			b = temp % b;
		}
		return a;
	}

	template<Integer T>
	inline constexpr T lcm(T a, T b) noexcept
	{
		return a > b ? b * (a / gcd(a, b)) : a * (b / gcd(a, b));
	}
}

#endif /* STM_NUMERIC_H */