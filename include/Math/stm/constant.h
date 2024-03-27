#ifndef STM_CONSTANT_H
#define STM_CONSTANT_H

#include <numbers>

namespace stm
{
	inline static constexpr const float pi_f = 3.1415926535897932384626433f;
	inline static constexpr const float e_f = 2.71828182845904523536f;
	inline static constexpr const float phi_f = 1.61803398874989484820f;
	inline static constexpr const float sqrt2_f = 1.41421356237309504880f;
	inline static constexpr const float sqrt3_f = 1.73205080756887729352f;
	inline static constexpr const float sqrt5_f = 2.23606797749978969640f;
	inline static constexpr const float cbrt2_f = 1.25992104989487316476f;
	inline static constexpr const float cbrt3_f = 1.44224957030740838232f;
	inline static constexpr const float ln2_f = 0.69314718055994530941f;

	inline static constexpr const double pi = 3.1415926535897932384626433;
	inline static constexpr const double e = 2.71828182845904523536;
	inline static constexpr const double phi = 1.61803398874989484820;
	inline static constexpr const double sqrt2 = 1.41421356237309504880;
	inline static constexpr const double sqrt3 = 1.73205080756887729352;
	inline static constexpr const double sqrt5 = 2.23606797749978969640;
	inline static constexpr const double cbrt2 = 1.25992104989487316476;
	inline static constexpr const double cbrt3 = 1.44224957030740838232;
	inline static constexpr const double ln2 = 0.69314718055994530941;

	inline static constexpr const long double pi_l = 3.141592653589793238462643383279502884L;
	inline static constexpr const long double inf_l = std::numeric_limits<long double>::infinity();
	inline static constexpr const double inf = std::numeric_limits<double>::infinity();
	inline static constexpr const float inf_f = std::numeric_limits<float>::infinity();

	// SI units
	namespace physics
	{
		inline static constexpr const float c_f = 299'792'458.f;
		inline static constexpr const float G_f = 6.674'30e-11f;
		inline static constexpr const float h_f = 6.62607015e-34f;
		inline static constexpr const float h_bar_f = 1.054571817e-34f;
		inline static constexpr const float e0_f = 8.8541878128e-12f;
		inline static constexpr const float u0_f = 1.25663706212e-6f;
		inline static constexpr const float k_f = 8.9875517923e9f;
		inline static constexpr const float e_charge_f = 1.602176634e-19f;
		inline static constexpr const float N_A_f = 6.02214076e23f;

		inline static constexpr const double c = 299'792'458.;
		inline static constexpr const double G = 6.674'30e-11;
		inline static constexpr const double h = 6.62607015e-34;
		inline static constexpr const double h_bar = 1.054571817e-34;
		inline static constexpr const double e0 = 8.8541878128e-12;
		inline static constexpr const double u0 = 1.25663706212e-6;
		inline static constexpr const double k = 8.9875517923e9;
		inline static constexpr const double e_charge = 1.602176634e-19;
		inline static constexpr const double N_A = 6.02214076e23;
	}

}

#endif /* STM_COMPLEX_H */