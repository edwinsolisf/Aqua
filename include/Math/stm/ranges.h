#ifndef STM_RANGES_H
#define STM_RANGES_H

#include "common.h"

namespace stm
{
    template<Integer T>
    constexpr auto up_to(T end) noexcept
    {
        return std::views::iota(T{0}, end);
    }

    template<Integer T>
    constexpr auto from_up_to(T begin, T end) noexcept
    {
        return std::views::iota(begin, end);
    }
}

#endif /* STM_RANGES_H */