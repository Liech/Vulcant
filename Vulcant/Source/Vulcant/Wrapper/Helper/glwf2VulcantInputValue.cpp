#include "glwf2VulcantInputValue.h"

#include <GLFW/glfw3.h>
#include <unordered_map>

namespace Vulcant::Wrapper
{
    static const std::unordered_map<int, VulcantInputValue> glwfMouse2VulcantMap = {

        { GLFW_MOUSE_BUTTON_1,    VulcantInputValue::MouseLeft },
        { GLFW_MOUSE_BUTTON_2,   VulcantInputValue::MouseRight },
        { GLFW_MOUSE_BUTTON_3,  VulcantInputValue::MouseMiddle },
        { GLFW_MOUSE_BUTTON_4, VulcantInputValue::MouseButton4 },
        { GLFW_MOUSE_BUTTON_5, VulcantInputValue::MouseButton5 },
        { GLFW_MOUSE_BUTTON_6, VulcantInputValue::MouseButton6 },
        { GLFW_MOUSE_BUTTON_7, VulcantInputValue::MouseButton7 },
        { GLFW_MOUSE_BUTTON_8, VulcantInputValue::MouseButton8 },
    };

    static const std::unordered_map<int, VulcantInputValue> glwfKeyboard2VulcantMap = {
        // Alphanumeric & Symbols
        {         GLFW_KEY_SPACE,        VulcantInputValue::Space },
        {    GLFW_KEY_APOSTROPHE,   VulcantInputValue::Apostrophe },
        {         GLFW_KEY_COMMA,        VulcantInputValue::Comma },
        {         GLFW_KEY_MINUS,     VulcantInputValue::Subtract },
        {        GLFW_KEY_PERIOD,       VulcantInputValue::Period },
        {         GLFW_KEY_SLASH,        VulcantInputValue::Slash },
        {             GLFW_KEY_0,         VulcantInputValue::Num0 },
        {             GLFW_KEY_1,         VulcantInputValue::Num1 },
        {             GLFW_KEY_2,         VulcantInputValue::Num2 },
        {             GLFW_KEY_3,         VulcantInputValue::Num3 },
        {             GLFW_KEY_4,         VulcantInputValue::Num4 },
        {             GLFW_KEY_5,         VulcantInputValue::Num5 },
        {             GLFW_KEY_6,         VulcantInputValue::Num6 },
        {             GLFW_KEY_7,         VulcantInputValue::Num7 },
        {             GLFW_KEY_8,         VulcantInputValue::Num8 },
        {             GLFW_KEY_9,         VulcantInputValue::Num9 },
        {     GLFW_KEY_SEMICOLON,    VulcantInputValue::Semicolon },
        {         GLFW_KEY_EQUAL,          VulcantInputValue::Add },
        {             GLFW_KEY_A,            VulcantInputValue::A },
        {             GLFW_KEY_B,            VulcantInputValue::B },
        {             GLFW_KEY_C,            VulcantInputValue::C },
        {             GLFW_KEY_D,            VulcantInputValue::D },
        {             GLFW_KEY_E,            VulcantInputValue::E },
        {             GLFW_KEY_F,            VulcantInputValue::F },
        {             GLFW_KEY_G,            VulcantInputValue::G },
        {             GLFW_KEY_H,            VulcantInputValue::H },
        {             GLFW_KEY_I,            VulcantInputValue::I },
        {             GLFW_KEY_J,            VulcantInputValue::J },
        {             GLFW_KEY_K,            VulcantInputValue::K },
        {             GLFW_KEY_L,            VulcantInputValue::L },
        {             GLFW_KEY_M,            VulcantInputValue::M },
        {             GLFW_KEY_N,            VulcantInputValue::N },
        {             GLFW_KEY_O,            VulcantInputValue::O },
        {             GLFW_KEY_P,            VulcantInputValue::P },
        {             GLFW_KEY_Q,            VulcantInputValue::Q },
        {             GLFW_KEY_R,            VulcantInputValue::R },
        {             GLFW_KEY_S,            VulcantInputValue::S },
        {             GLFW_KEY_T,            VulcantInputValue::T },
        {             GLFW_KEY_U,            VulcantInputValue::U },
        {             GLFW_KEY_V,            VulcantInputValue::V },
        {             GLFW_KEY_W,            VulcantInputValue::W },
        {             GLFW_KEY_X,            VulcantInputValue::X },
        {             GLFW_KEY_Y,            VulcantInputValue::Y },
        {             GLFW_KEY_Z,            VulcantInputValue::Z },
        {  GLFW_KEY_LEFT_BRACKET,  VulcantInputValue::BracketLeft },
        {     GLFW_KEY_BACKSLASH,    VulcantInputValue::Backslash },
        { GLFW_KEY_RIGHT_BRACKET, VulcantInputValue::BracketRight },
        {  GLFW_KEY_GRAVE_ACCENT,     VulcantInputValue::Backtick },

        // Navigation & Functions
        {        GLFW_KEY_ESCAPE,          VulcantInputValue::Esc },
        {         GLFW_KEY_ENTER,        VulcantInputValue::Enter },
        {           GLFW_KEY_TAB,          VulcantInputValue::Tab },
        {     GLFW_KEY_BACKSPACE,    VulcantInputValue::Backspace },
        {            GLFW_KEY_UP,      VulcantInputValue::ArrowUp },
        {          GLFW_KEY_DOWN,    VulcantInputValue::ArrowDown },
        {          GLFW_KEY_LEFT,    VulcantInputValue::ArrowLeft },
        {         GLFW_KEY_RIGHT,   VulcantInputValue::ArrowRight },

        // Modifiers (Mapping both Left/Right to a unified logical Key)
        {    GLFW_KEY_LEFT_SHIFT,        VulcantInputValue::Shift },
        {   GLFW_KEY_RIGHT_SHIFT,        VulcantInputValue::Shift },
        {  GLFW_KEY_LEFT_CONTROL,      VulcantInputValue::Control },
        { GLFW_KEY_RIGHT_CONTROL,      VulcantInputValue::Control },
        {      GLFW_KEY_LEFT_ALT,          VulcantInputValue::Alt },
        {     GLFW_KEY_RIGHT_ALT,          VulcantInputValue::Alt },
        {    GLFW_KEY_LEFT_SUPER,        VulcantInputValue::Super },
        {   GLFW_KEY_RIGHT_SUPER,        VulcantInputValue::Super },

        // Function Keys (Truncated for brevity, add F13-F25 if needed)
        {            GLFW_KEY_F1,           VulcantInputValue::F1 },
        {            GLFW_KEY_F2,           VulcantInputValue::F2 },
        {            GLFW_KEY_F3,           VulcantInputValue::F3 },
        {            GLFW_KEY_F4,           VulcantInputValue::F4 },
        {            GLFW_KEY_F5,           VulcantInputValue::F5 },
        {            GLFW_KEY_F6,           VulcantInputValue::F6 },
        {            GLFW_KEY_F7,           VulcantInputValue::F7 },
        {            GLFW_KEY_F8,           VulcantInputValue::F8 },
        {            GLFW_KEY_F9,           VulcantInputValue::F9 },
        {           GLFW_KEY_F10,          VulcantInputValue::F10 },
        {           GLFW_KEY_F11,          VulcantInputValue::F11 },
        {           GLFW_KEY_F12,          VulcantInputValue::F12 }
    };

    VulcantInputValue glwfKeyboard2Vulcant(int glwfValue)
    {
        auto it = glwfKeyboard2VulcantMap.find(glwfValue);
        if (it != glwfKeyboard2VulcantMap.end())
        {
            return it->second;
        }
        return VulcantInputValue::None;
    }

    VulcantInputValue glwfMouse2Vulcant(int glwfValue)
    {
        auto it = glwfMouse2VulcantMap.find(glwfValue);
        if (it != glwfMouse2VulcantMap.end())
        {
            return it->second;
        }
        return VulcantInputValue::None;
    }
}