#pragma once

namespace Aqua
{
    // From glfw3.h
    enum class MouseCode : uint16_t
    {
        Button0                = 0,
        Button1                = 1,
        Button2                = 2,
        Button3                = 3,
        Button4                = 4,
        Button5                = 5,
        Button6                = 6,
        Button7                = 7,

        ButtonLast             = Button7,
        ButtonLeft             = Button0,
        ButtonRight            = Button1,
        ButtonMiddle           = Button2
    };

    enum class MouseAction : uint16_t
    {
        Release = 0,
        Press = 1,
        Repeat = 2
    };

    struct MouseMoveEventData
    {
        double x, y;
    };

    struct MouseClickEventData
    {
        MouseCode code;
        MouseAction action;
    };
}