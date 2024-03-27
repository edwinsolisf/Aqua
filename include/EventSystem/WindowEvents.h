#pragma once

#include "NullEvent.h"

namespace Aqua
{
    struct WindowResizeEventData
    {
        int width;
        int height;
    };

    using WindowCloseEventData = NullEventData;
}