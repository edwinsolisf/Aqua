#ifndef STM_ERROR_H
#define STM_ERROR_H

#include <string>
#include <stdexcept>

namespace stm
{
	namespace intern
	{
		inline static void out_of_bounds_except(int index, int size)
		{
			throw std::out_of_range{ "Cannot access element [" + std::to_string(index) + "] from container of size [" +
				std::to_string(index) + "]"};
		}

		inline static void int_zero_division_except()
		{
			throw std::runtime_error{ "Cannot divide quantity by zero" };
		}
	}
}

#endif /* STM_ERROR_H */