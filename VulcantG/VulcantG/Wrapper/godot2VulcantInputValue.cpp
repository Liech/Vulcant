#include "godot2VulcantInputValue.h"
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <unordered_map>

namespace Vulcant::VulcantG::Wrapper
{
    // Mouse Mapping
    static const std::unordered_map<godot::MouseButton, VulcantInputValue> godotMouse2VulcantMap = {
        {        godot::MouseButton::MOUSE_BUTTON_LEFT,       VulcantInputValue::MouseLeft },
        {       godot::MouseButton::MOUSE_BUTTON_RIGHT,      VulcantInputValue::MouseRight },
        {      godot::MouseButton::MOUSE_BUTTON_MIDDLE,     VulcantInputValue::MouseMiddle },
        {    godot::MouseButton::MOUSE_BUTTON_XBUTTON1,    VulcantInputValue::MouseButton4 },
        {    godot::MouseButton::MOUSE_BUTTON_XBUTTON2,    VulcantInputValue::MouseButton5 },
        {    godot::MouseButton::MOUSE_BUTTON_WHEEL_UP,    VulcantInputValue::MouseWheelUp },
        {  godot::MouseButton::MOUSE_BUTTON_WHEEL_DOWN,  VulcantInputValue::MouseWheelDown },
        {  godot::MouseButton::MOUSE_BUTTON_WHEEL_LEFT,  VulcantInputValue::MouseWheelLeft },
        { godot::MouseButton::MOUSE_BUTTON_WHEEL_RIGHT, VulcantInputValue::MouseWheelRight },
    };

