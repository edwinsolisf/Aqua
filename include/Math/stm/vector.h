#ifndef STM_VECTOR_H
#define STM_VECTOR_H

#include "common.h"

namespace stm
{
	template<Number T, std::size_t Dims>
	class vector
	{
	public:
		using value_type = T;
		using underlying_num_type = underlying_num_t<T>;


		constexpr vector() noexcept = default;
		constexpr vector(const vector&) noexcept = default;
		constexpr vector(vector&&) noexcept = default;
		~vector() = default;

		explicit constexpr vector(const std::array<T, Dims>& values) noexcept
		{
			std::copy(values.begin(), values.end(), data_);
		}

		explicit constexpr vector(const T* values)
		{
			std::copy(values, values + Dims, data_);
		}

		explicit constexpr vector(std::span<T, Dims> values) noexcept
		{
			std::copy(values.begin(), values.end(), data_);
		}

		template<typename... Ts>
		constexpr vector(T&& first, Ts&&... rest) noexcept
			:data_{ std::forward<T>(first), std::forward<Ts>(rest) ... }
		{}

		static constexpr std::size_t size() noexcept { return Dims; }

		constexpr T& operator[](std::size_t index) noexcept { assert(index < size()); return data_[index]; }
		constexpr const T& operator[](std::size_t index) const noexcept { assert(index < size()); return data_[index]; }

		template<std::size_t index>
		constexpr T& get() noexcept { static_assert(index < size()); return data_[index]; }

		template<std::size_t index>
		constexpr const T& get() const noexcept { static_assert(index < Dims); return data_[index]; }

		constexpr T& at(std::size_t index) { if (index >= size()) intern::out_of_bounds_except(index, size()); return data_[index]; }
		constexpr const T& at(std::size_t index) const { if (index >= size()) intern::out_of_bounds_except(index, size()); return data_[index]; }

		constexpr friend vector operator+(const vector& lhs, const vector& rhs) noexcept
		{
			vector out { lhs };
			return out += rhs;
		}

		constexpr friend vector operator-(const vector& lhs, const vector& rhs) noexcept
		{
			vector out{ lhs };
			return out -= rhs;
		}

		constexpr friend vector operator*(const vector& lhs, const T& rhs) noexcept
		{
			vector out{ lhs };
			return out *= rhs;
		}

		constexpr friend vector operator/(const vector& lhs, const T& rhs) noexcept(noexcept(lhs /= rhs))
		{
			vector out{ lhs };
			return out /= rhs;
		}

		constexpr friend T operator*(const vector& lhs, const vector& rhs) noexcept
		{
			T out{};
			for (std::size_t i = 0; i < size(); ++i)
				out += lhs[i] * rhs[i];
			return out;
		}

		constexpr vector& operator+=(const vector& rhs) noexcept
		{
			for (std::size_t i = 0; i < size(); ++i)
				data_[i] += rhs[i];
			return *this;
		}

		constexpr vector& operator-=(const vector& rhs) noexcept
		{
			for (std::size_t i = 0; i < size(); ++i)
				data_[i] -= rhs[i];
			return *this;
		}

		constexpr vector& operator*=(const T& rhs) noexcept
		{
			for (std::size_t i = 0; i < size(); ++i)
				data_[i] *= rhs;
			return *this;
		}

		constexpr vector& operator/=(const T& rhs) noexcept(Float<underlying_num_type>)
		{
			if constexpr (Integer<T>)
				if (rhs == static_cast<T>(0)) intern::int_zero_division_except();
			for (std::size_t i = 0; i < size(); ++i)
				data_[i] /= rhs;
			return *this;
		}

		constexpr friend bool operator==(const vector& lhs, const vector& rhs) noexcept
		{
			for (std::size_t i = 0; i < size(); ++i)
				if (lhs[i] != rhs[i]) return false;
			return true;
		}

		constexpr friend bool operator!=(const vector& lhs, const vector& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		constexpr auto norm() const noexcept
		{
			auto out = stm::norm(data_[0]);
			for (std::size_t i = 1; i < size(); ++i)
				out += stm::norm(data_[i]);
			return out;
		}

		constexpr auto abs() const noexcept
		{
			return stm::sqrt(this->norm());
		}

		constexpr vector unit() const noexcept(Float<underlying_num_type>)
		{
			return *this / abs();
		}

		template<Number U>
		constexpr vector<U, Dims> cast() const noexcept
		{
			static_assert(!(Real<U> && Complex<T>), "Invalid conversion from complex number to real number");
			using Tu = underlying_num_type;
			using Uu = underlying_num_t<U>;
			if constexpr (Complex<U>)
			{
				if constexpr (Real<T>)
					return []<std::size_t... I>(const vector& v, std::index_sequence<I...>) {
						return vector<U, Dims>{ static_cast<Uu>(v.get<I>())... };
					}(*this, std::make_index_sequence<size()>{});
				else
					return []<std::size_t... I>(const vector& v, std::index_sequence<I...>) {
						return vector<U, Dims>{ U{static_cast<Uu>(v.get<I>())}... };
					}(*this, std::make_index_sequence<size()>{});
			}
			else if constexpr (Real<U>)
			{
				//if constexpr (Real<T>)
				return []<std::size_t... I>(const vector& v, std::index_sequence<I...>) {
					return vector<U, Dims>{ static_cast<U>(v.get<I>())... };
				}(*this, std::make_index_sequence<size()>{});
			}
		}

		operator std::span<T, Dims>() const noexcept { return { data_ }; }


		static_assert(Dims != 0, "Vector must contain at least one element");
		// friend std::ostream& operator<<<T, Dims>(std::ostream&, const vector&);

	private:
		T data_[Dims]{};
	};

	template<typename First, typename... Rest> vector(First&&, Rest...) -> vector<First, sizeof...(Rest) + 1>;

	template<Number T, std::size_t Dims>
	inline constexpr T dot(const vector<T, Dims>& lhs, const vector<T, Dims>& rhs) noexcept
	{
		return lhs.dot(rhs);
	}

	template<Number T, std::size_t Dims>
	inline constexpr auto unit(const vector<T, Dims>& vec) noexcept
	{
		return vec.unit();
	}

	template<Number T, std::size_t Dims>
	inline constexpr T length(const vector <T, Dims>& vec) noexcept
	{
		return vec.abs();
	}
}

#endif /* STM_VECTOR_H */