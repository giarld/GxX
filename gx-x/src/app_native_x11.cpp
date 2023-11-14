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

#include "gxx/app_native.h"

#if ENTRY_CONFIG_USE_NATIVE && (GX_PLATFORM_BSD || GX_PLATFORM_LINUX || GX_PLATFORM_RPI)

#include "gxx/app_entry.h"

#define XK_MISCELLANY
#define XK_LATIN1
#include <X11/keysymdef.h>
#include <X11/Xlib.h> // will include X11 which #defines None... Don't mess with order of includes.
#include <X11/Xutil.h>
#include <X11/Xcursor/Xcursor.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include <assert.h>

#include <gx/debug.h>

#ifdef None
#undef None
#endif
#define NoneN 0L

// Additional mouse button names for XButtonEvent
#define Button6            6
#define Button7            7

#define XCursorShape CursorShape
#undef CursorShape


using namespace gx;

namespace gxx
{

class NX11Window;

struct X11Global
{
    Display* display = nullptr;
    int32_t screen = 0;
    int32_t depth = 0;
    Visual* visual = nullptr;
    ::Window root = 0;
    XIM im = nullptr;

    NX11Window *focusWindow;

    Atom UTF8_STRING = 0;
    Atom NET_WM_NAME = 0;
    Atom NET_WMmIcON_NAME = 0;
    Atom WM_DELETE_WINDOW = 0;
};

static uint8_t sTranslateKey[512];

static void initTranslateKey(uint16_t xk, Key::Enum key);

static void initStatic();

static unsigned int decodeUTF8(const char** s);

static X11Global sX11App{};

static long EVENT_MASK = StructureNotifyMask | KeyPressMask | KeyReleaseMask |
                         PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
                         ExposureMask | FocusChangeMask | VisibilityChangeMask |
                         EnterWindowMask | LeaveWindowMask | PropertyChangeMask;


class NX11Window;

class NCursor
{
public:
    explicit NCursor() = default;

    explicit NCursor(const Cursor &cursor);

    ~NCursor();

public:
    void setToCursor(NX11Window *window);

    void updateCursorImage(NX11Window *window);

    static bool cursorInContentArea(NX11Window *window);

    static ::Cursor createCursor(const CursorBitmap &bitmap, int hotX, int hotY);

    static void destroyCursor(::Cursor cursor);

private:
    void createShapeCursor(Cursor::CursorShape shape);

    void createCustomCursor(const CursorBitmap &bitmap, int32_t hotX, int32_t hotY);

private:
    ::Cursor mCursor = 0;
};

class NX11Window : public NWindow
{
public:
    ~NX11Window() override
    = default;

    bool init(AppContext *appCtx, WindowHandle *wh) override
    {
        mAppContext = appCtx;
        mWh = wh;

        XSetWindowAttributes windowAttrs;
        memset(&windowAttrs, 0, sizeof(windowAttrs));
        windowAttrs.background_pixmap = 0;
        windowAttrs.border_pixel = 0;
        windowAttrs.event_mask = EVENT_MASK;
        mNativeWindow = XCreateWindow(sX11App.display
                , sX11App.root
                , wh->getX(), wh->getY()
                , wh->getWindowWidth(), wh->getWindowHeight(), 0
                , sX11App.depth
                , InputOutput
                , sX11App.visual
                , CWBorderPixel|CWEventMask
                , &windowAttrs
        );

        XSetWindowAttributes attr;
        memset(&attr, 0, sizeof(attr) );
        XChangeWindowAttributes(sX11App.display, mNativeWindow, CWBackPixel, &attr);


        XSetWMProtocols(sX11App.display, mNativeWindow, &sX11App.WM_DELETE_WINDOW, 1);

        const char *applicationName = "gxx";
        const char *applicationClass = "GXX";
        XMapWindow(sX11App.display, mNativeWindow);
        XStoreName(sX11App.display, mNativeWindow, applicationName);

        XClassHint* hint = XAllocClassHint();
        hint->res_name  = (char*)applicationName;
        hint->res_class = (char*)applicationClass;
        XSetClassHint(sX11App.display, mNativeWindow, hint);
        XFree(hint);

        if (sX11App.im) {
            mIc = XCreateIC(sX11App.im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, mNativeWindow,
                            XNFocusWindow, mNativeWindow, NULL
            );
        }

        x11SetDisplayWindow(mWh, sX11App.display, mNativeWindow);

        setWindowTitle(wh->getWindowTitle());
        setWindowPos(wh->getX(), wh->getY());
        setWindowSize(wh->getWindowWidth(), wh->getWindowHeight());

        setWindowFlags(wh->getWindowFlags());
        setWindowState(wh->getWindowState());

        CursorBitmap bitmap(16, 16);
        mHiddenCursor = NCursor::createCursor(bitmap, 0, 0);

        wh->init();
        return true;
    }

