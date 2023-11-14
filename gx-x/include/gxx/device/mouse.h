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

#ifndef GXX_MOUSE_H
#define GXX_MOUSE_H

#include <gxx/device/basedevice.h>
#include <gxx/gui.h>


namespace gxx
{

class GX_API Mouse : public BaseDeviceHandler
{
public:
    using MouseMoveEventFunc = std::function<void(int, int)>;
    using MousePressEventFunc = std::function<void(MouseButton::Enum)>;
    using MouseReleaseEventFunc = std::function<void(MouseButton::Enum)>;
    using MouseScrollEventFunc = std::function<void(double, double)>;

public:
    explicit Mouse(uint32_t windowId)
            : BaseDeviceHandler(DeviceType::Mouse, windowId)
    {}

public:
    void setMouseMoveEventCallback(MouseMoveEventFunc callback);

    void setMousePressEventCallback(MousePressEventFunc callback);

    void setMouseReleaseEventCallback(MouseReleaseEventFunc callback);

    void setMouseScrollEventCallback(MouseScrollEventFunc callback);

protected:
    void handleDeviceEEvent(Event *eEvent) override;

private:
    MouseMoveEventFunc mMouseMoveEventCb;

    MousePressEventFunc mMousePressEventCb;

    MouseReleaseEventFunc mMouseReleaseEventCb;

    MouseScrollEventFunc mMouseScrollEventCb;
};


struct MouseEventKey
{
    enum Enum
    {
        MouseMove,
        MouseButton,
        MouseScroll
    };
};


class GX_API MouseMoveEvent : public BaseDeviceEvent
{
public:
    class CCEvent : public Event
    {
    public:
        explicit CCEvent(int32_t x, int32_t y)
                : Event(MouseEventKey::MouseMove), x(x), y(y)
        {}

        ~CCEvent() override
        = default;

    public:
        int32_t x;
        int32_t y;
    };

public:
    explicit MouseMoveEvent(uint32_t windowId, int32_t x, int32_t y)
            : BaseDeviceEvent(DeviceType::Mouse, windowId)
    {
        setEEvent(new CCEvent(x, y));
    }
};


class GX_API MouseButtonEvent : public BaseDeviceEvent
{
public:
    class CCEvent : public Event
    {
    public:
        explicit CCEvent(MouseButton::Enum button, KeyAction::Enum action)
                : Event(MouseEventKey::MouseButton), button(button), action(action)
        {}

    public:
        MouseButton::Enum button;
        KeyAction::Enum action;
    };

public:
    explicit MouseButtonEvent(uint32_t windowId, MouseButton::Enum button, KeyAction::Enum action)
            : BaseDeviceEvent(DeviceType::Mouse, windowId)
    {
        setEEvent(new CCEvent(button, action));
    }
};

class GX_API MouseScrollEvent : public BaseDeviceEvent
{
public:
    class CCEvent : public Event
    {
    public:
        explicit CCEvent(double xOffset, double yOffset)
                : Event(MouseEventKey::MouseScroll), xOffset(xOffset), yOffset(yOffset)
        {}

    public:
        double xOffset;
        double yOffset;
    };

public:
    explicit MouseScrollEvent(uint32_t windowId, double xoffset, double yoffset)
            : BaseDeviceEvent(DeviceType::Mouse, windowId)
    {
        setEEvent(new CCEvent(xoffset, yoffset));
    }
};


class GX_API IMouseDeviceDriver : public BaseDeviceDriverInterface
{
public:
    explicit IMouseDeviceDriver(DeviceDriver *dd) : BaseDeviceDriverInterface(dd)
    {}

public:
    void postMouseMoveEvent(uint32_t windowId, int x, int y);

    void postMouseButtonEvent(uint32_t windowId, MouseButton::Enum button, KeyAction::Enum action);

    void postMouseScrollEvent(uint32_t windowId, double xoffset, double yoffset);
};

}

#endif //GXX_MOUSE_H
