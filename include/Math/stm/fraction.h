#ifndef STM_FRACTION_H
#define STM_FRACTION_H

#include "common.h"
#include "numeric.h"

namespace stm
{
	template<Integer T>
	class fraction
	{
	public:
		using value_type = T;
		using numerator_type = T;
		using denominator_type = std::make_unsigned_t<T>;

		constexpr fraction() noexcept = default;
		constexpr fraction(const fraction&) noexcept = default;
		constexpr fraction(fraction&&) noexcept = default;

		constexpr fraction(const numerator_type& numerator, const denominator_type& denominator) noexcept
			:num_{numerator}, denom_{denominator}
		{
			simplify();
		}

		constexpr fraction& operator=(const fraction&) noexcept = default;
		constexpr fraction& operator=(fraction&&) noexcept = default;

		constexpr numerator_type& num() noexcept { return num_; }
		constexpr const numerator_type& num() const noexcept { return num_; }

		constexpr denominator_type& denom() noexcept { return denom_; }
		constexpr const denominator_type& denom() const noexcept { return denom_; }

		template<Float U>
		constexpr U decimal() const noexcept { return static_cast<U>(num_) / static_cast<U>(denom_); }

		constexpr fraction operator+() const noexcept { return *this; }
		constexpr fraction operator-() const noexcept { return { -num_ , denom_ }; }

		constexpr fraction& simplify() noexcept
		{
			using denom_t = denominator_type;
			using num_t = numerator_type;

			denom_t gcd;
			if (num_ >= num_t{0})
			{
				gcd = stm::gcd(static_cast<denom_t>(num_), denom_);
				num_ = static_cast<num_t>(static_cast<denom_t>(num_) / gcd);
			}
			else
			{
				gcd = stm::gcd(static_cast<denom_t>(-num_), denom_);
				num_ = -static_cast<num_t>(static_cast<denom_t>(-num_) / gcd);
			}
			denom_ /= gcd;
			return *this;
		}

		constexpr friend fraction operator+(const fraction& lhs, const fraction& rhs) noexcept
		{
			auto lcm = stm::lcm(lhs.denom_, rhs.denom_);
			auto left = lcm / lhs.denom_;
			auto right = lcm / rhs.denom_;
			auto num = static_cast<numerator_type>(left) * lhs.num_ + static_cast<numerator_type>(right) * rhs.num_;
			return fraction{ num , lcm }.simplify();
		}

		constexpr friend fraction operator-(const fraction& lhs, const fraction& rhs) noexcept
		{
			return lhs + (-rhs);
		}

		constexpr friend fraction operator*(const fraction& lhs, const fraction& rhs) noexcept
		{
			return fraction{ lhs.num_ * rhs.num_ , lhs.denom_ * rhs.denom_ }.simplify();
		}

		constexpr friend fraction operator/(const fraction& lhs, const fraction& rhs) noexcept
		{
			return fraction{ rhs.num_ >= 0? lhs.num_ : -lhs.num_ , rhs.num_ } * fraction{ rhs.denom_ , rhs.denom_ };
		}

		friend std::ostream& operator<<<T>(std::ostream&, const fraction<T>&);

	private:
		numerator_type num_{ 0 };
		denominator_type denom_{ 1 };
	};
}

#endif /* STM_FRACTION_H */