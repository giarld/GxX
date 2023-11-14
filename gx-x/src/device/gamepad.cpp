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

#include "gxx/device/gamepad.h"

#include <gxx/application.h>
#include <gxx/app_entry.h>


namespace gxx
{

Gamepad::Gamepad()
        : BaseDeviceHandler(DeviceType::GamePad, 0)
{
    mDd = dynamic_cast<IGamepadDeviceDriver *>(Application::application()->getAppContext());
}

void Gamepad::setGamepadStateEventCallback(Gamepad::GamepadStateEventFunc func)
{
    mGamepadStateEventFunc = std::move(func);
}

void Gamepad::setGamepadEventCallback(uint32_t jid, Gamepad::GamepadEventFunc func)
{
    if (func == nullptr) {
        auto it = mGamepadEventFuncs.find(jid);
        if (it != mGamepadEventFuncs.end()) {
            mGamepadEventFuncs.erase(it);
        }
    } else {
        mGamepadEventFuncs.emplace(jid, std::move(func));
    }
}

std::vector<GamepadStateInfo> Gamepad::getConnectedGamepadStateInfos() const
{
    return mDd->getConnectedGamepadStateInfos();
}

void Gamepad::handleDeviceEEvent(Event *eEvent)
{
    if (!eEvent) {
        return;
    }
    if (eEvent->key() == GamepadEventKey::StateChange) {
        if (mGamepadStateEventFunc) {
            auto *e = dynamic_cast<GamepadStateEvent::CEvent *>(eEvent);
            mGamepadStateEventFunc(e->gamepadStateInfo);
        }
    } else if (eEvent->key() == GamepadEventKey::Update) {
        auto *e = dynamic_cast<GamepadEvent::CEvent *>(eEvent);
        auto it = mGamepadEventFuncs.find(e->jid);
        if (it != mGamepadEventFuncs.end()) {
            it->second(e->jid, e->gamepadInfo);
        }
    }
}

void IGamepadDeviceDriver::postGamepadStateEvent(const GamepadStateInfo &info)
{
    _postDeviceEvent(new GamepadStateEvent(info));
}

void IGamepadDeviceDriver::postGamepadUpdateEvent(uint32_t jid, const GamepadInfo &info)
{
    _postDeviceEvent(new GamepadEvent(jid, info));
}

}