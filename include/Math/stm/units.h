#ifndef STM_UNITS_H
#define STM_UNITS_H

#include "common.h"
#include "conversion.h"

namespace stm
{
	template<Real T, Angle_unit_type Unit>
	struct angle_unit
	{
		using value_type = T;
		static constexpr Angle_unit_type unit = Unit;

		constexpr angle_unit() noexcept = default;
		constexpr angle_unit(const angle_unit&) noexcept = default;
		constexpr angle_unit(angle_unit&&) noexcept = default;
		constexpr angle_unit& operator=(const angle_unit&) noexcept = default;
		constexpr angle_unit& operator=(angle_unit&&) noexcept = default;
		~angle_unit() noexcept = default;

		explicit constexpr angle_unit(const T& angle) noexcept
			:value{ angle } {};

		template<Real U, Angle_unit_type OUnit>
		constexpr angle_unit(const angle_unit<U, OUnit>& angle) noexcept
			:value{ angle_conversion<U, T, OUnit, Unit>::convert(angle.value) }
		{}

		template<Real U, Angle_unit_type OUnit>
		constexpr angle_unit& operator=(const angle_unit<U, OUnit>& angle) noexcept
		{
			value = angle_conversion<U, T, OUnit, Unit>::convert(angle.value);
			return *this;
		}

		constexpr angle_unit operator+() const noexcept { return *this; }
		constexpr angle_unit operator-() const noexcept { return angle_unit{ -value }; }
		constexpr operator T() const noexcept { return value; }

		T value;
	};

	template<Real T>
	using radian = angle_unit<T, Angle_unit_type::Radian>;

	template<Real T>
	using degree = angle_unit<T, Angle_unit_type::Degree>;
}

#endif /* STM_UNITS_H */