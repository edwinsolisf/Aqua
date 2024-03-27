#ifndef STM_FUNCTION_H
#define STM_FUNCTION_H

#include "concept.h"
#include "internal.h"

namespace stm
{
    template<Number T>
    constexpr T tan(T x)
    {
        if (std::is_constant_evaluated())
            return internal::const_tan(x);
        else
            return internal::run_tan(x);
    }

}

#endif /* STM_FUNCTION_H */