    bool isInited() override
    {
        return mNativeWindow;
    }

    bool frame() override
    {
        if (mExit) {
            return false;
        }

        Display *display = sX11App.display;

        XPending(display);
        XEvent event;
        while(XCheckWindowEvent(display, mNativeWindow, EVENT_MASK, &event)) {
            processEvent(&event);
        }

        while (XCheckTypedEvent(display, ClientMessage, &event)) {
            processEvent(&event);
        }

        if (mCursorMode == CursorMode::Disabled) {
            int centerX = (int) mWidth / 2;
            int centerY = (int) mHeight / 2;

            if (centerX != mLastMouseX || centerY != mLastMouseY) {
                mLastMouseX = centerX;
                mLastMouseY = centerY;

                XWarpPointer(sX11App.display, NoneN, mNativeWindow, 0, 0, 0, 0, centerX, centerY);
                XFlush(sX11App.display);
            }
        }


//        XFlush(display);

        return !mExit;
    }

    void destroy() override
    {
        if (mHiddenCursor) {
            NCursor::destroyCursor(mHiddenCursor);
            mHiddenCursor = 0;
        }
        if (mIc) {
            XDestroyIC(mIc);
            mIc = nullptr;
        }
        if (mNativeWindow) {
            XUnmapWindow(sX11App.display, mNativeWindow);
            XDestroyWindow(sX11App.display, mNativeWindow);
            mNativeWindow = 0;
        }
        XFlush(sX11App.display);
    }

public:
    void exit() override
    {
        mExit = true;
    }

    void setWindowSize(uint32_t w, uint32_t h) override
    {
        XResizeWindow(sX11App.display, mNativeWindow, mWidth = w, mHeight = h);
        XFlush(sX11App.display);
        mWh->postWindowSizeEvent(w, h);
    }

    void setWindowPos(int x, int y) override
    {
        // HACK: Explicitly setting PPosition to any value causes some WMs, notably
        //       Compiz and Metacity, to honor the position of unmapped windows
        if (!isWindowVisible())
        {
            long supplied;
            XSizeHints* hints = XAllocSizeHints();

            if (XGetWMNormalHints(sX11App.display, mNativeWindow, hints, &supplied))
            {
                hints->flags |= PPosition;
                hints->x = hints->y = 0;

                XSetWMNormalHints(sX11App.display, mNativeWindow, hints);
            }

            XFree(hints);
        }

        XMoveWindow(sX11App.display, mNativeWindow, mX = x, mY = y);
        XFlush(sX11App.display);
    }

    void setWindowTitle(const std::string &title) override
    {
#if defined(X_HAVE_UTF8_STRING)
        Xutf8SetWMProperties(sX11App.display,
                             mNativeWindow,
                             title.c_str(), title.c_str(),
                             NULL, 0,
                             NULL, NULL, NULL);
#else
        // This may be a slightly better fallback than using XStoreName and
    // XSetIconName, which always store their arguments using STRING
    XmbSetWMProperties(sX11App.display,
                            mNativeWindow,
                            title.c_str(), title.c_str(),
                            NULL, 0,
                            NULL, NULL, NULL);
#endif

        XChangeProperty(sX11App.display, mNativeWindow,
                        sX11App.NET_WM_NAME, sX11App.UTF8_STRING, 8,
                        PropModeReplace,
                        (unsigned char *) title.c_str(), strlen(title.c_str()));

        XChangeProperty(sX11App.display, mNativeWindow,
                        sX11App.NET_WMmIcON_NAME, sX11App.UTF8_STRING, 8,
                        PropModeReplace,
                        (unsigned char *) title.c_str(), strlen(title.c_str()));

        XFlush(sX11App.display);
    }

