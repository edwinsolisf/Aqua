#ifndef STM_INTERNAL_H
#define STM_INTERNAL_H

#include "concept.h"

namespace stm
{
    namespace internal
    {
        template<Number T>
        constexpr T const_tan_impl(T x, T res, std::size_t i)
        {
            return T{1} / (T{1} / res - T{i * 2 + 1} / x);
        }

        template<Number T>
        constexpr T const_tan(T x)
        {
            T res = 0;
            T next = 1;
            for(std::size_t i = 0; res != next; ++i)
            {
                res = next;
                next = const_tan_impl(x, res, i);
            }
            return res;
        }
    }
}

#endif /* STM_INTERNAL_H */