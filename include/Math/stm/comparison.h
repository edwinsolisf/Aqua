#ifndef STM_COMPARISON_H
#define STM_COMPARISON_H

#include "common.h"

namespace stm
{
    template<Real T, Real U>
    constexpr auto min_of(T a, U b) noexcept
    {
        return a < b ? a : b;
    }

    template<Real T, Real>
    constexpr auto max_of(T a, T b) noexcept
    {
        return a > b ? a : b;
    }
}

#endif /* STM_COMPARISON_H */