    void setWindowState(WindowState::Enum state) override
    {
        switch (state) {
            case WindowState::Normal: {
                maximizedWindow(false);
                fullScreen(false);
            }
                break;
            case WindowState::Minimized: {
                minimizedWindow(true);
            }
                break;
            case WindowState::Maximized: {
                fullScreen(false);
                maximizedWindow(true);
            }
                break;
            case WindowState::FullScreen: {
                maximizedWindow(false);
                fullScreen(true);
            }
                break;
        }
    }

    void setWindowFlags(WindowFlags flags) override
    {
        // TODO
        if (flags & WindowFlag::Resizable) {

        }
        if (flags & WindowFlag::ShowBorder) {

        }
    }

    void showInfoDialog(const std::string &title, const std::string &msg) override
    {
    }

    bool isWindowVisible()
    {
        XWindowAttributes wa;
        XGetWindowAttributes(sX11App.display, mNativeWindow, &wa);
        return wa.map_state == IsViewable;
    }

    void setCursor(const Cursor &cursor) override
    {
        destroyCursor();
        mCursor = new NCursor(cursor);
        mCursor->setToCursor(this);
    }

    void setCursorMode(CursorMode::Enum mode) override
    {
        if (mode == mCursorMode) {
            return;
        }
        getCursorPosition(mVirtualCursorPosX, mVirtualCursorPosY);

        CursorMode::Enum oldMode = mCursorMode;
        mCursorMode = mode;

        if (mCursor == nullptr) {
            mCursor = new NCursor();
        }
        mCursor->setToCursor(this);

        if (mode == CursorMode::Disabled) {
            getCursorPosition(mRestoreCursorPosX, mRestoreCursorPosY);

            // move cursor to window center
            XWarpPointer(sX11App.display, NoneN, mNativeWindow, 0, 0, 0, 0, mWidth / 2, mHeight / 2);
            XFlush(sX11App.display);

            XGrabPointer(sX11App.display, mNativeWindow, True,
                         ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                         GrabModeAsync, GrabModeAsync,
                         mNativeWindow,
                         mHiddenCursor,
                         CurrentTime);
        } else if (oldMode == CursorMode::Disabled) {
            XUngrabPointer(sX11App.display, CurrentTime);
            setCursorPosition(mRestoreCursorPosX, mRestoreCursorPosY);
        }
    }

    void setCursorPosition(int32_t x, int32_t y) override
    {
        if (mCursorMode == CursorMode::Disabled) {
            return;
        }
        mMouseX = x;
        mMouseY = y;

        XWarpPointer(sX11App.display, NoneN, mNativeWindow,
                     0,0,0,0, (int) x, (int) y);
        XFlush(sX11App.display);
    }

    void getCursorPosition(int32_t &x, int32_t &y) override
    {
        if (mCursorMode == CursorMode::Disabled) {
            x = mVirtualCursorPosX;
            y = mVirtualCursorPosY;
        } else {
            ::Window root, child;
            int rootX, rootY, childX, childY;
            unsigned int mask;

            XQueryPointer(sX11App.display, mNativeWindow,
                          &root, &child,
                          &rootX, &rootY, &childX, &childY,
                          &mask);

            x = childX;
            y = childY;
        }
    }

