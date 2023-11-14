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

#ifndef GXX_KEYBOARD_H
#define GXX_KEYBOARD_H

#include <gxx/device/basedevice.h>
#include <gxx/gui.h>


namespace gxx
{

class GX_API Keyboard : public BaseDeviceHandler
{
public:
    using KeyPressEventFunc = std::function<void(Key::Enum, uint8_t)>;
    using KeyReleaseEventFunc = std::function<void(Key::Enum, uint8_t)>;

public:
    explicit Keyboard(uint32_t windowId)
            : BaseDeviceHandler(DeviceType::Keyboard, windowId)
    {}

public:
    void setKeyPressEventCallback(KeyPressEventFunc callback);

    void setKeyReleaseEventCallback(KeyReleaseEventFunc callback);

protected:
    void handleDeviceEEvent(Event *eEvent) override;

private:
    KeyPressEventFunc mKeyPressEventCb;

    KeyReleaseEventFunc mKeyReleaseEventCb;
};


class GX_API KeyEvent : public BaseDeviceEvent
{
public:
    class CCEvent : public Event
    {
    public:
        explicit CCEvent(Key::Enum key, uint8_t modifier, KeyAction::Enum action)
                : Event(0), key(key), modifier(modifier), action(action)
        {}

    public:
        Key::Enum key;
        uint8_t modifier = 0;
        KeyAction::Enum action;
    };

public:
    explicit KeyEvent(uint32_t windowId, Key::Enum key, uint8_t modifier, KeyAction::Enum action)
            : BaseDeviceEvent(DeviceType::Keyboard, windowId)
    {
        setEEvent(new CCEvent(key, modifier, action));
    }
};


class GX_API IKeyboardDeviceDriver : public BaseDeviceDriverInterface
{
public:
    explicit IKeyboardDeviceDriver(DeviceDriver *dd) : BaseDeviceDriverInterface(dd)
    {}

public:
    void postKeyEvent(uint32_t windowId, Key::Enum key, uint8_t modifier, KeyAction::Enum action);
};

}

#endif //GXX_KEYBOARD_H
