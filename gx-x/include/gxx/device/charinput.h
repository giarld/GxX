/*
 * Copyright (c) 2020 Gxin
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

#ifndef GXX_CHARINPUT_H
#define GXX_CHARINPUT_H

#include <gxx/device/basedevice.h>

#include <string>


namespace gxx
{

class GX_API CharInput : public BaseDeviceHandler
{
public:
    using CharInputEventFunc = std::function<void(const std::string &)>;

public:
    explicit CharInput(uint32_t windowId)
            : BaseDeviceHandler(DeviceType::CharInput, windowId)
    {}

public:
    void setCharInputEventCallback(CharInputEventFunc callback);

protected:
    void handleDeviceEEvent(Event *eEvent) override;

private:
    CharInputEventFunc mCharInputEventCb;
};


struct CharInputEventKey
{
    enum
    {
        CharInput
    };
};


class GX_API CharInputEvent : public BaseDeviceEvent
{
public:
    class CCEvent : public Event
    {
    public:
        explicit CCEvent(std::string c)
                : Event(CharInputEventKey::CharInput), mCharInput(std::move(c))
        {}

    public:
        std::string mCharInput;
    };

public:
    explicit CharInputEvent(uint32_t windowId, const std::string &c)
            : BaseDeviceEvent(DeviceType::CharInput, windowId)
    {
        setEEvent(new CCEvent(c));
    }
};

class GX_API ICharInputDriver : public BaseDeviceDriverInterface
{
public:
    explicit ICharInputDriver(DeviceDriver *dd) : BaseDeviceDriverInterface(dd)
    {}

public:
    void postCharInputEvent(uint32_t windowId, const std::string &c);
};

}

#endif //GXX_CHARINPUT_H
