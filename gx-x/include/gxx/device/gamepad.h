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

#ifndef GXX_GAMEPAD_H
#define GXX_GAMEPAD_H

#include <gxx/gui.h>
#include <gxx/device/basedevice.h>

#include <unordered_map>
#include <string>


namespace gxx
{

struct GamepadStateInfo
{
    uint32_t jid;
    std::string name;
    GamepadAction::Enum action;
};

struct GamepadInfo
{
    /**
     * Corresponds to the status of each button on the Gamepad Button
     */
    KeyAction::Enum buttons[GamepadButton::Count];

    /**
     * Corresponding to the status of each analog axis of the Gamepad Axis
     * [-1, 1]
     */
    float axes[GamepadAxis::Count];
};

struct GamepadEventKey
{
    enum Enum
    {
        StateChange,
        Update
    };
};

class IGamepadDeviceDriver;

class GX_API Gamepad : public BaseDeviceHandler
{
public:
    using GamepadStateEventFunc = std::function<void(const GamepadStateInfo &)>;

    using GamepadEventFunc = std::function<void(uint32_t, const GamepadInfo &)>;

public:
    explicit Gamepad();

public:
    void setGamepadStateEventCallback(GamepadStateEventFunc func);

    void setGamepadEventCallback(uint32_t jid, GamepadEventFunc func);

    std::vector<GamepadStateInfo> getConnectedGamepadStateInfos() const;

protected:
    void handleDeviceEEvent(Event *eEvent) override;

private:
    GamepadStateEventFunc mGamepadStateEventFunc;
    std::unordered_map<uint32_t, GamepadEventFunc> mGamepadEventFuncs;

    IGamepadDeviceDriver *mDd = nullptr;
};


class GX_API GamepadStateEvent : public BaseDeviceEvent
{
public:
    class CEvent : public Event
    {
    public:
        explicit CEvent(GamepadStateInfo info)
                : Event(GamepadEventKey::StateChange),
                  gamepadStateInfo(std::move(info))
        {}

    public:
        GamepadStateInfo gamepadStateInfo;
    };

public:
    explicit GamepadStateEvent(const GamepadStateInfo &info)
            : BaseDeviceEvent(DeviceType::GamePad, 0)
    {
        setEEvent(new CEvent(info));
    }
};


class GX_API GamepadEvent : public BaseDeviceEvent
{
public:
    class CEvent : public Event
    {
    public:
        explicit CEvent(uint32_t jid, GamepadInfo gamepadInfo)
                : Event(GamepadEventKey::Update),
                  jid(jid),
                  gamepadInfo(gamepadInfo)
        {}

    public:
        uint32_t jid;
        GamepadInfo gamepadInfo;
    };

public:
    explicit GamepadEvent(uint32_t jid, const GamepadInfo &gamepadInfo)
            : BaseDeviceEvent(DeviceType::GamePad, 0)
    {
        setEEvent(new CEvent(jid, gamepadInfo));
    }
};


/** IGamepadDeviceDriver **/
class GX_API IGamepadDeviceDriver : public BaseDeviceDriverInterface
{
public:
    explicit IGamepadDeviceDriver(DeviceDriver *dd) : BaseDeviceDriverInterface(dd)
    {}

public:
    void postGamepadStateEvent(const GamepadStateInfo &info);

    void postGamepadUpdateEvent(uint32_t jid, const GamepadInfo &info);

    virtual std::vector<GamepadStateInfo> getConnectedGamepadStateInfos() = 0;
};


}

#endif //GXX_GAMEPAD_H
