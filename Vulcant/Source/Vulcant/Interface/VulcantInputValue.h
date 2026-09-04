#pragma once

namespace Vulcant
{
    enum class VulcantInputValue : short
    {
        // --- Special/System ---
        Unknown = 0,
        None,

        // --- Mouse Buttons ---
        MouseLeft,
        MouseRight,
        MouseMiddle,
        MouseButton4,
        MouseButton5,
        MouseButton6,
        MouseButton7,
        MouseButton8,
        MouseWheelUp,
        MouseWheelDown,
        MouseWheelLeft,
        MouseWheelRight,

        // --- Keyboard: Alphanumeric ---
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        Num0,
        Num1,
        Num2,
        Num3,
        Num4,
        Num5,
        Num6,
        Num7,
        Num8,
        Num9,

        // --- Keyboard: Modifiers ---
        Shift,
        Control,
        Alt,
        Super, // Cmd on Mac, Win key on Windows

        // --- Keyboard: Functions/Arrows ---
        Esc,
        Space,
        Enter,
        Backspace,
        Tab,
        ArrowUp,
        ArrowDown,
        ArrowLeft,
        ArrowRight,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,

        // --- Keyboard: Numpad/Symbols ---
        Add,
        Subtract,
        Multiply,
        Divide,
        Decimal,
        Comma,
        Period,
        Slash,
        Backslash,
        Backtick,
        BracketLeft,
        BracketRight,
        Semicolon,
        Apostrophe
    };
}