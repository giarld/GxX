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

#ifndef GXX_ENTRY_H
#define GXX_ENTRY_H

#include <gxx/eventhandler.h>
#include <gxx/eventsys.h>
#include <gxx/event.h>
#include <gxx/gui.h>
#include <gxx/device/basedevice.h>

#include <gxx/device/gamepad.h>
#include <gxx/device/keyboard.h>
#include <gxx/device/mouse.h>
#include <gxx/device/charinput.h>
#include <gxx/cursor.h>

#include <gx/gglobal.h>
#include <gx/gtime.h>
#include <gx/gtimer.h>

#include <vector>
#include <queue>
#include <string>


namespace gxx
{

class Application;

class Window;

class AppContext;

class NWindow;

/**
 * Interface for methods related to window oriented low-level windows+attribute encapsulation for upper level windows
 */
class GX_API WindowContext
{
public:
    struct PlatformData
    {
        PlatformData()
                : nativeDisplayType(nullptr),
                  nativeWindowHandle(nullptr),
                  context(nullptr),
                  backBuffer(nullptr),
                  backBufferDS(nullptr)
        {}

        void *nativeDisplayType;       //!< Native display type.
        void *nativeWindowHandle;      //!< Native window handle.
        void *context;                 //!< GL context, or D3D device.
        void *backBuffer;              //!< GL backbuffer, or D3D render target view.
        void *backBufferDS;            //!< Backbuffer depth/stencil.
    };

public:
    explicit WindowContext(Window *window);

    virtual ~WindowContext();

public:
    void setWindowTitle(const std::string &title);

    std::string getWindowTitle();

    void setWindowState(WindowState::Enum state);

    WindowState::Enum getWindowState() const;

    void setWindowFlags(WindowFlags flags);

    WindowFlags getWindowFlags() const;

    void setWindowPos(int32_t x, int32_t y);

    int32_t getX() const;

    int32_t getY() const;

    void setWindowSize(uint32_t width, uint32_t height);

    uint32_t getWindowWidth() const;

    uint32_t getWindowHeight() const;

    void close();

    uint32_t getWindowId() const;

    void setPlatformData(const PlatformData &pd);

    PlatformData *platformData();

    void showInfoDialog(const std::string &title, const std::string &msg);

    void setCursor(const Cursor &cursor);

    void setCursorMode(CursorMode::Enum mode);

    CursorMode::Enum getCursorMode() const;

    void setCursorPosition(int32_t x, int32_t y);

    virtual void getCursorPosition(int32_t &x, int32_t &y) = 0;

protected:
    friend class Window;

    friend class AppContext;

    friend class Application;

    Window *mWindow = nullptr;
    AppContext *mAppContext = nullptr;

    uint32_t mWindowId = 0;
    PlatformData mPlatformData;

    std::string mTitle;
    WindowState::Enum mWinState = WindowState::Normal;
    WindowFlags mWinFlags = WindowFlag::ShowBorder | WindowFlag::Resizable;
    int32_t mXPos = 0;
    int32_t mYPos = 0;
    uint32_t mWidth = 1280;
    uint32_t mHeight = 720;
    bool mFocused = true;

