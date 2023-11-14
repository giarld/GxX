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

#include "gxx/device/keyboard.h"


namespace gxx
{

void Keyboard::setKeyPressEventCallback(gxx::Keyboard::KeyPressEventFunc callback)
{
    mKeyPressEventCb = std::move(callback);
}

void Keyboard::setKeyReleaseEventCallback(gxx::Keyboard::KeyReleaseEventFunc callback)
{
    mKeyReleaseEventCb = std::move(callback);
}

void Keyboard::handleDeviceEEvent(Event *eEvent)
{
    if (!eEvent) {
        return;
    }

    if (eEvent->key() == 0) {
        const auto *_e = dynamic_cast<const KeyEvent::CCEvent *>(eEvent);
        if (!_e) {
            return;
        }
        if (_e->action == KeyAction::Enum::Release) {
            if (mKeyReleaseEventCb) {
                mKeyReleaseEventCb(_e->key, _e->modifier);
            }
        } else {
            if (mKeyPressEventCb) {
                mKeyPressEventCb(_e->key, _e->modifier);
            }
        }
    }
}

void IKeyboardDeviceDriver::postKeyEvent(uint32_t windowId, gxx::Key::Enum key, uint8_t modifier,
                                    gxx::KeyAction::Enum action)
{
    _postDeviceEvent(new KeyEvent(windowId, key, modifier, action));
}

}
