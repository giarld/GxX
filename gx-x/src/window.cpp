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

#include "gxx/window.h"

#include "gxx/app_entry.h"
#include "gxx/app_native.h"

#include <gxx/cursor.h>

#include <cstring>
#include <memory>


using namespace gx;

namespace gxx
{

/** ======== WindowContext ======== **/

WindowContext::WindowContext(gxx::Window *window)
        : mWindow(window)
{
    static int s_windowId = 0;
    this->mWindowId = s_windowId;
    ++s_windowId;
}

WindowContext::~WindowContext()
{
}

void WindowContext::setWindowTitle(const std::string &title)
{
    mTitle = title;
    if (mAppContext) {
        mAppContext->postSetWindowTitle(this, title);
    }
}

std::string WindowContext::getWindowTitle()
{
    return mTitle;
}

void WindowContext::setWindowState(WindowState::Enum state)
{
    mWinState = state;
    if (mAppContext) {
        mAppContext->postSetWindowState(this, state);
    }
}

WindowState::Enum WindowContext::getWindowState() const
{
    return mWinState;
}

void WindowContext::setWindowFlags(WindowFlags flags)
{
    mWinFlags = flags;
    if (mAppContext) {
        mAppContext->postSetWindowFlags(this, flags);
    }
}

WindowFlags WindowContext::getWindowFlags() const
{
    return mWinFlags;
}

void WindowContext::setWindowPos(int32_t x, int32_t y)
{
    mXPos = x;
    mYPos = y;
    if (mAppContext) {
        mAppContext->postSetWindowPos(this, x, y);
    }
}

int32_t WindowContext::getX() const
{
    return mXPos;
}

int32_t WindowContext::getY() const
{
    return mYPos;
}

void WindowContext::setWindowSize(uint32_t width, uint32_t height)
{
    mWidth = width;
    mHeight = height;
    if (mAppContext) {
        mAppContext->postSetWindowSize(this, width, height);
    }
}

uint32_t WindowContext::getWindowWidth() const
{
    return mWidth;
}

uint32_t WindowContext::getWindowHeight() const
{
    return mHeight;
}

void WindowContext::close()
{
    if (mAppContext) {
        mAppContext->postExitWindow(this);
    }
}

uint32_t WindowContext::getWindowId() const
{
    return mWindowId;
}

void WindowContext::setPlatformData(const gxx::WindowHandle::PlatformData &pd)
{
    memcpy(&mPlatformData, &pd, sizeof(PlatformData));
}

WindowContext::PlatformData *WindowContext::platformData()
{
    return &mPlatformData;
}

void WindowContext::showInfoDialog(const std::string &title, const std::string &msg)
{
    if (mAppContext) {
        mAppContext->postShowInfoDialog(this, title, msg);
    }
}

void WindowContext::setCursor(const Cursor &cursor)
{
    if (mAppContext) {
        mAppContext->postSetCursor(this, cursor);
    }
}

void WindowContext::setCursorMode(CursorMode::Enum mode)
{
    if (mAppContext) {
        mAppContext->postSetCursorMode(this, mode);
        mCursorMode = mode;
    }
}

CursorMode::Enum WindowContext::getCursorMode() const
{
    return mCursorMode;
}

void WindowContext::setCursorPosition(int32_t x, int32_t y)
{
    if (mAppContext) {
        mAppContext->postSetCursorPos(this, x, y);
    }
}

/** ======== WindowHandle ======== **/

WindowHandle::WindowHandle(Window *window)
        : WindowContext(window),
          EventHandler()
{
    mFrameTime = GTime(GTime::SteadyClock);

    mEventMana = new EventMana();
    mEventMana->addEventHandler(WinEvents::Exit, this);
    mEventMana->addEventHandler(WinEvents::WindowSize, this);
    mEventMana->addEventHandler(WinEvents::WindowPos, this);
    mEventMana->addEventHandler(WinEvents::Drop, this);
    mEventMana->addEventHandler(WinEvents::FocusChange, this);
}

WindowHandle::~WindowHandle()
{
    delete mEventMana;
    mEventMana = nullptr;

    if (mNativeWindow) {
        delete mNativeWindow;
        mNativeWindow = nullptr;
    }
}

void WindowHandle::init()
{
    mWindow->init();
    mRunning = true;
    mExited = false;
}

void WindowHandle::frame(bool update)
{
    if (mRunning) {
        mEventMana->processEvents();
        mAppContext->processDeviceEvents();

        //
        double delta = 0;
        if (mFrameTime.nanosecond() != 0) {
            GTime currentTime = GTime::currentSteadyTime();
            delta = currentTime.secsDTo(mFrameTime);
        }
        mFrameTime.update();

        if (!mRunning || (update && !mWindow->update(delta))) {
//            mAppContext->postExitWindow(this);
            mExited = true;
        }
    }
}

void WindowHandle::destroy()
{
    mRunning = false;
    mWindow->onDestroy();
}

void WindowHandle::postExitEvent()
{
    mEventMana->postEvent(new WinExitEvent);
}

void WindowHandle::postWindowSizeEvent(uint32_t w, uint32_t h)
{
    mEventMana->postEvent(new WinSizeEvent(w, h));
}

void WindowHandle::postWindowPosEvent(int32_t x, int32_t y)
{
    mEventMana->postEvent(new WinPosEvent(x, y));
}

void WindowHandle::postDropEvent(const std::vector<std::string> &dropFiles)
{
    mEventMana->postEvent(new WinDropEvent(dropFiles));
}

void WindowHandle::postWindowFocusChange(bool focused)
{
    mEventMana->postEvent(new WinFocusChangeEvent(focused));
}

void WindowHandle::getCursorPosition(int32_t &x, int32_t &y)
{
    mNativeWindow->getCursorPosition(x, y);
}

/** set functions end **/

void WindowHandle::handleEvent(gxx::Event *event)
{
    switch (event->key()) {
        case WinEvents::Exit:
            mRunning = false;
            break;
        case WinEvents::WindowSize: {
            const auto *_e = dynamic_cast<const WinSizeEvent *>(event);
            if (!_e) {
                break;
            }
            mWidth = _e->width;
            mHeight = _e->height;

            mWindow->resetSize((int32_t)mWidth, (int32_t)mHeight);
        }
            break;
        case WinEvents::WindowPos: {
            const auto *_e = dynamic_cast<const WinPosEvent *>(event);
            if (!_e) {
                break;
            }
            mXPos = _e->x;
            mYPos = _e->y;
            mWindow->winMoveEvent(mXPos, mYPos);
        }
            break;
        case WinEvents::Drop: {
            const auto *_e = dynamic_cast<const WinDropEvent *>(event);
            if (!_e) {
                break;
            }
            mWindow->dropEvent(_e->dropFiles);
        }
            break;
        case WinEvents::FocusChange: {
            const auto *_e = dynamic_cast<const WinFocusChangeEvent *>(event);
            if (!_e) {
                break;
            }
            mFocused = _e->focused;
            mWindow->winFocusChangeEvent(mFocused);
        }
            break;
    }
}

void WindowHandle::nativeLoop()
{
    mWindow->nativeLoop();
}

void WindowHandle::nativeEndWait()
{
    mWindow->nativeEndWait();
}

/** ======== Window ======== **/

Window::Window(const std::string &title, WindowFlags flags)
{
    mWinContext = std::make_shared<WindowHandle>(this);
    mWinContext->setWindowTitle(title);
    mWinContext->setWindowFlags(flags);
}

Window::~Window()
{
    mKeyboard = nullptr;
    mMouse = nullptr;
    mWinContext = nullptr;
}

void Window::setTitle(const std::string &title)
{
    mWinContext->setWindowTitle(title);
}

std::string Window::title() const
{
    return mWinContext->mTitle;
}

void Window::setWindowState(WindowState::Enum state)
{
    mWinContext->setWindowState(state);
}

void Window::setWindowFlags(WindowFlags flags)
{
    mWinContext->setWindowFlags(flags);
}

void Window::setSize(uint32_t w, uint32_t h)
{
    mWinContext->setWindowSize(w, h);
}

void Window::setWidth(uint32_t w)
{
    mWinContext->setWindowSize(w, mWinContext->mHeight);
}

void Window::setHeight(uint32_t h)
{
    mWinContext->setWindowSize(mWinContext->mWidth, h);
}

WindowState::Enum Window::getWindowState() const
{
    return mWinContext->mWinState;
}

std::pair<uint32_t, uint32_t> Window::size() const
{
    return {mWinContext->mWidth, mWinContext->mHeight};
}

uint32_t Window::width() const
{
    return mWinContext->mWidth;
}

uint32_t Window::height() const
{
    return mWinContext->mHeight;
}

std::pair<int32_t, int32_t> Window::position() const
{
    return {mWinContext->mXPos, mWinContext->mYPos};
}

int32_t Window::x() const
{
    return mWinContext->mXPos;
}

int32_t Window::y() const
{
    return mWinContext->mYPos;
}

void Window::setWindowPos(int32_t x, int32_t y)
{
    mWinContext->setWindowPos(x, y);
}

void Window::close()
{
    mWinContext->close();
}

bool Window::windowFocused()
{
    return mWinContext->mFocused;
}

void Window::showInfoDialog(const std::string &title, const std::string &msg)
{
    mWinContext->showInfoDialog(title, msg);
}

void Window::setCursor(const Cursor &cursor)
{
    mWinContext->setCursor(cursor);
}

void Window::resetCursor()
{
    mWinContext->setCursor(Cursor());
}

void Window::setCursorMode(CursorMode::Enum mode)
{
    mWinContext->setCursorMode(mode);
}

CursorMode::Enum Window::getCursorMode() const
{
    return mWinContext->getCursorMode();
}

void Window::setCursorPosition(int32_t x, int32_t y)
{
    mWinContext->setCursorPosition(x, y);
}

void Window::getCursorPosition(int32_t &x, int32_t &y) const
{
    mWinContext->getCursorPosition(x, y);
}

/** virtual functions **/

void Window::init()
{
    mKeyboard = std::make_shared<Keyboard>(mWinContext->mWindowId);
    mKeyboard->setKeyPressEventCallback(
            [this](auto &&PH1, auto &&PH2) {
                keyPressEvent(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
            });
    mKeyboard->setKeyReleaseEventCallback(
            [this](auto &&PH1, auto &&PH2) {
                keyReleaseEvent(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
            });

    mMouse = std::make_shared<Mouse>(mWinContext->mWindowId);
    mMouse->setMouseMoveEventCallback(
            [this](auto &&PH1, auto &&PH2) {
                mouseMoveEvent(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
            });
    mMouse->setMousePressEventCallback(
            [this](auto &&PH1) {
                mousePressEvent(std::forward<decltype(PH1)>(PH1));
            });
    mMouse->setMouseReleaseEventCallback(
            [this](auto &&PH1) {
                mouseReleaseEvent(std::forward<decltype(PH1)>(PH1));
            });
    mMouse->setMouseScrollEventCallback(
            [this](auto &&PH1, auto &&PH2) {
                mouseScrollEvent(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2));
            });
}

bool Window::update(double delta)
{
    GX_UNUSED(delta);
    return true;
}

void Window::resetSize(int32_t w, int32_t h)
{

}

void Window::onDestroy()
{

}

void Window::nativeLoop()
{

}

void Window::nativeEndWait()
{

}

/** event functions **/

void Window::winMoveEvent(int32_t x, int32_t y)
{

}

void Window::winFocusChangeEvent(bool focused)
{

}

void Window::keyPressEvent(gxx::Key::Enum key, uint8_t modifier)
{

}

void Window::keyReleaseEvent(gxx::Key::Enum key, uint8_t modifier)
{

}

void Window::mouseMoveEvent(int32_t x, int32_t y)
{

}

void Window::mousePressEvent(gxx::MouseButton::Enum button)
{

}

void Window::mouseReleaseEvent(gxx::MouseButton::Enum button)
{

}

void Window::mouseScrollEvent(double xoffset, double yoffset)
{

}

void Window::dropEvent(const std::vector<std::string> &dropFiles)
{

}

WindowContext *Window::getWinContext()
{
    return mWinContext.get();
}

uint32_t Window::getWindowId() const
{
    return mWinContext->mWindowId;
}

/** GUIContext functions **/

Application *Window::getApplication() const
{
    return Application::application();
}

}