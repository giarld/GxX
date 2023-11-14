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

#ifndef GXX_NATIVE_APP_H
#define GXX_NATIVE_APP_H

#include <gxx/device/gamepad.h>

#include <gxx/device/device_type.h>
#include <gxx/event.h>

#define ENTRY_CONFIG_USE_NATIVE 1


namespace gxx
{

class WindowHandle;

class AppContext;

class Cursor;

class NWindow
{
public:
    explicit NWindow()
    {}

    virtual ~NWindow()
    {}

public:
    virtual bool init(AppContext *appCtx, WindowHandle *wh) = 0;

    virtual bool isInited() = 0;

    /**
     * native window Single frame
     * @return 0 no error; -1 exit; -2 not initialized
     */
    virtual bool frame() = 0;

    virtual void destroy() = 0;

public:
    virtual void exit() = 0;

    virtual void setWindowSize(uint32_t w, uint32_t h) = 0;

    virtual void setWindowPos(int32_t x, int32_t y) = 0;

    virtual void setWindowTitle(const std::string &title) = 0;

    virtual void setWindowState(WindowState::Enum state) = 0;

    virtual void setWindowFlags(WindowFlags flags) = 0;

    virtual void showInfoDialog(const std::string &title, const std::string &msg) = 0;

    virtual void setCursor(const Cursor &cursor) = 0;

    virtual void setCursorMode(CursorMode::Enum mode) = 0;

    virtual void setCursorPosition(int32_t x, int32_t y) = 0;

    virtual void getCursorPosition(int32_t &x, int32_t &y) = 0;
};

extern NWindow *createNativeWindow();

extern int nativeInit(AppContext *appCtx);

extern int nativeTerminate(AppContext *appCtx);

extern bool nativeDeviceSupport(DeviceType::Enum type);

extern std::vector<GamepadStateInfo> nativeGetConnectedGamepadStateInfos();

extern void nativeGetDesktopSize(uint32_t &w, uint32_t &h);

}

#endif //GXX_NATIVE_APP_H
