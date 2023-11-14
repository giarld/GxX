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

#ifndef GXX_BASEDEVICE_H
#define GXX_BASEDEVICE_H

#include <gxx/eventsys.h>
#include <gxx/device/device_type.h>

#include <gx/gglobal.h>


namespace gxx
{

class BaseDeviceHandler;

class BaseDeviceDriverInterface;

class BaseDeviceEvent;

/**
 * Device driver interface (implemented by window graphics interface)
 */
class GX_API DeviceDriver
{
public:
    explicit DeviceDriver();

    virtual ~DeviceDriver();

public:
    virtual bool deviceSupport(DeviceType::Enum type) = 0;

    void registerDeviceHandler(BaseDeviceHandler *deviceHandler);

    void unregisterDeviceHandler(BaseDeviceHandler *deviceHandler);

    void postDeviceEvent(BaseDeviceEvent *event);

    void processDeviceEvents();

private:
    EventMana *mEventMana;
};


/**
 * Interface base class for device driver related methods
 */
class GX_API BaseDeviceDriverInterface
{
public:
    explicit BaseDeviceDriverInterface(DeviceDriver *dd);

    virtual ~BaseDeviceDriverInterface();

protected:
    void _postDeviceEvent(BaseDeviceEvent *event);

private:
    DeviceDriver *mDd;
};


/**
 * Device Event Base Class
 */
class GX_API BaseDeviceEvent : public Event
{
public:
    explicit BaseDeviceEvent(DeviceType::Enum deviceType, uint32_t deviceId);

    virtual ~BaseDeviceEvent();

public:
    uint32_t deviceId() const;

    Event *getEEvent() const;

protected:
    void setEEvent(Event *event);

private:
    Event *mEEvent = nullptr;
    uint32_t mDeviceId = 0;
};


/**
 * Device User Event Processor Interface
 */
class GX_API BaseDeviceHandler : public EventHandler
{
public:
    explicit BaseDeviceHandler(DeviceType::Enum deviceType, uint32_t deviceId);

    virtual ~BaseDeviceHandler();

public:
    DeviceType::Enum deviceType() const;

    uint32_t deviceId() const;

protected:
    void handleEvent(Event *event) final;

    virtual void handleDeviceEEvent(Event *eEvent) = 0;

protected:
    DeviceDriver *deviceDriver();

private:
    friend class DeviceDriver;

    DeviceDriver *mDeviceDriver = nullptr;
    DeviceType::Enum mDeviceType = DeviceType::Unknown;
    uint32_t mDeviceId = 0;
};

}


#endif //GXX_BASEDEVICE_H