    // Full Keyboard Mapping
    static const std::unordered_map<godot::Key, VulcantInputValue> godotKeyboard2VulcantMap = {
        {        godot::Key::KEY_SPACE,        VulcantInputValue::Space },
        {   godot::Key::KEY_APOSTROPHE,   VulcantInputValue::Apostrophe },
        {        godot::Key::KEY_COMMA,        VulcantInputValue::Comma },
        {        godot::Key::KEY_MINUS,     VulcantInputValue::Subtract },
        {       godot::Key::KEY_PERIOD,       VulcantInputValue::Period },
        {        godot::Key::KEY_SLASH,        VulcantInputValue::Slash },
        {            godot::Key::KEY_0,         VulcantInputValue::Num0 },
        {            godot::Key::KEY_1,         VulcantInputValue::Num1 },
        {            godot::Key::KEY_2,         VulcantInputValue::Num2 },
        {            godot::Key::KEY_3,         VulcantInputValue::Num3 },
        {            godot::Key::KEY_4,         VulcantInputValue::Num4 },
        {            godot::Key::KEY_5,         VulcantInputValue::Num5 },
        {            godot::Key::KEY_6,         VulcantInputValue::Num6 },
        {            godot::Key::KEY_7,         VulcantInputValue::Num7 },
        {            godot::Key::KEY_8,         VulcantInputValue::Num8 },
        {            godot::Key::KEY_9,         VulcantInputValue::Num9 },
        {    godot::Key::KEY_SEMICOLON,    VulcantInputValue::Semicolon },
        {        godot::Key::KEY_EQUAL,          VulcantInputValue::Add },
        {            godot::Key::KEY_A,            VulcantInputValue::A },
        {            godot::Key::KEY_B,            VulcantInputValue::B },
        {            godot::Key::KEY_C,            VulcantInputValue::C },
        {            godot::Key::KEY_D,            VulcantInputValue::D },
        {            godot::Key::KEY_E,            VulcantInputValue::E },
        {            godot::Key::KEY_F,            VulcantInputValue::F },
        {            godot::Key::KEY_G,            VulcantInputValue::G },
        {            godot::Key::KEY_H,            VulcantInputValue::H },
        {            godot::Key::KEY_I,            VulcantInputValue::I },
        {            godot::Key::KEY_J,            VulcantInputValue::J },
        {            godot::Key::KEY_K,            VulcantInputValue::K },
        {            godot::Key::KEY_L,            VulcantInputValue::L },
        {            godot::Key::KEY_M,            VulcantInputValue::M },
        {            godot::Key::KEY_N,            VulcantInputValue::N },
        {            godot::Key::KEY_O,            VulcantInputValue::O },
        {            godot::Key::KEY_P,            VulcantInputValue::P },
        {            godot::Key::KEY_Q,            VulcantInputValue::Q },
        {            godot::Key::KEY_R,            VulcantInputValue::R },
        {            godot::Key::KEY_S,            VulcantInputValue::S },
        {            godot::Key::KEY_T,            VulcantInputValue::T },
        {            godot::Key::KEY_U,            VulcantInputValue::U },
        {            godot::Key::KEY_V,            VulcantInputValue::V },
        {            godot::Key::KEY_W,            VulcantInputValue::W },
        {            godot::Key::KEY_X,            VulcantInputValue::X },
        {            godot::Key::KEY_Y,            VulcantInputValue::Y },
        {            godot::Key::KEY_Z,            VulcantInputValue::Z },
        {  godot::Key::KEY_BRACKETLEFT,  VulcantInputValue::BracketLeft },
        {    godot::Key::KEY_BACKSLASH,    VulcantInputValue::Backslash },
        { godot::Key::KEY_BRACKETRIGHT, VulcantInputValue::BracketRight },
        {    godot::Key::KEY_QUOTELEFT,     VulcantInputValue::Backtick },
        {       godot::Key::KEY_ESCAPE,          VulcantInputValue::Esc },
        {        godot::Key::KEY_ENTER,        VulcantInputValue::Enter },
        {          godot::Key::KEY_TAB,          VulcantInputValue::Tab },
        {    godot::Key::KEY_BACKSPACE,    VulcantInputValue::Backspace },
        {           godot::Key::KEY_UP,      VulcantInputValue::ArrowUp },
        {         godot::Key::KEY_DOWN,    VulcantInputValue::ArrowDown },
        {         godot::Key::KEY_LEFT,    VulcantInputValue::ArrowLeft },
        {        godot::Key::KEY_RIGHT,   VulcantInputValue::ArrowRight },
        {        godot::Key::KEY_SHIFT,        VulcantInputValue::Shift },
        {         godot::Key::KEY_CTRL,      VulcantInputValue::Control },
        {          godot::Key::KEY_ALT,          VulcantInputValue::Alt },
        {         godot::Key::KEY_META,        VulcantInputValue::Super },
        {           godot::Key::KEY_F1,           VulcantInputValue::F1 },
        {           godot::Key::KEY_F2,           VulcantInputValue::F2 },
        {           godot::Key::KEY_F3,           VulcantInputValue::F3 },
        {           godot::Key::KEY_F4,           VulcantInputValue::F4 },
        {           godot::Key::KEY_F5,           VulcantInputValue::F5 },
        {           godot::Key::KEY_F6,           VulcantInputValue::F6 },
        {           godot::Key::KEY_F7,           VulcantInputValue::F7 },
        {           godot::Key::KEY_F8,           VulcantInputValue::F8 },
        {           godot::Key::KEY_F9,           VulcantInputValue::F9 },
        {          godot::Key::KEY_F10,          VulcantInputValue::F10 },
        {          godot::Key::KEY_F11,          VulcantInputValue::F11 },
        {          godot::Key::KEY_F12,          VulcantInputValue::F12 }
    };

    VulcantInputValue godotKeyboard2Vulcant(const godot::Ref<godot::InputEventKey>& p_event)
    {
        auto it = godotKeyboard2VulcantMap.find(p_event->get_keycode());
        return (it != godotKeyboard2VulcantMap.end()) ? it->second : VulcantInputValue::None;
    }

    VulcantInputValue godotMouse2Vulcant(const godot::Ref<godot::InputEventMouseButton>& p_event)
    {
        auto it = godotMouse2VulcantMap.find(p_event->get_button_index());
        return (it != godotMouse2VulcantMap.end()) ? it->second : VulcantInputValue::None;
    }
}