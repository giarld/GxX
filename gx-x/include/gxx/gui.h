/*
 * Copyright (c) 2019 Gxin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef GXX_GUI_H
#define GXX_GUI_H

#include <gx/gglobal.h>


namespace gxx
{
struct MouseButton
{
    enum Enum : uint8_t
    {
        None,
        Left,
        Middle,
        Right,

        Count
    };
};

struct Modifier
{
    enum Enum : uint8_t
    {
        None = 0,
        LeftAlt = 0x01,
        RightAlt = 0x02,
        LeftCtrl = 0x04,
        RightCtrl = 0x08,
        LeftShift = 0x10,
        RightShift = 0x20,
        LeftMeta = 0x40,
        RightMeta = 0x80,
    };
};

struct Key
{
    enum Enum : uint8_t
    {
        None = 0,
        Esc,
        Return,
        Tab,
        Space,
        Backspace,
        Up,
        Down,
        Left,
        Right,
        Insert,
        Delete,
        Home,
        End,
        PageUp,
        PageDown,
        Print,
        Plus,
        Minus,
        LeftBracket,
        RightBracket,
        Semicolon,
        Quote,
        Comma,
        Period,
        Slash,
        Backslash,
        Tilde,
        CapsLock,
        NumLock,
        Menu,
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
        NumPad0,
        NumPad1,
        NumPad2,
        NumPad3,
        NumPad4,
        NumPad5,
        NumPad6,
        NumPad7,
        NumPad8,
        NumPad9,
        Key0,
        Key1,
        Key2,
        Key3,
        Key4,
        Key5,
        Key6,
        Key7,
        Key8,
        Key9,
        KeyA,
        KeyB,
        KeyC,
        KeyD,
        KeyE,
        KeyF,
        KeyG,
        KeyH,
        KeyI,
        KeyJ,
        KeyK,
        KeyL,
        KeyM,
        KeyN,
        KeyO,
        KeyP,
        KeyQ,
        KeyR,
        KeyS,
        KeyT,
        KeyU,
        KeyV,
        KeyW,
        KeyX,
        KeyY,
        KeyZ,

        Count
    };
};

struct KeyAction
{
    enum Enum : uint8_t
    {
        Release,
        Press,
        Repeat,

        Count
    };
};

struct GamepadButton
{
    enum Enum : uint8_t
    {
        GamepadA,
        GamepadB,
        GamepadX,
        GamepadY,
        GamepadLeftBumper,
        GamepadRightBumper,
        GamepadLeftThumb,
        GamepadRightThumb,
        GamepadUp,
        GamepadDown,
        GamepadLeft,
        GamepadRight,
        GamepadBack,
        GamepadStart,
        GamepadGuide,

        Count
    };
};

struct GamepadAxis
{
    enum Enum : uint8_t
    {
        AxisLeftX,
        AxisLeftY,
        AxisRightX,
        AxisRightY,
        AxisLeftTrigger,
        AxisRightTrigger,

        Count
    };
};

struct GamepadAction
{
    enum Enum : uint8_t
    {
        Connected,
        DisConnected,

        Count
    };
};

struct CursorMode
{
    enum Enum : uint8_t
    {
        Normal,
        Hidden,
        Disabled,

        Count
    };
};

struct WindowState
{
    enum Enum
    {
        Normal = 0,
        Minimized,
        Maximized,
        FullScreen
    };
};

struct WindowFlag
{
    enum Enum
    {
        ShowBorder = 0x01,
        Resizable = 0x02
    };
};

typedef uint8_t WindowFlags;

}

#endif //GXX_GUI_H
