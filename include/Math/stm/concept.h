#ifndef STM_CONCEPT_H
#define STM_CONCEPT_H

#include <concepts>
#include <type_traits>
#include <complex>

namespace stm
{
	template<typename T>
	concept Integer = std::is_integral_v<T>;

	template<typename T>
	concept Float = std::is_floating_point_v<T>;

	template<Integer T>
	class fraction;

	template<typename T>
	concept Rational = Integer<T> || std::is_same_v<T, stm::fraction<typename T::value_type>>;

	template<typename T>
	concept Real = Integer<T> || Float<T>;

	template<Real>
	class complex;

	template<Float>
	class polar_complex;

	template<Real>
	class quaternion;

	template<typename T>
	concept Complex = std::is_same_v<T, std::complex<typename T::value_type>> ||
					  std::is_same_v<T, stm::complex<typename T::value_type>> ||
					  std::is_same_v<T, stm::polar_complex<typename T::value_type>>;

	template<typename T>
	concept Number = Real<T> || Complex<T>;

	template<Number, std::size_t>
	class vector;

	template<Number, std::size_t>
	class array;

	template<Number, std::size_t, std::size_t>
	class matrix;

	template<typename T, std::size_t N>
	concept Vector = std::is_same_v<T, stm::complex<typename T::value_type>> ||
					 std::is_same_v<T, stm::vector<typename T::value_type, N>> ||
					 std::is_same_v<T, stm::quaternion<typename T::value_type>>;

	template<typename T, std::size_t M, std::size_t N>
	concept Array = std::is_same_v<T, stm::vector<typename T::value_type, M>> ||
					std::is_same_v<T, stm::matrix<typename T::value_type, M, N>> ||
					std::is_same_v<T, stm::array<typename T::value_type, M>>;

	template<typename T, typename = void> struct underlying_num { using type = T; };
	template<typename T> struct underlying_num<T, std::void_t<typename T::value_type>> { using type = typename T::value_type; };

	template<typename T>
	using underlying_num_t = underlying_num<T>::type;

	template<Real, std::size_t>
	class ray;

	template<Real, std::size_t>
	class sphere;

	template<Real, std::size_t>
	class plane;

	enum class Angle_unit_type;
	template<Real, Angle_unit_type>
	struct angle_unit;
}

#endif /* STM_CONCEPT_H */