    void destroyCursor()
    {
        if (mCursor != nullptr) {
            XUndefineCursor(sX11App.display, mNativeWindow);
            delete mCursor;
            mCursor = nullptr;
        }
    }

private:
    void processEvent(XEvent *event)
    {
        bool filtered = false;
        if (sX11App.im) {
            filtered = XFilterEvent(event, NoneN);
        }

        switch (event->type)
        {
            case Expose:
                break;

            case ClientMessage:
                if (!filtered) {
//                    Log("ClientMessage");
                    const Atom protocol = event->xclient.data.l[0];
                    if (protocol != NoneN) {
                        if (protocol == sX11App.WM_DELETE_WINDOW && sX11App.focusWindow) {
                            sX11App.focusWindow->mWh->postExitEvent();
                            sX11App.focusWindow = nullptr;
                        }
                    }
                }
                break;

            case ButtonPress:
            case ButtonRelease:
            {
                const XButtonEvent& xbutton = event->xbutton;
                gxx::MouseButton::Enum mb = gxx::MouseButton::Enum::None;
                float mouseScrollX = 0;
                float mouseScrollY = 0;
                switch (xbutton.button)
                {
                    case Button1: mb = MouseButton::Left;   break;
                    case Button2: mb = MouseButton::Middle; break;
                    case Button3: mb = MouseButton::Right;  break;
                    case Button4: mouseScrollY = 1.0f; break;
                    case Button5: mouseScrollY = -1.0f; break;
                    case Button6: mouseScrollX = 1.0f; break;
                    case Button7: mouseScrollX = -1.0f; break;
                }

                if (MouseButton::None != mb)
                {
                    mAppContext->postMouseButtonEvent(mWh->getWindowId()
                            , mb
                            , event->type == ButtonPress ? KeyAction::Press : KeyAction::Release);
                }
                if (xbutton.x != mMouseX || xbutton.y != mMouseY) {
                    mMouseX = xbutton.x;
                    mMouseY = xbutton.y;

                    if (mCursorMode == CursorMode::Disabled) {
                        const int dx = mMouseX - mLastMouseX;
                        const int dy = mMouseY - mLastMouseY;

                        inputCursorPos(mVirtualCursorPosX + dx, mVirtualCursorPosY + dy);
                    } else {
                        inputCursorPos(mMouseX, mMouseY);
                    }

                    mLastMouseX = mMouseX;
                    mLastMouseY = mMouseY;
                }
                if (mouseScrollX != 0 || mouseScrollY != 0) {
                    mAppContext->postMouseScrollEvent(mWh->getWindowId(), mouseScrollX, mouseScrollY);
                }
            }
                break;

            case MotionNotify:
            {
                const XMotionEvent& xmotion = event->xmotion;

                mMouseX = xmotion.x;
                mMouseY = xmotion.y;

                if (mCursorMode == CursorMode::Disabled) {
                    const int dx = mMouseX - mLastMouseX;
                    const int dy = mMouseY - mLastMouseY;

                    inputCursorPos(mVirtualCursorPosX + dx, mVirtualCursorPosY + dy);
                } else {
                    inputCursorPos(mMouseX, mMouseY);
                }

                mLastMouseX = mMouseX;
                mLastMouseY = mMouseY;
            }
                break;

            case KeyPress:
            case KeyRelease:
            {
                XKeyEvent& xkey = event->xkey;
                KeySym keysym = XLookupKeysym(&xkey, 0);
                switch (keysym)
                {
                    case XK_Meta_L:    setModifier(Modifier::LeftMeta,   KeyPress == event->type); break;
                    case XK_Meta_R:    setModifier(Modifier::RightMeta,  KeyPress == event->type); break;
                    case XK_Control_L: setModifier(Modifier::LeftCtrl,   KeyPress == event->type); break;
                    case XK_Control_R: setModifier(Modifier::RightCtrl,  KeyPress == event->type); break;
                    case XK_Shift_L:   setModifier(Modifier::LeftShift,  KeyPress == event->type); break;
                    case XK_Shift_R:   setModifier(Modifier::RightShift, KeyPress == event->type); break;
                    case XK_Alt_L:     setModifier(Modifier::LeftAlt,    KeyPress == event->type); break;
                    case XK_Alt_R:     setModifier(Modifier::RightAlt,   KeyPress == event->type); break;

                    default:
                    {
                        Key::Enum key = fromXk(keysym);
                        if (KeyPress == event->type)
                        {
                            if (mLastKeyTime < event->xkey.time) {
                                if (Key::None != key) {
                                    mAppContext->postKeyEvent(mWh->getWindowId(), key, mModifiers,KeyAction::Press);
                                }
                                mLastKeyTime = event->xkey.time;
                            }

                            if (!filtered)
                            {
                                int count;
                                Status status;

                                char buffer[100];
                                char* chars = buffer;

                                count = Xutf8LookupString(mIc,
                                                          &event->xkey,
                                                          buffer, sizeof(buffer) - 1,
                                                          NULL, &status);

                                if (status == XBufferOverflow)
                                {
                                    chars = (char *) calloc(count + 1, 1);
                                    count = Xutf8LookupString(mIc,
                                                              &event->xkey,
                                                              chars, count,
                                                              NULL, &status);
                                }

                                if (status == XLookupChars || status == XLookupBoth)
                                {
                                    const char* c = chars;
                                    chars[count] = '\0';
                                    while (c - chars < count)
                                        mAppContext->postCharInputEvent(
                                                mWh->getWindowId(),
                                                GString::fromCodepoint(decodeUTF8(&c)).toStdString());
                                }
                                if (chars != buffer)
                                    free(chars);
                            }
                        }
                        else if (KeyRelease == event->type) {
                            if (Key::None != key) {
                                mAppContext->postKeyEvent(mWh->getWindowId(), key, mModifiers,KeyAction::Release);
                            }
                        }
                    }
                        break;
                }
            }
                break;

            case ConfigureNotify:
            {
                const XConfigureEvent& xev = event->xconfigure;
                if (mX != xev.x || mY != xev.y) {
                    mX = xev.x;
                    mY = xev.y;
                    mWh->postWindowPosEvent(mX, mY);
                }
                if (mWidth != xev.width || mHeight != xev.height) {
                    mWidth = xev.width;
                    mHeight = xev.height;
                    mWh->postWindowSizeEvent(mWidth, mHeight);
                }
            }
                break;
            case FocusIn:
            {
                if (event->xfocus.mode == NotifyGrab ||
                    event->xfocus.mode == NotifyUngrab) {
                    return;
                }
                Log("FocusIn %s", mWh->getWindowTitle().c_str());

                mWh->postWindowFocusChange(true);

                sX11App.focusWindow = this;

                if (mIc) {
                    XSetICFocus(mIc);
                }
            }
                break;
            case FocusOut:
            {
                if (event->xfocus.mode == NotifyGrab ||
                    event->xfocus.mode == NotifyUngrab) {
                    return;
                }
                Log("FocusOut %s", mWh->getWindowTitle().c_str());

                mWh->postWindowFocusChange(false);

                if (mIc) {
                    XUnsetICFocus(mIc);
                }
            }
                break;
        }
    }

