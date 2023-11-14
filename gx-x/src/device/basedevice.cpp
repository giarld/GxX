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

#include "gxx/device/basedevice.h"

#include <gxx/application.h>

namespace gxx
{
/** DeviceDriver **/

DeviceDriver::DeviceDriver()
        : mEventMana(new EventMana())
{

}

DeviceDriver::~DeviceDriver()
{
    delete mEventMana;
    mEventMana = nullptr;
}

void DeviceDriver::registerDeviceHandler(BaseDeviceHandler *deviceHandler)
{
    mEventMana->addEventHandler(deviceHandler->deviceType(), deviceHandler);
    deviceHandler->mDeviceDriver = this;
}

void DeviceDriver::unregisterDeviceHandler(BaseDeviceHandler *deviceHandler)
{
    mEventMana->removeEventHandler(deviceHandler->deviceType(), deviceHandler);
    deviceHandler->mDeviceDriver = nullptr;
}

void DeviceDriver::postDeviceEvent(BaseDeviceEvent *event)
{
    mEventMana->postEvent(event);
}

void DeviceDriver::processDeviceEvents()
{
    mEventMana->processEvents();
}

/** BaseDeviceDriverInterface **/

BaseDeviceDriverInterface::BaseDeviceDriverInterface(gxx::DeviceDriver *dd)
        : mDd(dd)
{
}

BaseDeviceDriverInterface::~BaseDeviceDriverInterface()
{
}

void BaseDeviceDriverInterface::_postDeviceEvent(gxx::BaseDeviceEvent *event)
{
    mDd->postDeviceEvent(event);
}

/** BaseDeviceEvent **/

BaseDeviceEvent::BaseDeviceEvent(DeviceType::Enum deviceType, uint32_t deviceId)
        : Event(deviceType),
          mDeviceId(deviceId)
{

}

BaseDeviceEvent::~BaseDeviceEvent()
{
    delete mEEvent;
}

uint32_t BaseDeviceEvent::deviceId() const
{
    return mDeviceId;
}

Event *BaseDeviceEvent::getEEvent() const
{
    return mEEvent;
}

void BaseDeviceEvent::setEEvent(gxx::Event *event)
{
    this->mEEvent = event;
}

/** BaseDeviceHandler **/
BaseDeviceHandler::BaseDeviceHandler(DeviceType::Enum deviceType, uint32_t deviceId)
        : EventHandler(),
          mDeviceType(deviceType),
          mDeviceId(deviceId)
{
    Application::application()->registerDeviceHandler(this);
}

BaseDeviceHandler::~BaseDeviceHandler()
{
    Application::application()->unregisterDeviceHandler(this);
}

DeviceType::Enum BaseDeviceHandler::deviceType() const
{
    return mDeviceType;
}

uint32_t BaseDeviceHandler::deviceId() const
{
    return mDeviceId;
}

void BaseDeviceHandler::handleEvent(Event *event)
{
    if (event->key() == this->mDeviceType) {
        BaseDeviceEvent *_e = dynamic_cast<BaseDeviceEvent *>(event);
        if (_e && _e->deviceId() == this->mDeviceId) {
            handleDeviceEEvent(_e->getEEvent());
        }
    }
}

DeviceDriver *BaseDeviceHandler::deviceDriver()
{
    return this->mDeviceDriver;
}

}