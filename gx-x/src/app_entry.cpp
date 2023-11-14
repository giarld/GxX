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

#include "gxx/app_entry.h"

#include <gxx/app_native.h>
#include <gxx/window.h>

#include "gx/debug.h"


using namespace gx;

namespace gxx
{

AppContext::AppContext()
        : EventHandler(),
          DeviceDriver(),
          IGamepadDeviceDriver(this),
          IKeyboardDeviceDriver(this),
          IMouseDeviceDriver(this),
          ICharInputDriver(this)
{
    mEventMana = new EventMana();
    mEventMana->addEventHandler(AppNativeEvents::Exit, this);
    mEventMana->addEventHandler(AppNativeEvents::SetWindowSize, this);
    mEventMana->addEventHandler(AppNativeEvents::SetWindowPos, this);
    mEventMana->addEventHandler(AppNativeEvents::SetWindowTitle, this);
    mEventMana->addEventHandler(AppNativeEvents::SetWindowState, this);
    mEventMana->addEventHandler(AppNativeEvents::SetWindowFlags, this);
    mEventMana->addEventHandler(AppNativeEvents::ShowInfoDialog, this);
    mEventMana->addEventHandler(AppNativeEvents::SetCursor, this);
    mEventMana->addEventHandler(AppNativeEvents::SetCursorMode, this);
    mEventMana->addEventHandler(AppNativeEvents::SetCursorPos, this);
}

AppContext::~AppContext()
{
    delete mEventMana;
    mEventMana = nullptr;
}

void AppContext::postExitWindow(WindowContext *window)
{
    mEventMana->postEvent(new ANWinExitEvent(window));
    this->postNativeEvent();
}

void AppContext::postSetWindowSize(WindowContext *window, uint32_t w, uint32_t h)
{
    mEventMana->postEvent(new ANSetWinSizeEvent(window, w, h));
    this->postNativeEvent();
}

void AppContext::postSetWindowPos(WindowContext *window, int32_t x, int32_t y)
{
    mEventMana->postEvent(new ANSetWinPosEvent(window, x, y));
    this->postNativeEvent();
}

void AppContext::postSetWindowTitle(WindowContext *window, const std::string &title)
{
    mEventMana->postEvent(new ANSetWinTitleEvent(window, title));
    this->postNativeEvent();
}

void AppContext::postSetWindowState(WindowContext *window, WindowState::Enum state)
{
    mEventMana->postEvent(new ANSetWinStateEvent(window, state));
    this->postNativeEvent();
}

void AppContext::postSetWindowFlags(WindowContext *window, WindowFlags flags)
{
    mEventMana->postEvent(new ANSetWinFlagsEvent(window, flags));
    this->postNativeEvent();
}

void AppContext::postShowInfoDialog(gxx::WindowContext *window, const std::string &title, const std::string &msg)
{
    mEventMana->postEvent(new ANShowInfoDialogEvent(window, title, msg));
    this->postNativeEvent();
}

void AppContext::postSetCursor(WindowContext *window, const Cursor &cursor)
{
    mEventMana->postEvent(new ANSetCursorEvent(window, cursor));
    this->postNativeEvent();
}

void AppContext::postSetCursorMode(WindowContext *window, CursorMode::Enum mode)
{
    mEventMana->postEvent(new ANSetCursorModeEvent(window, mode));
    this->postNativeEvent();
}

void AppContext::postSetCursorPos(WindowContext *window, int32_t x, int32_t y)
{
    mEventMana->postEvent(new ANSetCursorPosEvent(window, x, y));
    this->postNativeEvent();
}

void AppContext::destroyWindow(gxx::WindowHandle *wh)
{
    wh->destroy();
    nativeDestroyW(wh);
    delete wh->mWindow;
}

NWindow *AppContext::createNWindow(WindowHandle *wh)
{
    return gxx::createNativeWindow();
}

bool AppContext::nativeFrameW(gxx::WindowHandle *wh)
{
    if (wh->mNativeWindow) {
        if (wh->mNativeWindow->isInited()) {
            return wh->mNativeWindow->frame();
        } else {
            return true;
        }
    }
    return false;
}

void AppContext::nativeDestroyW(WindowHandle *wh)
{
    wh->nativeEndWait();
    if (wh->mNativeWindow) {
        wh->mNativeWindow->destroy();
    }
}

/** ========= ========= **/
int AppContext::init(Application *app)
{
    mScheduler = GTimerScheduler::create("MainScheduler");
    GTimerScheduler::makeGlobal(mScheduler);
    return nativeInit(this);
}

int AppContext::run(Application *app)
{
    mScheduler->start();
    while (!mWindows.empty() || !mDelayedTasks.empty()) {
        while (!mDelayedTasks.empty()) {
            auto task = mDelayedTasks.front();
            mDelayedTasks.pop();
            if (task) {
                task();
            }
        }

        mScheduler->loop();

        auto it = mWindows.begin();
        while (it != mWindows.end()) {
            WindowHandle *wh = *it;
            // [1] native window的事件处理
            if (!nativeFrameW(wh)) {
                wh->postExitEvent();
            }
            // [2] window的事件处理与绘制
            wh->frame();
            // [3] window到native window的事件处理
            this->processEvents();
            if (wh->mExited) {
                destroyWindow(wh);
                it = mWindows.erase(it);
            } else {
                it++;
            }
        }
    }

    mScheduler->stop();
    mScheduler = nullptr;

    return nativeTerminate(this);
}

void AppContext::addWindow(Window *window)
{
    if (!window) {
        return;
    }
    WindowHandle *wh = dynamic_cast<WindowHandle *>(window->mWinContext.get());
    if (!wh) {
        return;
    }
    auto it = mWindows.begin();
    while (it != mWindows.end()) {
        if (*it == wh) {
            return;
        }
        it++;
    }
    wh->mAppContext = this;

    mDelayedTasks.emplace([=]() {
        // wh->init();   // init交给native来进行，存在平台创建Window为异步过程(ex: Android)
        NWindow *nw = createNWindow(wh);
        if (!nw) {
            return;
        }
        if (!nw->init(this, wh)) {
            Log("create and init new Native Window failure");
            delete nw;
            return;
        }
        wh->mNativeWindow = nw;
        mWindows.push_back(wh);
    });
}

void AppContext::getDesktopSize(uint32_t &w, uint32_t &h)
{
    nativeGetDesktopSize(w, h);
}

void AppContext::closeAll()
{
    for (WindowHandle *wh : mWindows) {
        wh->close();
    }
}

void AppContext::handleANEvent(ANBaseEvent *event)
{
    WindowHandle *wh = dynamic_cast<WindowHandle *>(event->window);
    NWindow *nw;
    if (!wh || !(nw = wh->mNativeWindow)) {
        return;
    }
    switch (event->key()) {
        case AppNativeEvents::Exit: {
            nw->exit();
        }
            break;
        case AppNativeEvents::SetWindowSize: {
            const auto *e = dynamic_cast<const ANSetWinSizeEvent *>(event);
            if (!e) {
                break;
            }
            nw->setWindowSize(e->width, e->height);
        }
            break;
        case AppNativeEvents::SetWindowPos: {
            const auto *e = dynamic_cast<const ANSetWinPosEvent *>(event);
            if (!e) {
                break;
            }
            nw->setWindowPos(e->x, e->y);
        }
            break;
        case AppNativeEvents::SetWindowTitle: {
            const auto *e = dynamic_cast<const ANSetWinTitleEvent *>(event);
            if (!e) {
                break;
            }
            nw->setWindowTitle(e->title);
        }
            break;
        case AppNativeEvents::SetWindowState: {
            const auto *e = dynamic_cast<const ANSetWinStateEvent *>(event);
            if (!e) {
                break;
            }
            nw->setWindowState(e->state);
        }
            break;
        case AppNativeEvents::SetWindowFlags: {
            const auto *e = dynamic_cast<const ANSetWinFlagsEvent *>(event);
            if (!e) {
                break;
            }
            nw->setWindowFlags(e->flags);
        }
            break;
        case AppNativeEvents::ShowInfoDialog: {
            const auto *e = dynamic_cast<const ANShowInfoDialogEvent *>(event);
            if (!e) {
                break;
            }
            nw->showInfoDialog(e->title, e->message);
        }
            break;
        case AppNativeEvents::SetCursor: {
            const auto *e = dynamic_cast<const ANSetCursorEvent *>(event);
            if (!e) {
                break;
            }
            nw->setCursor(e->cursor);
        }
            break;
        case AppNativeEvents::SetCursorMode: {
            const auto *e = dynamic_cast<const ANSetCursorModeEvent *>(event);
            if (!e) {
                break;
            }
            nw->setCursorMode(e->mode);
        }
            break;
        case AppNativeEvents::SetCursorPos: {
            const auto *e = dynamic_cast<const ANSetCursorPosEvent *>(event);
            if (!e) {
                break;
            }
            nw->setCursorPosition(e->x, e->y);
        }
            break;
    }
}

void AppContext::postNativeEvent()
{

}

bool AppContext::deviceSupport(DeviceType::Enum type)
{
    return nativeDeviceSupport(type);
}

std::vector<GamepadStateInfo> AppContext::getConnectedGamepadStateInfos()
{
    return nativeGetConnectedGamepadStateInfos();
}

}