    void inputCursorPos(int32_t x, int32_t y)
    {
        if (mVirtualCursorPosX == x && mVirtualCursorPosY == y)
            return;

        mVirtualCursorPosX = x;
        mVirtualCursorPosY = y;

        mAppContext->postMouseMoveEvent(mWh->getWindowId(), x, y);
    }

    void maximizedWindow(bool maximized)
    {
        Atom wmState = XInternAtom(sX11App.display, "_NET_WM_STATE", false);
        Atom wmVMaximizedState = XInternAtom(sX11App.display, "_NET_WM_STATE_MAXIMIZED_VERT", false);
        Atom wmHMaximizedState = XInternAtom(sX11App.display, "_NET_WM_STATE_MAXIMIZED_HORZ", false);

        XEvent xev;
        memset(&xev, 0, sizeof(xev));
        xev.xclient.type = ClientMessage;
        xev.xclient.display = sX11App.display;
        xev.xclient.window = mNativeWindow;
        xev.xclient.message_type = wmState;
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = maximized;
        xev.xclient.data.l[1] = wmVMaximizedState;
        xev.xclient.data.l[2] = wmHMaximizedState;
        xev.xclient.data.l[3] = 1;

        XSendEvent(sX11App.display,
                   sX11App.root,
                   false,
                   SubstructureRedirectMask | SubstructureNotifyMask,
                   &xev);
        XFlush(sX11App.display);
    }

    void minimizedWindow(bool minimized)
    {
        Atom wmState = XInternAtom(sX11App.display, "_NET_WM_STATE", false);
        Atom wmHiddenState = XInternAtom(sX11App.display, "_NET_WM_STATE_HIDDEN", false);

        XEvent xev;
        memset(&xev, 0, sizeof(xev));
        xev.xclient.type = ClientMessage;
        xev.xclient.display = sX11App.display;
        xev.xclient.window = mNativeWindow;
        xev.xclient.message_type = wmState;
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = minimized;
        xev.xclient.data.l[1] = wmHiddenState;
        xev.xclient.data.l[2] = 0;
        xev.xclient.data.l[3] = 1;

        XSendEvent(sX11App.display,
                   sX11App.root,
                   false,
                   SubstructureRedirectMask | SubstructureNotifyMask,
                   &xev
        );
        XIconifyWindow(sX11App.display, mNativeWindow, sX11App.screen);
        XFlush(sX11App.display);
    }

