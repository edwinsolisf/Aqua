#ifndef STM_CONVERSION_H
#define STM_CONVERSION_H

#include "common.h"
#include "constant.h"

namespace stm
{
	enum class Angle_unit_type
	{
		Radian, Degree
	};

	template<Real T>
	inline constexpr T deg_to_rad(const T& degree) noexcept
	{
		return degree * static_cast<T>(stm::pi_l) / static_cast<T>(180);
	}

	template<Real T>
	inline constexpr T rad_to_deg(const T& radian) noexcept
	{
		return radian * static_cast<T>(180) / static_cast<T>(stm::pi_l);
	}

	template<Real TFrom, Real TTo, Angle_unit_type FromUnit, Angle_unit_type ToUnit>
	struct angle_conversion;

	template<Real TFrom, Real TTo>
	struct angle_conversion<TFrom, TTo, Angle_unit_type::Radian, Angle_unit_type::Degree>
	{
		static constexpr TTo convert(const TFrom& value) noexcept
		{
			return static_cast<TTo>(rad_to_deg(value));
		}
	};

	template<Real TFrom, Real TTo>
	struct angle_conversion<TFrom, TTo, Angle_unit_type::Degree, Angle_unit_type::Radian>
	{
		static constexpr TTo convert(const TFrom& value) noexcept
		{
			return static_cast<TTo>(deg_to_rad(value));
		}
	};

	template<Real TFrom, Real TTo>
	struct angle_conversion<TFrom, TTo, Angle_unit_type::Degree, Angle_unit_type::Degree>
	{
		static constexpr TTo convert(const TFrom& value) noexcept
		{
			return static_cast<TTo>(value);
		}
	};

	template<Real TFrom, Real TTo>
	struct angle_conversion<TFrom, TTo, Angle_unit_type::Radian, Angle_unit_type::Radian>
	{
		static constexpr TTo convert(const TFrom& value) noexcept
		{
			return static_cast<TTo>(value);
		}
	};
}

#endif /* STM_CONVERSION_H */