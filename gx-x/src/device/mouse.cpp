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

#include "gxx/device/mouse.h"


namespace gxx
{

void Mouse::setMouseMoveEventCallback(Mouse::MouseMoveEventFunc callback)
{
    mMouseMoveEventCb = std::move(callback);
}

void Mouse::setMousePressEventCallback(Mouse::MousePressEventFunc callback)
{
    mMousePressEventCb = std::move(callback);
}

void Mouse::setMouseReleaseEventCallback(Mouse::MouseReleaseEventFunc callback)
{
    mMouseReleaseEventCb = std::move(callback);
}

void Mouse::setMouseScrollEventCallback(Mouse::MouseScrollEventFunc callback)
{
    mMouseScrollEventCb = std::move(callback);
}

void Mouse::handleDeviceEEvent(Event *eEvent)
{
    if (!eEvent) {
        return;
    }
    switch (eEvent->key()) {
        case MouseEventKey::MouseMove: {
            auto *_e = dynamic_cast<MouseMoveEvent::CCEvent *>(eEvent);
            if (!_e) {
                break;
            }
            if (mMouseMoveEventCb) {
                mMouseMoveEventCb(_e->x, _e->y);
            }
        }
            break;
        case MouseEventKey::MouseButton: {
            auto *_e = dynamic_cast<MouseButtonEvent::CCEvent *>(eEvent);
            if (!_e) {
                break;
            }
            if (_e->action == KeyAction::Release && mMouseReleaseEventCb) {
                mMouseReleaseEventCb(_e->button);
            } else if (mMousePressEventCb) {
                mMousePressEventCb(_e->button);
            }
        }
            break;
        case MouseEventKey::MouseScroll: {
            auto *_e = dynamic_cast<MouseScrollEvent::CCEvent *>(eEvent);
            if (!_e) {
                break;
            }
            if (mMouseScrollEventCb) {
                mMouseScrollEventCb(_e->xOffset, _e->yOffset);
            }
        }
            break;
    }
}


void IMouseDeviceDriver::postMouseMoveEvent(uint32_t windowId, int32_t x, int32_t y)
{
    _postDeviceEvent(new MouseMoveEvent(windowId, x, y));
}

void IMouseDeviceDriver::postMouseButtonEvent(uint32_t windowId, gxx::MouseButton::Enum button,
                                              gxx::KeyAction::Enum action)
{
    _postDeviceEvent(new MouseButtonEvent(windowId, button, action));
}

void IMouseDeviceDriver::postMouseScrollEvent(uint32_t windowId, double xoffset, double yoffset)
{
    _postDeviceEvent(new MouseScrollEvent(windowId, xoffset, yoffset));
}

}