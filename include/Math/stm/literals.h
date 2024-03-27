#ifndef STM_LITERALS_H
#define STM_LITERALS_H

#include "common.h"

namespace stm
{
	namespace literals
	{
		inline constexpr radian<long double> operator"" _rad(long double angle) noexcept
		{
			return radian<long double>(angle);
		}

		inline constexpr degree<long double> operator"" _deg(long double angle) noexcept
		{
			return degree<long double>(angle);
		}

		inline constexpr radian<long long> operator"" _rad(unsigned long long angle) noexcept
		{
			return radian<long long>(angle);
		}

		inline constexpr degree<long long> operator"" _deg(unsigned long long angle) noexcept
		{
			return degree<long long>(angle);
		}
	}
}

#endif /* STM_LITERALS_H */