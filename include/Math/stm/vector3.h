#ifndef STM_VECTOR_3_H
#define STM_VECTOR_3_H

#include "common.h"
#include "math.h"

namespace stm
{
	template<Number T, std::size_t Dims>
	class vector;

	template<Number T>
	class vector<T, 3>
	{
	public:
		using value_type = T;
		using underlying_num_type = underlying_num_t<T>;

		constexpr vector() noexcept = default;
		constexpr vector(const vector&) noexcept = default;
		constexpr vector(vector&&) noexcept = default;
		constexpr vector& operator=(const vector&) noexcept = default;
		constexpr vector& operator=(vector&&) noexcept = default;
		~vector() noexcept = default;

		constexpr vector(const T& x, const T& y, const T& z) noexcept
			:x{x} , y{y}, z{z}
		{
		}

		constexpr vector(std::span<T, 3> values) noexcept
		{
			std::copy(values.begin(), values.end(), data());
		}

		constexpr T* data() noexcept {	return &x; }
		constexpr const T* data() const noexcept {	return &x; }
		static constexpr std::size_t size() noexcept { return 3; }

		constexpr T& operator[](std::size_t index) noexcept { assert(index < size()); return data()[index]; }
		constexpr const T& operator[](std::size_t index) const noexcept { assert(index < size()); return data()[index]; }

		template<typename std::size_t index>
		constexpr T& get() noexcept { static_assert(index < size()); return data()[index]; }

		template<typename std::size_t index>
		constexpr const T& get() const noexcept { static_assert(index < size()); return data()[index]; }

		constexpr T& at(std::size_t index) { if (index >= size()) intern::out_of_bounds_except(index, size()); return data()[index]; }
		constexpr const T& at(std::size_t index) const { if (index >= size()) intern::out_of_bounds_except(index, size()); return data()[index]; }

		constexpr vector operator+() const noexcept { return *this; }
		constexpr vector operator-() const noexcept { return { -x, -y, -z }; }

		constexpr friend vector operator+(const vector& lhs, const vector& rhs) noexcept
		{
			return { lhs.x + rhs.x , lhs.y + rhs.y , lhs.z + rhs.z };
		}

		constexpr friend vector operator-(const vector& lhs, const vector& rhs) noexcept
		{
			return { lhs.x - rhs.x , lhs.y - rhs.y , lhs.z - rhs.z };
		}

		constexpr friend vector operator*(const vector& lhs, const T& rhs) noexcept
		{
			return { lhs.x * rhs , lhs.y * rhs , lhs.z * rhs };
		}

		constexpr friend vector operator*(const T& lhs, const vector& rhs) noexcept
		{
			return rhs * lhs;
		}

		constexpr friend T operator*(const vector& lhs, const vector& rhs) noexcept
		{
			return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
		}

		constexpr friend vector operator/(const vector& lhs, const T& rhs) noexcept(Float<underlying_num_type>)
		{
			if constexpr (Integer<T>)
				if (rhs == static_cast<T>(0)) intern::int_zero_division_except();
			return { lhs.x / rhs , lhs.y / rhs , lhs.z / rhs };
		}

		constexpr vector& operator+=(const vector& rhs) noexcept
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			return *this;
		}

		constexpr vector& operator-=(const vector& rhs) noexcept
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			return *this;
		}

		constexpr vector& operator*=(const T& rhs) noexcept
		{
			x *= rhs;
			y *= rhs;
			z *= rhs;
			return *this;
		}

		constexpr vector& operator/=(const T& rhs) noexcept(Float<underlying_num_type>)
		{
			if constexpr (Integer<underlying_num_type>)
				if (rhs == static_cast<T>(0)) intern::int_zero_division_except();
			x /= rhs;
			y /= rhs;
			z /= rhs;
			return *this;
		}

		constexpr friend bool operator==(const vector& lhs, const vector& rhs) noexcept
		{
			return (lhs.x == rhs.x) && (lhs.y == rhs.y);
		}

		constexpr friend bool operator!=(const vector& lhs, const vector& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		constexpr auto norm() const noexcept
		{
			return stm::norm(x) + stm::norm(y) + stm::norm(z);
		}

		constexpr auto abs() const noexcept { return stm::sqrt(this->norm()); }

		constexpr vector unit() const noexcept(Float<underlying_num_type>) { return *this / abs(); }

		template<Number U>
		constexpr vector<U, 3> cast() const noexcept
		{
			static_assert(!(Real<U> && Complex<T>), "Invalid conversion from complex number to real number");
			using Tu = underlying_num_type;
			using Uu = underlying_num_t<U>;
			if constexpr (Complex<U>)
			{
				if constexpr (Real<T>)
					return { static_cast<Uu>(x) , static_cast<Uu>(y) , static_cast<Uu>(z) };
				else
					return { U{static_cast<Uu>(x)} , U{static_cast<Uu>(y)} , U{static_cast<Uu>(z)} };
			}
			else if constexpr (Real<U>)
			{
				//if constexpr (Real<T>)
					return { static_cast<U>(x) , static_cast<U>(y) , static_cast<U>(z) };
			}
		}

		operator std::span<T, 3>() const noexcept { return { data() }; }
		// friend std::ostream& operator<<<T>(std::ostream&, const vector&);

	public:
		T x{}, y{}, z{};
	};

	template<Number T>
	inline constexpr vector<T, 3> cross(const stm::vector<T, 3>& lhs, const stm::vector<T, 3>& rhs) noexcept
	{
		return {
			lhs.y * rhs.z - lhs.z * rhs.y,
			lhs.z * rhs.x - lhs.x * rhs.z,
			lhs.x * rhs.y - lhs.y * rhs.x
		};
	}

	template<Number T>
	inline constexpr vector<T, 3> dot(const stm::vector<T, 3>& lhs, const stm::vector<T, 3>& rhs) noexcept
	{
		return lhs.dot(rhs);
	}

	template<Number T>
	inline constexpr auto unit(const vector<T, 3>& vec) noexcept
	{
		return vec.unit();
	}

	template<Number T>
	inline constexpr T length(const vector <T, 3>& vec) noexcept
	{
		return vec.abs();
	}

	using vec3f = vector<float, 3>;
	using vec3d = vector<double, 3>;
	using vec3i = vector<int, 3>;
}

#endif /* STM_VECTOR_3_H */