    CursorMode::Enum mCursorMode = CursorMode::Normal;
};

/**
 * The graphics window is a low-level processing interface that accepts low-level related events and dispatches them
 * to the generation window
 * Undertaking functions:
 * 1. Pass events to lower level windows
 * 2. Transferring events to lower level special devices
 */
class GX_API WindowHandle : public WindowContext,
                             public EventHandler
{
public:
    explicit WindowHandle(Window *window);

    ~WindowHandle() override;

public:

    /**
     * The initialization method of view layer synchronization architecture
     * Called by NWindow
     */
    void init();

    /**
     * A Single Loop Method for View Layer Synchronous Architecture
     */
    void frame(bool update = true);

    /**
     * End method of view layer synchronization architecture
     */
    void destroy();

public:
    void postExitEvent();

    void postWindowSizeEvent(uint32_t w, uint32_t h);

    void postWindowPosEvent(int32_t x, int32_t y);

    void postDropEvent(const std::vector<std::string> &dropFiles);

    void postWindowFocusChange(bool focused);

    void getCursorPosition(int32_t &x, int32_t &y) override;

public:
    void nativeLoop();

    void nativeEndWait();

protected:
    void handleEvent(Event *event) override;

private:
    friend class AppContext;

    NWindow *mNativeWindow = nullptr;

    EventMana *mEventMana = nullptr;

    gx::GTime mFrameTime;

    bool mRunning = false;
    bool mExited = false;
};

class ANBaseEvent;

class GX_API AppContext : public EventHandler,
                           public DeviceDriver,
                           public IGamepadDeviceDriver,
                           public IKeyboardDeviceDriver,
                           public IMouseDeviceDriver,
                           public ICharInputDriver
{
public:
    explicit AppContext();

    ~AppContext() override;

    int init(Application *app);

    int run(Application *app);

    void addWindow(Window *window);

    void getDesktopSize(uint32_t &w, uint32_t &h);

    void closeAll();

    void processEvents()
    {
        mEventMana->processEvents();
    }

    bool deviceSupport(DeviceType::Enum type) override;

    std::vector<GamepadStateInfo> getConnectedGamepadStateInfos() override;

protected:  // Platform related, down
    void handleEvent(Event *event) override
    {
        handleANEvent(reinterpret_cast<ANBaseEvent *>(event));
    }

    void handleANEvent(ANBaseEvent *event);

    /**
     * Trigger platform related event driven
     */
    void postNativeEvent();

protected:  // Platform independent, on top
    void postExitWindow(WindowContext *window);

    void postSetWindowSize(WindowContext *window, uint32_t w, uint32_t h);

    void postSetWindowPos(WindowContext *window, int32_t x, int32_t y);

    void postSetWindowTitle(WindowContext *window, const std::string &title);

    void postSetWindowState(WindowContext *window, WindowState::Enum state);

    void postSetWindowFlags(WindowContext *window, WindowFlags flags);

    void postShowInfoDialog(WindowContext *window, const std::string &title, const std::string &msg);

    void postSetCursor(WindowContext *window, const Cursor &cursor);

    void postSetCursorMode(WindowContext *window, CursorMode::Enum mode);

    void postSetCursorPos(WindowContext *window, int32_t x, int32_t y);

private:
    void destroyWindow(WindowHandle *wh);

    NWindow *createNWindow(WindowHandle *wh);

    inline bool nativeFrameW(WindowHandle *wh);

    void nativeDestroyW(WindowHandle *wh);

private:
    friend class WindowContext;

    friend class WindowHandle;

    EventMana *mEventMana;
    gx::GTimerSchedulerPtr mScheduler;

    std::vector<WindowHandle *> mWindows;

    using DelayedTask = std::function<void()>;
    std::queue<DelayedTask> mDelayedTasks;
};


/** WinEvents **/
struct WinEvents
{
    enum Enum
    {
        Exit,
        WindowSize,
        WindowPos,
        Drop,
        FocusChange,
    };
};

class WinExitEvent : public Event
{
public:
    explicit WinExitEvent() : Event(WinEvents::Exit)
    {}
};

class WinSizeEvent : public Event
{
public:
    explicit WinSizeEvent(uint32_t w, uint32_t h)
            : Event(WinEvents::WindowSize), width(w), height(h)
    {}

public:
    uint32_t width;
    uint32_t height;
};

class WinPosEvent : public Event
{
public:
    explicit WinPosEvent(int32_t x, int32_t y)
            : Event(WinEvents::WindowPos), x(x), y(y)
    {}

public:
    int32_t x;
    int32_t y;
};

class WinDropEvent : public Event
{
public:
    explicit WinDropEvent(std::vector<std::string> dropFiles)
            : Event(WinEvents::Drop), dropFiles(std::move(dropFiles))
    {}

public:
    std::vector<std::string> dropFiles;
};

class WinFocusChangeEvent : public Event
{
public:
    explicit WinFocusChangeEvent(bool focused)
            : Event(WinEvents::FocusChange), focused(focused)
    {}

public:
    bool focused;
};


/** AppNativeEvents **/
struct AppNativeEvents
{
    enum Enum
    {
        Exit,
        SetWindowSize,
        SetWindowPos,
        SetWindowTitle,
        SetWindowState,
        SetWindowFlags,
        ShowInfoDialog,
        SetCursor,
        SetCursorMode,
        SetCursorPos,
    };
};

class ANBaseEvent : public Event
{
public:
    explicit ANBaseEvent(WindowContext *window, int key) : Event(key), window(window)
    {}

public:
    WindowContext *window;
};

class ANWinExitEvent : public ANBaseEvent
{
public:
    explicit ANWinExitEvent(WindowContext *window)
            : ANBaseEvent(window, AppNativeEvents::Exit)
    {}
};

class ANSetWinSizeEvent : public ANBaseEvent
{
public:
    explicit ANSetWinSizeEvent(WindowContext *window, uint32_t w, uint32_t h)
            : ANBaseEvent(window, AppNativeEvents::SetWindowSize), width(w), height(h)
    {}

public:
    uint32_t width;
    uint32_t height;
};

class ANSetWinPosEvent : public ANBaseEvent
{
public:
    explicit ANSetWinPosEvent(WindowContext *window, int32_t x, int32_t y)
            : ANBaseEvent(window, AppNativeEvents::SetWindowPos), x(x), y(y)
    {}

public:
    int32_t x;
    int32_t y;
};

class ANSetWinTitleEvent : public ANBaseEvent
{
public:
    explicit ANSetWinTitleEvent(WindowContext *window, std::string title)
            : ANBaseEvent(window, AppNativeEvents::SetWindowTitle), title(std::move(title))
    {}

public:
    std::string title;
};

class ANSetWinStateEvent : public ANBaseEvent
{
public:
    explicit ANSetWinStateEvent(WindowContext *window, WindowState::Enum state)
            : ANBaseEvent(window, AppNativeEvents::SetWindowState), state(state)
    {}

public:
    WindowState::Enum state;
};

class ANSetWinFlagsEvent : public ANBaseEvent
{
public:
    explicit ANSetWinFlagsEvent(WindowContext *window, WindowFlags flags)
            : ANBaseEvent(window, AppNativeEvents::SetWindowFlags), flags(flags)
    {}

public:
    WindowFlags flags;
};

class ANShowInfoDialogEvent : public ANBaseEvent
{
public:
    explicit ANShowInfoDialogEvent(WindowContext *window, std::string title, std::string message)
            : ANBaseEvent(window, AppNativeEvents::ShowInfoDialog), title(std::move(title)), message(std::move(message))
    {}

public:
    std::string title;
    std::string message;
};

class ANSetCursorEvent : public ANBaseEvent
{
public:
    explicit ANSetCursorEvent(WindowContext *window, const Cursor &cursor)
            : ANBaseEvent(window, AppNativeEvents::SetCursor), cursor(cursor)
    {}

public:
    Cursor cursor;
};

class ANSetCursorModeEvent : public ANBaseEvent
{
public:
    explicit ANSetCursorModeEvent(WindowContext *window, CursorMode::Enum mode)
            : ANBaseEvent(window, AppNativeEvents::SetCursorMode), mode(mode)
    {}

public:
    CursorMode::Enum mode;
};

class ANSetCursorPosEvent : public ANBaseEvent
{
public:
    explicit ANSetCursorPosEvent(WindowContext *window, int32_t x, int32_t y)
            : ANBaseEvent(window, AppNativeEvents::SetCursorPos), x(x), y(y)
    {}

public:
    int32_t x;
    int32_t y;
};

}

#endif //GXX_ENTRY_H