    void fullScreen(bool fullScreen)
    {
        Atom wmState = XInternAtom(sX11App.display, "_NET_WM_STATE", false);
        Atom wmfullscreenState = XInternAtom(sX11App.display, "_NET_WM_STATE_FULLSCREEN", false);

        XEvent xev;
        memset(&xev, 0, sizeof(xev));
        xev.xclient.type = ClientMessage;
        xev.xclient.display = sX11App.display;
        xev.xclient.window = mNativeWindow;
        xev.xclient.message_type = wmState;
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = fullScreen;
        xev.xclient.data.l[1] = wmfullscreenState;
        xev.xclient.data.l[2] = 0;
        xev.xclient.data.l[3] = 1;

        XSendEvent(sX11App.display,
                   sX11App.root,
                   false,
                   SubstructureRedirectMask | SubstructureNotifyMask,
                   &xev
        );
        XFlush(sX11App.display);
    }

private:
    static inline void x11SetDisplayWindow(WindowHandle *wh, void* display, uint32_t window, void* glx = nullptr)
    {
        WindowHandle::PlatformData pd;
        pd.nativeDisplayType = display;
        pd.nativeWindowHandle = (void*)(uintptr_t)window;
        pd.context = glx;
        wh->setPlatformData(pd);
    }

    void setModifier(Modifier::Enum modifier, bool set)
    {
        mModifiers &= ~modifier;
        mModifiers |= set ? modifier : 0;
    }

    static Key::Enum fromXk(uint16_t xk)
    {
        xk += 256;
        return 512 > xk ? (Key::Enum)sTranslateKey[xk] : Key::None;
    }

private:
    friend class NCursor;

    AppContext *mAppContext = nullptr;
    WindowHandle *mWh = nullptr;

    ::Window mNativeWindow = 0;
    XIC mIc = nullptr;

    uint8_t mModifiers = 0;
    int mMouseX = 0;
    int mMouseY = 0;
    ::Time mLastKeyTime = 0;

    int mX = 0;
    int mY = 0;
    int mWidth = 0;
    int mHeight = 0;

    bool mExit = false;

    CursorMode::Enum mCursorMode = CursorMode::Normal;
    NCursor *mCursor = nullptr;
    int32_t mLastMouseX = 0, mLastMouseY = 0;
    int32_t mVirtualCursorPosX = 0, mVirtualCursorPosY = 0;
    int32_t mRestoreCursorPosX = 0, mRestoreCursorPosY = 0;

