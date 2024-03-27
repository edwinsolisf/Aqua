#ifndef STM_POLAR_COMPLEX_H
#define STM_POLAR_COMPLEX_H

#include "common.h"
#include "complex.h"

namespace stm
{
	template<Float T>
	class polar_complex
	{
	public:
		using value_type = T;

		constexpr polar_complex() noexcept = default;
		constexpr polar_complex(const polar_complex&) noexcept = default;
		constexpr polar_complex(polar_complex&&) noexcept = default;
		constexpr polar_complex& operator=(const polar_complex&) noexcept = default;
		constexpr polar_complex& operator=(polar_complex&&) noexcept = default;
		~polar_complex() noexcept = default;

		template<Float U, Angle_unit_type Unit>
		constexpr polar_complex(const T& abs, const angle_unit<U, Unit>& arg) noexcept
			:abs_{ abs }, arg_{ stm::fmod<T>(radian<T>(arg), static_cast<T>(stm::pi_l)) } {}

		constexpr polar_complex(const stm::complex<T>&) noexcept;
		constexpr polar_complex(const std::complex<T>&) noexcept;

		constexpr polar_complex& operator=(const stm::complex<T>&) noexcept;
		constexpr polar_complex& operator=(const std::complex<T>&) noexcept;

		constexpr T real() const noexcept;
		constexpr T imag() const noexcept;
		constexpr T& abs() noexcept { return abs_; }
		constexpr T& arg() noexcept { return arg_; }
		constexpr T norm() const noexcept { return abs_ * abs_; }
		constexpr const T& abs() const noexcept { return abs_; }
		constexpr const T& arg() const noexcept { return arg_; }
		constexpr polar_complex conj() const noexcept { return { abs_ , radian<T>(-arg_) }; }
		constexpr polar_complex unit() const noexcept { return { T{1} , radian<T>(arg_) }; }

		constexpr polar_complex operator+() const noexcept { return *this; }
		constexpr polar_complex operator-() const noexcept
		{ 
			return { abs_, radian<T>(arg_ >= 0 ? arg_ - static_cast<T>(stm::pi_l) : arg_ + static_cast<T>(stm::pi_l)) }; 
		}

		constexpr friend polar_complex operator+(const polar_complex& lhs, const polar_complex& rhs) noexcept
		{
			const T rcos = rhs.abs_ * stm::cos(rhs.arg_ - lhs.arg_);
			return { stm::sqrt(lhs.norm() + rhs.norm() + T{2} * lhs.abs_ * rcos),
					 radian<T>(normalize_arg(lhs.arg_ + stm::atan2(rhs.abs_ * stm::sin(rhs.arg_ - lhs.arg_) , lhs.abs_ + rcos))) };
		}

		constexpr friend polar_complex operator-(const polar_complex& lhs, const polar_complex& rhs) noexcept
		{
			return lhs + (-rhs);
		}

		constexpr friend polar_complex operator*(const polar_complex& lhs, const polar_complex& rhs) noexcept
		{
			return { lhs.abs_ * rhs.abs_ , radian<T>(normalize_arg(lhs.arg_ + rhs.arg_)) };
		}

		constexpr friend polar_complex operator/(const polar_complex& lhs, const polar_complex& rhs) noexcept
		{
			return { lhs.abs_ / rhs.abs_ , radian<T>(normalize_arg(lhs.arg_ + rhs.arg_)) };
		}

		constexpr polar_complex& operator+=(const polar_complex& rhs) noexcept
		{
			const T rcos = rhs.abs_ * stm::cos(rhs.arg_ - arg_);
			const T arg = normalize_arg(arg_ + stm::atan2(rhs.abs_ * stm::sin(rhs.arg_ - arg_), abs_ + rcos));
			abs_ = stm::sqrt(norm() + rhs.norm() + T{ 2 } * abs_ * rcos);
			arg_ = arg;
			return *this;
		}

		constexpr polar_complex& operator-=(const polar_complex& rhs) noexcept
		{
			return *this += (-rhs);
		}

		constexpr polar_complex& operator*=(const polar_complex& rhs) noexcept
		{
			abs_ *= rhs.abs;
			arg_ = normalize_arg(arg_ + rhs.arg_);
			return *this;
		}

		constexpr polar_complex& operator/=(const polar_complex& rhs) noexcept
		{
			abs_ /= rhs.abs;
			arg_ = normalize_arg(arg_ - rhs.arg_);
			return *this;
		}

		constexpr polar_complex& operator+=(const T& rhs) noexcept
		{
			const T r = real();
			const T arg = stm::atan2(imag(), rhs + r);
			abs_ = stm::sqrt(norm() + (rhs * rhs) + T{ 2 } * rhs * r);
			arg_ = arg;
			return *this;
		}

		constexpr polar_complex& operator-=(const T& rhs) noexcept
		{
			return *this += (-rhs);
		}

		constexpr polar_complex& operator*=(const T& rhs) noexcept
		{
			abs_ *= rhs;
			return *this;
		}

		constexpr polar_complex& operator/=(const T& rhs) noexcept
		{
			abs_ /= rhs;
			return *this;
		}

	private:
		/**
		* @brief Normalizes angle argument to range [-pi, pi]
		* @param arg value must be [-2pi, 2pi]
		*/
		static constexpr T normalize_arg(const T& arg) { return (arg <= stm::pi) ? (arg >= -stm::pi ? arg : arg + stm::pi) : arg; }

		T abs_{};
		T arg_{};
	};
}

#endif /* STM_POLAR_COMPLEX_H */