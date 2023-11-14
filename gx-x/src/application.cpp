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

#include "gxx/application.h"
#include "gxx/app_entry.h"

#include <gxx/window.h>
#include <gxx/device/basedevice.h>

#include <gx/debug.h>

namespace gxx
{

/** Application **/
Application *Application::sApplication = nullptr;

Application::Application(int argc, char **argv)
{
    mAppARG.argc = argc;
    mAppARG.argv = argv;
    sApplication = this;
    int initRet;
    if ((initRet = init()) != 0) {
        Log("init application failure! ret=%d", initRet);
        exit(initRet);
    }
}

Application::Application()
{
    sApplication = this;
    int initRet;
    if ((initRet = init()) != 0) {
        Log("init application failure! ret=%d", initRet);
        exit(initRet);
    }
}

Application::~Application()
{
    delete mAppContext;
}

int Application::init()
{
#if GX_PLATFORM_WINDOWS
    system("chcp 65001");
#endif
    mAppContext = new AppContext;
    return mAppContext->init(this);
}

int Application::exec()
{
    int ret = mAppContext->run(this);
    return ret;
}

void Application::quit()
{
    if (mAppContext) {
        mAppContext->closeAll();
    }
}

void Application::addWindow(Window *window)
{
    if (mAppContext) {
        mAppContext->addWindow(window);
    }
}

void Application::setName(const std::string &appName)
{
    this->mName = appName;
}

std::string Application::getName()
{
    return this->mName;
}

void Application::getDesktopSize(uint32_t &w, uint32_t &h)
{
    if (mAppContext) {
        mAppContext->getDesktopSize(w, h);
    }
}

AppARG *Application::appArg()
{
    return &mAppARG;
}

AppContext *Application::getAppContext() const
{
    return mAppContext;
}

bool Application::deviceSupport(DeviceType::Enum type)
{
    if (mAppContext) {
        return mAppContext->deviceSupport(type);
    }
    return false;
}

void Application::registerDeviceHandler(gxx::BaseDeviceHandler *deviceHandler)
{
    if (mAppContext) {
        mAppContext->registerDeviceHandler(deviceHandler);
    }
}

void Application::unregisterDeviceHandler(gxx::BaseDeviceHandler *deviceHandler)
{
    if (mAppContext) {
        mAppContext->unregisterDeviceHandler(deviceHandler);
    }
}

/** static functions **/
Application *Application::application()
{
    return Application::sApplication;
}

}