    ::Cursor mHiddenCursor = 0;
};


NCursor::NCursor(const Cursor &cursor)
{
    switch (cursor.getCursorStyle()) {
        case Cursor::System: {
            createShapeCursor(cursor.getCursorShape());
        }
            break;
        case Cursor::Custom: {
            createCustomCursor(cursor.getBitmap(), cursor.getHotX(), cursor.getHotY());
        }
            break;
    }
}

NCursor::~NCursor()
{
    destroyCursor(mCursor);
}

void NCursor::setToCursor(NX11Window *window)
{
    if (cursorInContentArea(window)) {
        updateCursorImage(window);
    }
}

void NCursor::updateCursorImage(NX11Window *window)
{
    if (window->mCursorMode == CursorMode::Normal) {
        if (mCursor) {
            XDefineCursor(sX11App.display, window->mNativeWindow, mCursor);
        } else
            XUndefineCursor(sX11App.display, window->mNativeWindow);
    } else {
        XDefineCursor(sX11App.display, window->mNativeWindow, window->mHiddenCursor);
    }
    XFlush(sX11App.display);
}

bool NCursor::cursorInContentArea(NX11Window *window)
{
    return true;
}

void NCursor::createShapeCursor(Cursor::CursorShape shape)
{
    int id;
    switch (shape) {
        default:
        case Cursor::Arrow:
            id = XC_left_ptr;
            break;
        case Cursor::IBeam:
            id = XC_xterm;
            break;
        case Cursor::Cross:
            id = XC_crosshair;
            break;
        case Cursor::Hand:
            id = XC_hand2;
            break;
        case Cursor::SizeVer:
            id = XC_sb_v_double_arrow;
            break;
        case Cursor::SizeHor:
            id = XC_sb_h_double_arrow;
            break;
    }

    mCursor = XCreateFontCursor(sX11App.display, id);
}

void NCursor::createCustomCursor(const CursorBitmap &bitmap, int32_t hotX, int32_t hotY)
{
    mCursor = createCursor(bitmap, hotX, hotY);
}

::Cursor NCursor::createCursor(const CursorBitmap &bitmap, int hotX, int hotY)
{
    ::Cursor cursor;

    XcursorImage* native = XcursorImageCreate(bitmap.width(), bitmap.height());
    if (native == nullptr)
        return NoneN;

    native->xhot = hotX;
    native->yhot = hotY;

    const unsigned char* source = bitmap.data();
    XcursorPixel* target = native->pixels;

    for (int i = 0;  i < bitmap.width() * bitmap.height();  i++, target++, source += 4)
    {
        unsigned int alpha = source[3];

        *target = (alpha << 24) |
                  ((unsigned char) ((source[0] * alpha) / 255) << 16) |
                  ((unsigned char) ((source[1] * alpha) / 255) <<  8) |
                  ((unsigned char) ((source[2] * alpha) / 255) <<  0);
    }

    cursor = XcursorImageLoadCursor(sX11App.display, native);
    XcursorImageDestroy(native);

    return cursor;
}

void NCursor::destroyCursor(::Cursor cursor)
{
    if (cursor) {
        XFreeCursor(sX11App.display, cursor);
    }
}


NWindow *createNativeWindow()
{
    return new NX11Window();
}

int nativeInit(AppContext *appCtx)
{
    XInitThreads();
    XrmInitialize();

    sX11App.display = XOpenDisplay(NULL);
    if (!sX11App.display) {
        return EXIT_FAILURE;
    }
    sX11App.screen = DefaultScreen(sX11App.display);
    sX11App.depth  = DefaultDepth(sX11App.display, sX11App.screen);
    sX11App.visual = DefaultVisual(sX11App.display, sX11App.screen);
    sX11App.root   = RootWindow(sX11App.display, sX11App.screen);

    sX11App.im = XOpenIM(sX11App.display, 0, NULL, NULL);

    sX11App.UTF8_STRING = XInternAtom(sX11App.display, "UTF8_STRING", False);

    sX11App.NET_WM_NAME = XInternAtom(sX11App.display, "_NET_WM_NAME", False);
    sX11App.NET_WMmIcON_NAME = XInternAtom(sX11App.display, "_NET_WMmIcON_NAME", False);
    sX11App.WM_DELETE_WINDOW = XInternAtom(sX11App.display, "WM_DELETE_WINDOW", False);

    initStatic();
    return 0;
}

int nativeTerminate(AppContext *appCtx)
{
    if (sX11App.display) {
        XCloseDisplay(sX11App.display);
    }
    return EXIT_SUCCESS;
}

bool nativeDeviceSupport(DeviceType::Enum type)
{
    switch (type) {
        case DeviceType::Keyboard:
        case DeviceType::Mouse:
        case DeviceType::GamePad:
        case DeviceType::CharInput:
            return true;
        default:
            return false;
    }
}

static void initTranslateKey(uint16_t xk, Key::Enum key)
{
    xk += 256;
    assert(xk < ARRAY_LEN(sTranslateKey));
    sTranslateKey[xk&0x1ff] = (uint8_t)key;
}

std::vector<GamepadStateInfo> nativeGetConnectedGamepadStateInfos()
{
    return {};
}

void nativeGetDesktopSize(uint32_t &w, uint32_t &h)
{
    Screen *s = DefaultScreenOfDisplay(sX11App.display);

    w = s->width;
    h = s->height;
}


static void initStatic()
{
    memset(sTranslateKey, 0, sizeof(sTranslateKey));
    initTranslateKey(XK_Escape,       Key::Esc);
    initTranslateKey(XK_Return,       Key::Return);
    initTranslateKey(XK_Tab,          Key::Tab);
    initTranslateKey(XK_BackSpace,    Key::Backspace);
    initTranslateKey(XK_space,        Key::Space);
    initTranslateKey(XK_Up,           Key::Up);
    initTranslateKey(XK_Down,         Key::Down);
    initTranslateKey(XK_Left,         Key::Left);
    initTranslateKey(XK_Right,        Key::Right);
    initTranslateKey(XK_Insert,       Key::Insert);
    initTranslateKey(XK_Delete,       Key::Delete);
    initTranslateKey(XK_Home,         Key::Home);
    initTranslateKey(XK_KP_End,       Key::End);
    initTranslateKey(XK_Page_Up,      Key::PageUp);
    initTranslateKey(XK_Page_Down,    Key::PageDown);
    initTranslateKey(XK_Print,        Key::Print);
    initTranslateKey(XK_equal,        Key::Plus);
    initTranslateKey(XK_minus,        Key::Minus);
    initTranslateKey(XK_bracketleft,  Key::LeftBracket);
    initTranslateKey(XK_bracketright, Key::RightBracket);
    initTranslateKey(XK_semicolon,    Key::Semicolon);
    initTranslateKey(XK_apostrophe,   Key::Quote);
    initTranslateKey(XK_comma,        Key::Comma);
    initTranslateKey(XK_period,       Key::Period);
    initTranslateKey(XK_slash,        Key::Slash);
    initTranslateKey(XK_backslash,    Key::Backslash);
    initTranslateKey(XK_grave,        Key::Tilde);
    initTranslateKey(XK_F1,           Key::F1);
    initTranslateKey(XK_F2,           Key::F2);
    initTranslateKey(XK_F3,           Key::F3);
    initTranslateKey(XK_F4,           Key::F4);
    initTranslateKey(XK_F5,           Key::F5);
    initTranslateKey(XK_F6,           Key::F6);
    initTranslateKey(XK_F7,           Key::F7);
    initTranslateKey(XK_F8,           Key::F8);
    initTranslateKey(XK_F9,           Key::F9);
    initTranslateKey(XK_F10,          Key::F10);
    initTranslateKey(XK_F11,          Key::F11);
    initTranslateKey(XK_F12,          Key::F12);
    initTranslateKey(XK_KP_Insert,    Key::NumPad0);
    initTranslateKey(XK_KP_End,       Key::NumPad1);
    initTranslateKey(XK_KP_Down,      Key::NumPad2);
    initTranslateKey(XK_KP_Page_Down, Key::NumPad3);
    initTranslateKey(XK_KP_Left,      Key::NumPad4);
    initTranslateKey(XK_KP_Begin,     Key::NumPad5);
    initTranslateKey(XK_KP_Right,     Key::NumPad6);
    initTranslateKey(XK_KP_Home,      Key::NumPad7);
    initTranslateKey(XK_KP_Up,        Key::NumPad8);
    initTranslateKey(XK_KP_Page_Up,   Key::NumPad9);
    initTranslateKey('0',             Key::Key0);
    initTranslateKey('1',             Key::Key1);
    initTranslateKey('2',             Key::Key2);
    initTranslateKey('3',             Key::Key3);
    initTranslateKey('4',             Key::Key4);
    initTranslateKey('5',             Key::Key5);
    initTranslateKey('6',             Key::Key6);
    initTranslateKey('7',             Key::Key7);
    initTranslateKey('8',             Key::Key8);
    initTranslateKey('9',             Key::Key9);
    initTranslateKey('a',             Key::KeyA);
    initTranslateKey('b',             Key::KeyB);
    initTranslateKey('c',             Key::KeyC);
    initTranslateKey('d',             Key::KeyD);
    initTranslateKey('e',             Key::KeyE);
    initTranslateKey('f',             Key::KeyF);
    initTranslateKey('g',             Key::KeyG);
    initTranslateKey('h',             Key::KeyH);
    initTranslateKey('i',             Key::KeyI);
    initTranslateKey('j',             Key::KeyJ);
    initTranslateKey('k',             Key::KeyK);
    initTranslateKey('l',             Key::KeyL);
    initTranslateKey('m',             Key::KeyM);
    initTranslateKey('n',             Key::KeyN);
    initTranslateKey('o',             Key::KeyO);
    initTranslateKey('p',             Key::KeyP);
    initTranslateKey('q',             Key::KeyQ);
    initTranslateKey('r',             Key::KeyR);
    initTranslateKey('s',             Key::KeyS);
    initTranslateKey('t',             Key::KeyT);
    initTranslateKey('u',             Key::KeyU);
    initTranslateKey('v',             Key::KeyV);
    initTranslateKey('w',             Key::KeyW);
    initTranslateKey('x',             Key::KeyX);
    initTranslateKey('y',             Key::KeyY);
    initTranslateKey('z',             Key::KeyZ);
}

static unsigned int decodeUTF8(const char** s)
{
    unsigned int ch = 0, count = 0;
    static const unsigned int offsets[] =
            {
                    0x00000000u, 0x00003080u, 0x000e2080u,
                    0x03c82080u, 0xfa082080u, 0x82082080u
            };

    do
    {
        ch = (ch << 6) + (unsigned char) **s;
        (*s)++;
        count++;
    } while ((**s & 0xc0) == 0x80);

    assert(count <= 6);
    return ch - offsets[count - 1];
}

}

#endif