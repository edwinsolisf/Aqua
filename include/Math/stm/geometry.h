#ifndef STM_GEOMETRY_H
#define STM_GEOMETRY_H

#include "common.h"

namespace stm
{
	template<Real T, std::size_t Dims>
	using point = stm::vector<T, Dims>;

	template<Number T>
	using point3 = stm::vector<T, 3>;

	template<Number T>
	using point2 = stm::vector<T, 3>;

	template<Real T, std::size_t Dims>
	class ray
	{
	public:
		constexpr ray() noexcept = default;
		constexpr ray(const ray&) noexcept = default;
		constexpr ray(ray&&) noexcept = default;
		constexpr ray& operator=(const ray&) noexcept = default;
		constexpr ray& operator=(ray&&) noexcept = default;
		~ray() noexcept = default;

		constexpr ray(const stm::point<T, Dims>& origin, const stm::vector<T, Dims>& direction, bool normalize = false) noexcept
			:origin_{ origin }, direction_{ direction }
		{
			if (normalize)
				direction_.unit();
		}

		constexpr auto& origin() noexcept { return origin_; }
		constexpr const auto& origin() const noexcept { return origin_; }

		constexpr auto& direction() noexcept { return direction_; }
		constexpr const auto& direction() const noexcept { return direction_; }

		static constexpr std::size_t dimensions() noexcept { return Dims; }

		constexpr point<T, Dims> at(const T& scalar) const noexcept
		{
			return origin_ + (direction_ * scalar);
		}

		constexpr ray& translate(const vector<T, Dims>& translation) noexcept
		{
			origin_ += translation;
			return *this;
		}

		constexpr ray& rotate(const vector<T, Dims>& axis, const T& angle_rads) noexcept;

		constexpr ray& scale(const T& scale) noexcept
		{
			direction_ *= scale;
			return *this;
		}

		constexpr friend bool operator==(const ray& lhs, const ray& rhs) noexcept
		{
			return (lhs.origin_ == rhs.origin_) && (lhs.direction_ == rhs.direction_);
		}

		constexpr friend bool operator!=(const ray& lhs, const ray& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		friend std::ostream& operator<<<T, Dims>(std::ostream&, const ray&);

	private:
		stm::point<T, Dims> origin_{};
		stm::vector<T, Dims> direction_{};
	};

	template<Real T, std::size_t Dims>
	class sphere
	{
	public:
		constexpr sphere() noexcept = default;
		constexpr sphere(const sphere&) noexcept = default;
		constexpr sphere(sphere&&) noexcept = default;
		constexpr sphere& operator=(const sphere&) noexcept = default;
		constexpr sphere& operator=(sphere&&) noexcept = default;
		~sphere() noexcept = default;

		constexpr sphere(const T& radius, const stm::point<T, Dims>& origin = {}) noexcept
			:origin_{origin}, radius_{radius}
		{}

		constexpr T& radius() noexcept { return radius_; }
		constexpr const T& radius() const noexcept { return radius_; }

		constexpr auto& origin() noexcept { return origin_; }
		constexpr const auto& origin() const noexcept { return origin_; }

		constexpr sphere& translate(const stm::vector<T, Dims>& translation) noexcept
		{
			origin_ += translation;
			return *this;
		}

		constexpr sphere& scale(const T& scale) noexcept
		{
			origin *= scale;
			return *this;
		}

		constexpr friend bool operator==(const sphere& lhs, const sphere& rhs) noexcept
		{
			return (lhs.origin_ == rhs.origin_) && (lhs.radius_ == rhs.radius_);
		}

		constexpr friend bool operator!=(const sphere& lhs, const sphere& rhs) noexcept
		{
			return !(lhs == rhs);
		}

		friend std::ostream& operator<<<T, Dims>(std::ostream&, const sphere&);

	private:
		stm::point<T, Dims> origin_{};
		T radius_{};
	};

	template<Real T, std::size_t Dims>
	class plane
	{
	public:
		constexpr plane() noexcept = default;
		constexpr plane(const plane&) noexcept = default;
		constexpr plane(plane&&) noexcept = default;
		constexpr plane& operator=(const plane&) noexcept = default;
		constexpr plane& operator=(plane&&) noexcept = default;
		~sphere() noexcept = default;
	private:
		stm::point<T, Dims> origin_;
		stm::point<T, Dims> normal_;
	};

	template<Real T, std::size_t Dims>
	constexpr std::pair<std::size_t, std::array<T, 2>> intersection(const ray<T, Dims>& ray, const sphere<T, Dims>& sphere) noexcept
	{
		const auto& m = ray.direction();
		const auto& a = ray.origin();
		const auto& r0 = sphere.origin();
		const auto& R = sphere.radius();

		const auto diff = a - r0;
		const auto m_norm = m.norm();
		const T lhs = m * diff;
		const T rhs = (diff.norm() - R * R) * m_norm;
		T determinant = lhs * lhs - rhs;

		if (determinant < T{ 0 })
		{
			return { 0 , std::array<T, 2>{} };
		}
		else if (determinant > T{ 0 })
		{
			const T sqrtdet = stm::sqrt(determinant);
			return { 2 , std::array<T, 2>{(-lhs + sqrtdet) / m_norm, (-lhs - sqrtdet) / m_norm} };
		}
		else
		{
			return { 1 , std::array<T, 2>{ -lhs / m_norm , 0 } };
		}
	}

	template<Number T>
	using circle = sphere<T, 2>;

	template<Number T>
	using sphere3 = sphere<T, 3>;
}

#endif /* STM_GEOMETRY_H */