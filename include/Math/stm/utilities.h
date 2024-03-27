#ifndef STM_UTILITIES_H
#define STM_UTILITIES_H

#include <iostream>

#include "common.h"

namespace stm
{
	template<Real T>
	std::ostream& operator<<(std::ostream& stream, const stm::complex<T>& value)
	{
		return stream << "( " << value.r << " , " << value.i << " )";
	}

	template<Number T, std::size_t Dims>
	std::ostream& operator<<(std::ostream& stream, const stm::vector<T, Dims>& value)
	{
		stream << "[ ";
		for (std::size_t i = 0; i < Dims - 1; ++i)
			stream << value[i] << " , ";
		return stream << value[Dims - 1] << " ]";
	}

	template<Integer T>
	std::ostream& operator<<(std::ostream& stream, const stm::fraction<T>& value)
	{
		return stream << value.num() << "/" << value.denom();
	}

	//template<Number T>

	template<Number T, std::size_t Dims>
	std::ostream& operator<<(std::ostream& stream, const stm::sphere<T, Dims>& value)
	{
		return stream << "Sphere{ r=" << value.radius() << " , origin=" << value.origin() << " }";
	}

	template<Number T>
	std::ostream& operator<<(std::ostream& stream, const stm::sphere<T, 2>& value)
	{
		return stream << "Circle{ r=" << value.radius() << " , origin=" << value.origin() << " }";
	}

	template<Number T, std::size_t Dims>
	std::ostream& operator<<(std::ostream& stream, const stm::ray<T, Dims>& value)
	{
		return stream << "Ray{ dir=" << value.direction() << " , origin=" << value.origin() << " }";
	}
}

#endif /* STM_UTILITIES_H */