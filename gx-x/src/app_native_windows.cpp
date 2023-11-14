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

#include <gxx/app_native.h>

#if ENTRY_CONFIG_USE_NATIVE && GX_PLATFORM_WINDOWS

#include <gxx/app_entry.h>

#define OEMRESOURCE

#include <windows.h>
#include <windowsx.h>

#include <xinput.h>

#include <gx/gglobal.h>
#include <gx/debug.h>


using namespace gx;

namespace gxx
{

class NWndWindow;

class NGamepadMgr;

typedef std::vector<WCHAR> WSTRING;

struct TranslateKeyModifiers
{
    uint8_t vk;
    Modifier::Enum modifier;
};

static const TranslateKeyModifiers sTranslateKeyModifiers[8] =
        {
                {VK_LMENU,    Modifier::LeftAlt},
                {VK_RMENU,    Modifier::RightAlt},
                {VK_LCONTROL, Modifier::LeftCtrl},
                {VK_RCONTROL, Modifier::RightCtrl},
                {VK_LSHIFT,   Modifier::LeftShift},
                {VK_RSHIFT,   Modifier::RightShift},
                {VK_LWIN,     Modifier::LeftMeta},
                {VK_RWIN,     Modifier::RightMeta},
        };

static Key::Enum sTranslateKey[256];

static NGamepadMgr *sGamepadMgr = nullptr;

static void initStatic();

typedef DWORD (WINAPI *PFN_XInputGetCapabilities)(DWORD, DWORD, XINPUT_CAPABILITIES *);

typedef DWORD (WINAPI *PFN_XInputGetState)(DWORD, XINPUT_STATE *);

// HACK: Define macros that some xinput.h variants don't
#ifndef XINPUT_CAPS_WIRELESS
#define XINPUT_CAPS_WIRELESS 0x0002
#endif
#ifndef XINPUT_DEVSUBTYPE_WHEEL
#define XINPUT_DEVSUBTYPE_WHEEL 0x02
#endif
#ifndef XINPUT_DEVSUBTYPE_ARCADE_STICK
#define XINPUT_DEVSUBTYPE_ARCADE_STICK 0x03
#endif
#ifndef XINPUT_DEVSUBTYPE_FLIGHT_STICK
#define XINPUT_DEVSUBTYPE_FLIGHT_STICK 0x04
#endif
#ifndef XINPUT_DEVSUBTYPE_DANCE_PAD
#define XINPUT_DEVSUBTYPE_DANCE_PAD 0x05
#endif
#ifndef XINPUT_DEVSUBTYPE_GUITAR
#define XINPUT_DEVSUBTYPE_GUITAR 0x06
#endif
#ifndef XINPUT_DEVSUBTYPE_DRUM_KIT
#define XINPUT_DEVSUBTYPE_DRUM_KIT 0x08
#endif
#ifndef XINPUT_DEVSUBTYPE_ARCADE_PAD
#define XINPUT_DEVSUBTYPE_ARCADE_PAD 0x13
#endif
#ifndef XUSER_MAX_COUNT
#define XUSER_MAX_COUNT 4
#endif
#ifndef XINPUT_GAMEPAD_GUIDE
#	define XINPUT_GAMEPAD_GUIDE 0x400
#endif // XINPUT_GAMEPAD_GUIDE

struct XInputRemap
{
    uint16_t bit;
    GamepadButton::Enum key;
};

static constexpr const XInputRemap sXinputRemap[] =
        {
                {XINPUT_GAMEPAD_DPAD_UP,        GamepadButton::GamepadUp},
                {XINPUT_GAMEPAD_DPAD_DOWN,      GamepadButton::GamepadDown},
                {XINPUT_GAMEPAD_DPAD_LEFT,      GamepadButton::GamepadLeft},
                {XINPUT_GAMEPAD_DPAD_RIGHT,     GamepadButton::GamepadRight},
                {XINPUT_GAMEPAD_START,          GamepadButton::GamepadStart},
                {XINPUT_GAMEPAD_BACK,           GamepadButton::GamepadBack},
                {XINPUT_GAMEPAD_LEFT_THUMB,     GamepadButton::GamepadLeftThumb},
                {XINPUT_GAMEPAD_RIGHT_THUMB,    GamepadButton::GamepadRightThumb},
                {XINPUT_GAMEPAD_LEFT_SHOULDER,  GamepadButton::GamepadLeftBumper},
                {XINPUT_GAMEPAD_RIGHT_SHOULDER, GamepadButton::GamepadRightBumper},
                {XINPUT_GAMEPAD_GUIDE,          GamepadButton::GamepadGuide},
                {XINPUT_GAMEPAD_A,              GamepadButton::GamepadA},
                {XINPUT_GAMEPAD_B,              GamepadButton::GamepadB},
                {XINPUT_GAMEPAD_X,              GamepadButton::GamepadX},
                {XINPUT_GAMEPAD_Y,              GamepadButton::GamepadY},
        };


class NGamepadMgr
{
public:
    constexpr const static uint32_t MAX_GAMEPAD_COUNT = XUSER_MAX_COUNT;

public:
    void init(IGamepadDeviceDriver *gamepadDD);

    void destroy();

public:
    void update();

    std::vector<GamepadStateInfo> getConnectedGamepadStateInfos();

private:
    static std::string getDeviceDescription(const XINPUT_CAPABILITIES *xic);

private:
    struct
    {
        HINSTANCE instance;
        PFN_XInputGetCapabilities GetCapabilities;
        PFN_XInputGetState GetState;
    } mXinput{};

    bool mConnected[MAX_GAMEPAD_COUNT]{false};
    GamepadInfo mGamepadInfos[MAX_GAMEPAD_COUNT]{};
    XINPUT_STATE mXStates[MAX_GAMEPAD_COUNT]{};

    IGamepadDeviceDriver *mGamepadDD = nullptr;
};


class NCursor
{
public:
    explicit NCursor() = default;

    explicit NCursor(const Cursor &cursor);

    ~NCursor();

public:
    void setToCursor(NWndWindow *window);

    void updateCursorImage(NWndWindow *window);

    static bool cursorInContentArea(NWndWindow *window);

private:
    void createShapeCursor(Cursor::CursorShape shape);

    void createCustomCursor(const CursorBitmap &bitmap, int32_t hotX, int32_t hotY);

    static HICON createIcon(const CursorBitmap &bitmap, int hotX, int hotY);

private:
    HCURSOR mCursor = nullptr;
};


class NWndWindow : public NWindow
{
public:
    ~NWndWindow() override
    = default;

    bool init(AppContext *appCtx, WindowHandle *wh) override
    {
        mAppContext = appCtx;
        mWh = wh;

        HINSTANCE instance = (HINSTANCE) GetModuleHandle(NULL);

        WNDCLASSEXW wnd;
        memset(&wnd, 0, sizeof(wnd));
        wnd.cbSize = sizeof(wnd);
        wnd.style = CS_HREDRAW | CS_VREDRAW;
        wnd.lpfnWndProc = wndProc;
        wnd.hInstance = instance;
        wnd.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
        wnd.lpszClassName = L"gxx";
        wnd.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
        RegisterClassExW(&wnd);

        mNativeWindow = CreateWindowExW(
                WS_EX_ACCEPTFILES, L"gxx", UTF8ToUTF16(wh->getWindowTitle().c_str()).data(),
                WS_OVERLAPPEDWINDOW | WS_VISIBLE, wh->getX(), wh->getY(), wh->getWindowWidth(),
                wh->getWindowHeight(), NULL, NULL, instance, 0
        );

        winSetHwnd(mNativeWindow);

        SetWindowLongPtr(mNativeWindow, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

        ShowWindow(mNativeWindow, SW_RESTORE);
        setWindowSize(wh->getWindowWidth(), wh->getWindowHeight());
        setWindowPos(wh->getX(), wh->getY());
        clear(mNativeWindow);

        setWindowFlags(wh->getWindowFlags());
        setWindowState(wh->getWindowState());

        // Gamepad is a globally shared singleton initialized by the first window
        if (sGamepadMgr == nullptr) {
            sGamepadMgr = new NGamepadMgr;
            sGamepadMgr->init(mAppContext);
        }

        wh->init();

        mMsg.message = WM_NULL;

        return true;
    }

    bool isInited() override
    {
        return mNativeWindow != nullptr;
    }

    bool frame() override
    {
        if (sGamepadMgr) {
            sGamepadMgr->update();
        }

        WaitForInputIdle(GetCurrentProcess(), 16);

        while (0 != PeekMessageW(&mMsg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&mMsg);
            DispatchMessageW(&mMsg);
        }

        if (mCursorMode == CursorMode::Disabled) {
            int centerX = (int) mWh->getWindowWidth() / 2;
            int centerY = (int) mWh->getWindowHeight() / 2;

            if (centerX != mLastMouseX || centerY != mLastMouseY) {
                mLastMouseX = centerX;
                mLastMouseY = centerY;
                POINT pos = {centerX, centerY};
                ClientToScreen(mNativeWindow, &pos);
                SetCursorPos(pos.x, pos.y);
            }
        }

        return !mExit;
    }

    void destroy() override
    {
        destroyCursor();
        DestroyWindow(mNativeWindow);
        mNativeWindow = nullptr;
    }

public:
    void exit() override
    {
        this->mExit = true;
    }

    void setWindowSize(uint32_t w, uint32_t h) override
    {
        RECT rect;
        RECT newrect = {0, 0, (LONG) w, (LONG) h};
        DWORD style = GetWindowLongA(mNativeWindow, GWL_STYLE);

        GetWindowRect(mNativeWindow, &rect);
        AdjustWindowRect(&newrect, style, FALSE);

        int32_t left = rect.left;
        int32_t top = rect.top;
        int32_t nw = (newrect.right - newrect.left);
        int32_t nh = (newrect.bottom - newrect.top);

        SetWindowPos(mNativeWindow, HWND_TOP, left, top, nw, nh, SWP_SHOWWINDOW);

        if (mWidth != w || mHeight != h) {
            mWh->postWindowSizeEvent(mWidth = w, mHeight = h);
        }
    }

    void setWindowPos(int32_t x, int32_t y) override
    {
        mWinPosX = x;
        mWinPosY = y;

        SetWindowPos(mNativeWindow, nullptr, x, y, 0, 0,
                     SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOSIZE);
    }

    void setWindowTitle(const std::string &title) override
    {
        SetWindowTextW(mNativeWindow, UTF8ToUTF16(title.c_str()).data());
    }

    void setWindowState(WindowState::Enum state) override
    {
        WindowState::Enum oldState = mWinState;
        mWinState = state;

        if (state == WindowState::Normal) {
            if (oldState == WindowState::FullScreen) {
                exitFullScreen();
            }
            ShowWindow(mNativeWindow, SW_RESTORE);
        } else if (state == WindowState::Minimized) {
            ShowWindow(mNativeWindow, SW_MINIMIZE);
        } else if (state == WindowState::Maximized) {
            if (oldState == WindowState::FullScreen) {
                exitFullScreen();
            }
            ShowWindow(mNativeWindow, SW_MAXIMIZE);
        } else if (state == WindowState::FullScreen) {
            int32_t dw, dh;
            getDesktopSize(dw, dh);

            mOriWidth = mWidth;
            mOriHeight = mHeight;
            mOriWinPosX = mWinPosX;
            mOriWinPosY = mWinPosY;

            SetWindowLongA(mNativeWindow, GWL_STYLE, WS_POPUP);
            setWindowSize(dw, dh);
            setWindowPos(0, 0);
        }
    }

    void setWindowFlags(WindowFlags flags) override
    {

        mNativeFlags = WS_VISIBLE |
                       WS_OVERLAPPED;

        if (flags & WindowFlag::Resizable) {
            mNativeFlags |= WS_THICKFRAME;
        }
        if (flags & WindowFlag::ShowBorder) {
            mNativeFlags = mNativeFlags |
                    WS_CAPTION |
                    WS_SYSMENU |
                    WS_MINIMIZEBOX;
            if (flags & WindowFlag::Resizable) {
                mNativeFlags |= WS_MAXIMIZEBOX;
            }
        }

        int32_t ow = mWidth;
        int32_t oh = mHeight;

        int32_t opx = mWinPosX;
        int32_t opy = mWinPosY;

        SetWindowLongA(mNativeWindow, GWL_STYLE, mNativeFlags);

        setWindowSize(ow, oh);
    }

    void showInfoDialog(const std::string &title, const std::string &msg) override
    {
        MessageBoxW(mNativeWindow, UTF8ToUTF16(msg.c_str()).data(), UTF8ToUTF16(title.c_str()).data(),
                    MB_OK | MB_ICONINFORMATION);
    }

    bool windowFocused()
    {
        return mNativeWindow == GetActiveWindow();
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

        // Disabled mode, lock the mouse to the center of the screen and hide it.
        // Each time the mouse offset is obtained and accumulated into a virtual mouse coordinate
        // When disabling mode ends, restore the mouse pointer to its previous position.
        if (mode == CursorMode::Disabled) {
            getCursorPosition(mRestoreCursorPosX, mRestoreCursorPosY);
        } else if (oldMode == CursorMode::Disabled) {
            setCursorPosition(mRestoreCursorPosX, mRestoreCursorPosY);
        }

        if (mCursor == nullptr) {
            mCursor = new NCursor();
        }
        mCursor->setToCursor(this);
    }

    void setCursorPosition(int32_t x, int32_t y) override
    {
        if (mCursorMode == CursorMode::Disabled) {
            return;
        }
        if (x < 0 || x >= mWh->getWindowWidth() ||
            y < 0 || y >= mWh->getWindowHeight()) {
            return;
        }

        if (!windowFocused())
            return;

        POINT pos = {x, y};

        mLastMouseX = pos.x;
        mLastMouseY = pos.y;

        ClientToScreen(mNativeWindow, &pos);
        SetCursorPos(pos.x, pos.y);
    }

    void getCursorPosition(int32_t &x, int32_t &y) override
    {
        if (mCursorMode == CursorMode::Disabled) {
            x = mVirtualCursorPosX;
            y = mVirtualCursorPosY;
        } else {
            POINT pos;
            if (GetCursorPos(&pos)) {
                ScreenToClient(mNativeWindow, &pos);
                x = pos.x;
                y = pos.y;
            }
        }
    }

private:
    inline void winSetHwnd(::HWND window)
    {
        WindowContext::PlatformData pd;
        memset(&pd, 0, sizeof(pd));
        pd.nativeWindowHandle = window;
        mWh->setPlatformData(pd);
    }

    inline void clear(HWND hwnd)
    {
        RECT rect;
        GetWindowRect(hwnd, &rect);
        HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
        HDC hdc = GetDC(hwnd);
        SelectObject(hdc, brush);
        FillRect(hdc, &rect, brush);
        ReleaseDC(hwnd, hdc);
    }

    LRESULT process(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam)
    {
        switch (id) {
            case WM_DESTROY:
                break;

            case WM_QUIT:
            case WM_CLOSE:
                this->mExit = true;
                // Don't process message. Window will be destroyed later.
                return 0;
            case WM_MOVE: {
                mWinPosX = GET_X_LPARAM(lparam);
                mWinPosY = GET_Y_LPARAM(lparam);

                mWh->postWindowPosEvent(mWinPosX, mWinPosY);
                return 0;
            }
                break;
            case WM_SIZING:
                return 0;
            case WM_SIZE: {
                int width = GET_X_LPARAM(lparam);
                int height = GET_Y_LPARAM(lparam);
                if (mWidth != width || mHeight != height) {
                    mWidth = width;
                    mHeight = height;
                    mWh->postWindowSizeEvent(width, height);
                    mWh->frame();
                }
                return 0;
            }

            case WM_SYSCOMMAND:
                switch (wparam) {
                    case SC_MINIMIZE:
                    case SC_RESTORE: {
                        HWND parent = GetWindow(hwnd, GW_OWNER);
                        if (NULL != parent) {
                            PostMessage(parent, id, wparam, lparam);
                        }
                    }
                }
                break;

            case WM_MOUSEMOVE: {
                int mx = GET_X_LPARAM(lparam);
                int my = GET_Y_LPARAM(lparam);

                if (mCursorMode == CursorMode::Disabled) {
                    const int dx = mx - mLastMouseX;
                    const int dy = my - mLastMouseY;

                    inputCursorPos(mVirtualCursorPosX + dx, mVirtualCursorPosY + dy);
                } else {
                    inputCursorPos(mx, my);
                }

                mLastMouseX = mx;
                mLastMouseY = my;

                return 0;
            }

            case WM_MOUSEWHEEL: {
                int sy = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
                mAppContext->postMouseScrollEvent(mWh->getWindowId(), 0, sy);
                return 0;
            }

            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_LBUTTONDBLCLK: {
                mouseCapture(hwnd, id == WM_LBUTTONDOWN);
                int mx = GET_X_LPARAM(lparam);
                int my = GET_Y_LPARAM(lparam);
                mAppContext->postMouseButtonEvent(mWh->getWindowId(), MouseButton::Left,
                                                  id == WM_LBUTTONDOWN ? KeyAction::Press : KeyAction::Release);
                return 0;
            }

            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_MBUTTONDBLCLK: {
                mouseCapture(hwnd, id == WM_MBUTTONDOWN);
                int mx = GET_X_LPARAM(lparam);
                int my = GET_Y_LPARAM(lparam);
                mAppContext->postMouseButtonEvent(mWh->getWindowId(), MouseButton::Middle,
                                                  id == WM_MBUTTONDOWN ? KeyAction::Press : KeyAction::Release);
                return 0;
            }

            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_RBUTTONDBLCLK: {
                mouseCapture(hwnd, id == WM_RBUTTONDOWN);
                int mx = GET_X_LPARAM(lparam);
                int my = GET_Y_LPARAM(lparam);
                mAppContext->postMouseButtonEvent(mWh->getWindowId(), MouseButton::Right,
                                                  id == WM_RBUTTONDOWN ? KeyAction::Press : KeyAction::Release);
                return 0;
            }

            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYUP: {
                uint8_t modifiers = translateKeyModifiers();
                Key::Enum key = translateKey(wparam);

                if (Key::Print == key
                    && 0x3 == ((int) (lparam) >> 30)) {
                    // VK_SNAPSHOT doesn't generate keydown event. Fire on down event when previous
                    // key state bit is set to 1 and transition state bit is set to 1.
                    //
                    // http://msdn.microsoft.com/en-us/library/windows/desktop/ms646280%28v=vs.85%29.aspx
                    mAppContext->postKeyEvent(mWh->getWindowId(), key, modifiers, KeyAction::Press);
                }

                mAppContext->postKeyEvent(mWh->getWindowId(), key, modifiers,
                                          id == WM_KEYDOWN || id == WM_SYSKEYDOWN ? KeyAction::Press
                                                                                  : KeyAction::Release);
            }
                break;

            case WM_CHAR: {
                WCHAR utf16[2] = {(WCHAR) wparam};
                unsigned char utf8[4] = {};

                if (utf16[0] >= 0xD800 && utf16[0] <= 0xDBFF) {
                    mSurrogate = utf16[0];
                } else {
                    int utf16_len;
                    if (utf16[0] >= 0xDC00 && utf16[0] <= 0xDFFF) {
                        utf16[1] = utf16[0];
                        utf16[0] = mSurrogate;
                        mSurrogate = 0;
                        utf16_len = 2;
                    } else {
                        utf16_len = 1;
                    }

                    int len = WideCharToMultiByte(CP_UTF8, 0, utf16, utf16_len, (LPSTR) utf8,
                                                  ARRAY_LEN(utf8), NULL, NULL);
                    if (0 != len) {
                        mAppContext->postCharInputEvent(mWh->getWindowId(), std::string((char *) utf8, len));
                    }
                }
                return 0;
            }

            case WM_DROPFILES: {
                HDROP drop = (HDROP) wparam;
                const int maxPathLen = 1024;
                int count = DragQueryFile(drop, 0xFFFFFFFF, NULL, 0);
                std::vector<std::string> filePaths;
                if (count) {
                    filePaths.resize(count);
                    for (int nIndex = 0; nIndex < count; ++nIndex) {
                        char tmp[maxPathLen];
                        WCHAR utf16[maxPathLen];
                        uint32_t result = DragQueryFileW(drop, nIndex, utf16, maxPathLen);
                        WideCharToMultiByte(CP_UTF8, 0, utf16, -1, tmp, maxPathLen, NULL, NULL);
                        filePaths[nIndex] = tmp;
                    }
                }
                mWh->postDropEvent(filePaths);
                return 0;
            }

                // Maintain cursor state
            case WM_SETCURSOR: {
                if (LOWORD(lparam) == HTCLIENT && mCursor != nullptr) {
                    mCursor->updateCursorImage(this);
                    return TRUE;
                }
            }
                break;

            case WM_SETFOCUS: {
                mWh->postWindowFocusChange(true);
                return 0;
            }

            case WM_KILLFOCUS: {
                mWh->postWindowFocusChange(false);
                return 0;
            }

            default:
                break;
        }

        return DefWindowProcW(hwnd, id, wparam, lparam);
    }

    void inputCursorPos(int32_t x, int32_t y)
    {
        if (mVirtualCursorPosX == x && mVirtualCursorPosY == y)
            return;

        mVirtualCursorPosX = x;
        mVirtualCursorPosY = y;

        mAppContext->postMouseMoveEvent(mWh->getWindowId(), x, y);
    }

    void destroyCursor()
    {
        if (mCursor != nullptr) {
            SetCursor(LoadCursor(nullptr, IDC_ARROW));
            delete mCursor;
            mCursor = nullptr;
        }
    }

public:
    HWND getNativeWindow() const
    {
        return mNativeWindow;
    }

    CursorMode::Enum getCursorMode() const
    {
        return mCursorMode;
    }

    void exitFullScreen()
    {
        SetWindowLongA(mNativeWindow, GWL_STYLE, mNativeFlags);
        setWindowSize(mOriWidth, mOriHeight);
        setWindowPos(mOriWinPosX, mOriWinPosY);
    }

private:
    static LRESULT CALLBACK wndProc(HWND hwnd, UINT id, WPARAM wparam, LPARAM lparam)
    {
        NWndWindow *win = reinterpret_cast<NWndWindow *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (win) {
            return win->process(hwnd, id, wparam, lparam);
        }
        return DefWindowProcW(hwnd, id, wparam, lparam);
    }

    static inline WSTRING UTF8ToUTF16(const char *utf8_str)
    {
        int len = MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, NULL, 0);
        WSTRING utf16(len);
        MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, utf16.data(), len);
        return utf16;
    }

    static void mouseCapture(HWND hwnd, bool capture)
    {
        if (capture) {
            SetCapture(hwnd);
        } else {
            ReleaseCapture();
        }
    }

    static uint8_t translateKeyModifiers()
    {
        uint8_t modifiers = 0;
        for (uint32_t ii = 0; ii < ARRAY_LEN(sTranslateKeyModifiers); ++ii) {
            const TranslateKeyModifiers &tkm = sTranslateKeyModifiers[ii];
            modifiers |= 0 > GetKeyState(tkm.vk) ? tkm.modifier : Modifier::None;
        }
        return modifiers;
    }

    static Key::Enum translateKey(WPARAM wparam)
    {
        return sTranslateKey[wparam & 0xff];
    }

    static void getDesktopSize(int32_t &w, int32_t &h)
    {
        RECT rc;
        HWND deskW = GetDesktopWindow();
        GetWindowRect(deskW, &rc);

        w = rc.right - rc.left;
        h = rc.bottom - rc.top;
    }

private:
    AppContext *mAppContext = nullptr;
    WindowHandle *mWh = nullptr;

    HWND mNativeWindow = nullptr;

    bool mExit = false;

    WCHAR mSurrogate{};

    MSG mMsg{};

    int32_t mWidth = 0, mHeight = 0;
    int32_t mOriWidth = 0, mOriHeight = 0;

    int32_t mWinPosX = 0, mWinPosY = 0;
    int32_t mOriWinPosX = 0, mOriWinPosY = 0;

    int32_t mLastMouseX = 0, mLastMouseY = 0;

    WindowState::Enum mWinState = WindowState::Normal;
    int32_t mNativeFlags = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

    CursorMode::Enum mCursorMode = CursorMode::Normal;
    NCursor *mCursor = nullptr;
    int32_t mVirtualCursorPosX = 0, mVirtualCursorPosY = 0;
    int32_t mRestoreCursorPosX = 0, mRestoreCursorPosY = 0;
};


/** ==== NGamepadMgr ==== **/

void NGamepadMgr::init(IGamepadDeviceDriver *gamepadDD)
{
    mGamepadDD = gamepadDD;

    // xinput
    const char *names[] =
            {
                    "xinput1_4.dll",
                    "xinput1_3.dll",
                    "xinput9_1_0.dll",
                    "xinput1_2.dll",
                    "xinput1_1.dll",
                    nullptr
            };

    for (int i = 0; names[i]; i++) {
        mXinput.instance = LoadLibraryA(names[i]);
        if (mXinput.instance) {
            mXinput.GetCapabilities = (PFN_XInputGetCapabilities)
                    GetProcAddress(mXinput.instance, "XInputGetCapabilities");
            mXinput.GetState = (PFN_XInputGetState)
                    GetProcAddress(mXinput.instance, "XInputGetState");

            break;
        }
    }

    memset(mXStates, 0, sizeof(mXStates));
}

void NGamepadMgr::destroy()
{
    if (mXinput.instance) {
        FreeLibrary(mXinput.instance);
    }

    mGamepadDD = nullptr;
}

void NGamepadMgr::update()
{
    if (!mXinput.instance) {
        return;
    }

    for (uint32_t jid = 0; jid < MAX_GAMEPAD_COUNT; jid++) {
        XINPUT_STATE state;
        DWORD result = mXinput.GetState(jid, &state);

        bool connected = ERROR_SUCCESS == result;
        if (connected != mConnected[jid]) {
            if (connected) {
                // Get device name
                XINPUT_CAPABILITIES xic;
                mXinput.GetCapabilities(jid, XINPUT_FLAG_GAMEPAD, &xic);
                mGamepadDD->postGamepadStateEvent(
                        {jid, getDeviceDescription(&xic), GamepadAction::Connected});
            } else {
                mGamepadDD->postGamepadStateEvent(
                        {jid, "", GamepadAction::DisConnected});
            }
            mConnected[jid] = connected;
        }

        if (connected && mXStates[jid].dwPacketNumber != state.dwPacketNumber) {
            GamepadInfo &info = mGamepadInfos[jid];

            XINPUT_GAMEPAD &gamepad = mXStates[jid].Gamepad;
            const uint16_t changed = gamepad.wButtons ^ state.Gamepad.wButtons;
            const uint16_t current = gamepad.wButtons;
            bool hasChanged = false;
            if (changed != 0) {
                for (auto &jj : sXinputRemap) {
                    uint16_t bit = jj.bit;
                    if (bit & changed) {
                        info.buttons[jj.key] =
                                0 == (current & bit) ? KeyAction::Press : KeyAction::Release;
                        hasChanged = true;
                    }
                }
                gamepad.wButtons = state.Gamepad.wButtons;
            }
            if (gamepad.bLeftTrigger != state.Gamepad.bLeftTrigger) {
                info.axes[GamepadAxis::AxisLeftTrigger] = (float) state.Gamepad.bLeftTrigger / 255.0f;
                gamepad.bLeftTrigger = state.Gamepad.bLeftTrigger;
                hasChanged = true;
            }
            if (gamepad.bRightTrigger != state.Gamepad.bRightTrigger) {
                info.axes[GamepadAxis::AxisRightTrigger] = (float) state.Gamepad.bRightTrigger / 255.0f;
                gamepad.bRightTrigger = state.Gamepad.bRightTrigger;
                hasChanged = true;
            }
            if (gamepad.sThumbLX != state.Gamepad.sThumbLX) {
                info.axes[GamepadAxis::AxisLeftX] = ((float) state.Gamepad.sThumbLX + 0.5f) / 32767.5f;
                gamepad.sThumbLX = state.Gamepad.sThumbLX;
                hasChanged = true;
            }
            if (gamepad.sThumbLY != state.Gamepad.sThumbLY) {
                info.axes[GamepadAxis::AxisLeftY] = ((float) state.Gamepad.sThumbLY + 0.5f) / 32767.5f;
                gamepad.sThumbLY = state.Gamepad.sThumbLY;
                hasChanged = true;
            }
            if (gamepad.sThumbRX != state.Gamepad.sThumbRX) {
                info.axes[GamepadAxis::AxisRightX] = ((float) state.Gamepad.sThumbRX + 0.5f) / 32767.5f;
                gamepad.sThumbRX = state.Gamepad.sThumbRX;
                hasChanged = true;
            }
            if (gamepad.sThumbRY != state.Gamepad.sThumbRY) {
                info.axes[GamepadAxis::AxisRightY] = ((float) state.Gamepad.sThumbRY + 0.5f) / 32767.5f;
                gamepad.sThumbRY = state.Gamepad.sThumbRY;
                hasChanged = true;
            }
            if (hasChanged) {
                mGamepadDD->postGamepadUpdateEvent(jid, info);
            }
        }
    }
}

std::vector<GamepadStateInfo> NGamepadMgr::getConnectedGamepadStateInfos()
{
    std::vector<GamepadStateInfo> stateInfos;
    for (uint32_t jid = 0; jid < MAX_GAMEPAD_COUNT; jid++) {
        XINPUT_STATE state;
        DWORD result = mXinput.GetState(jid, &state);

        bool connected = ERROR_SUCCESS == result;
        if (connected) {
            // Get device name
            XINPUT_CAPABILITIES xic;
            mXinput.GetCapabilities(jid, XINPUT_FLAG_GAMEPAD, &xic);
            stateInfos.push_back({jid, getDeviceDescription(&xic), GamepadAction::Connected});
        }
        mConnected[jid] = connected;
    }
    return stateInfos;
}

std::string NGamepadMgr::getDeviceDescription(const XINPUT_CAPABILITIES *xic)
{
    switch (xic->SubType) {
        case XINPUT_DEVSUBTYPE_WHEEL:
            return "XInput Wheel";
        case XINPUT_DEVSUBTYPE_ARCADE_STICK:
            return "XInput Arcade Stick";
        case XINPUT_DEVSUBTYPE_FLIGHT_STICK:
            return "XInput Flight Stick";
        case XINPUT_DEVSUBTYPE_DANCE_PAD:
            return "XInput Dance Pad";
        case XINPUT_DEVSUBTYPE_GUITAR:
            return "XInput Guitar";
        case XINPUT_DEVSUBTYPE_DRUM_KIT:
            return "XInput Drum Kit";
        case XINPUT_DEVSUBTYPE_GAMEPAD: {
            if (xic->Flags & XINPUT_CAPS_WIRELESS)
                return "Wireless Xbox Controller";
            else
                return "Xbox Controller";
        }
    }

    return "Unknown XInput Device";
}


/** ==== NCursor ==== **/

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
    if (mCursor) {
        DestroyIcon((HICON) mCursor);
    }
}

void NCursor::setToCursor(NWndWindow *window)
{
    if (cursorInContentArea(window)) {
        updateCursorImage(window);
    }
}

#define W_OCR_NORMAL          32512
#define W_OCR_IBEAM           32513
#define W_OCR_WAIT            32514
#define W_OCR_CROSS           32515
#define W_OCR_UP              32516
#define W_OCR_SIZE            32640   /* OBSOLETE: use OCR_SIZEALL */
#define W_OCR_ICON            32641   /* OBSOLETE: use OCR_NORMAL */
#define W_OCR_SIZENWSE        32642
#define W_OCR_SIZENESW        32643
#define W_OCR_SIZEWE          32644
#define W_OCR_SIZENS          32645
#define W_OCR_SIZEALL         32646
#define W_OCR_ICOCUR          32647   /* OBSOLETE: use OIC_WINLOGO */
#define W_OCR_NO              32648
#if(WINVER >= 0x0500)
#define W_OCR_HAND            32649
#endif /* WINVER >= 0x0500 */
#if(WINVER >= 0x0400)
#define W_OCR_APPSTARTING     32650
#endif /* WINVER >= 0x0400 */


void NCursor::createShapeCursor(Cursor::CursorShape shape)
{
    int id;
    switch (shape) {
        default:
        case Cursor::Arrow:
            id = W_OCR_NORMAL;
            break;
        case Cursor::IBeam:
            id = W_OCR_IBEAM;
            break;
        case Cursor::Cross:
            id = W_OCR_CROSS;
            break;
        case Cursor::Hand:
            id = W_OCR_HAND;
            break;
        case Cursor::SizeVer:
            id = W_OCR_SIZENS;
            break;
        case Cursor::SizeHor:
            id = W_OCR_SIZEWE;
            break;
    }

    mCursor = (HCURSOR) LoadImage(nullptr,
                                  MAKEINTRESOURCE(id), IMAGE_CURSOR, 0, 0,
                                  LR_DEFAULTSIZE | LR_SHARED);
}

void NCursor::createCustomCursor(const CursorBitmap &bitmap, int32_t hotX, int32_t hotY)
{
    mCursor = (HCURSOR) createIcon(bitmap, hotX, hotY);
}

HICON NCursor::createIcon(const CursorBitmap &bitmap, int hotX, int hotY)
{
    int i;
    HDC dc;
    HICON handle;
    HBITMAP color, mask;
    BITMAPV5HEADER bi;
    ICONINFO ii;
    unsigned char *target = nullptr;
    unsigned char *source = (unsigned char *) bitmap.data();

    ZeroMemory(&bi, sizeof(bi));
    bi.bV5Size = sizeof(bi);
    bi.bV5Width = bitmap.width();
    bi.bV5Height = -bitmap.height();
    bi.bV5Planes = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;
    bi.bV5RedMask = 0x00ff0000;
    bi.bV5GreenMask = 0x0000ff00;
    bi.bV5BlueMask = 0x000000ff;
    bi.bV5AlphaMask = 0xff000000;

    dc = GetDC(nullptr);
    color = CreateDIBSection(dc,
                             (BITMAPINFO *) &bi,
                             DIB_RGB_COLORS,
                             (void **) &target,
                             NULL,
                             (DWORD) 0);
    ReleaseDC(nullptr, dc);

    if (!color) {
        return nullptr;
    }

    mask = CreateBitmap(bitmap.width(), bitmap.height(), 1, 1, NULL);
    if (!mask) {
        DeleteObject(color);
        return nullptr;
    }

    for (i = 0; i < bitmap.width() * bitmap.height(); i++) {
        target[0] = source[2];
        target[1] = source[1];
        target[2] = source[0];
        target[3] = source[3];
        target += 4;
        source += 4;
    }

    ZeroMemory(&ii, sizeof(ii));
    ii.fIcon = false;
    ii.xHotspot = hotX;
    ii.yHotspot = hotY;
    ii.hbmMask = mask;
    ii.hbmColor = color;

    handle = CreateIconIndirect(&ii);

    DeleteObject(color);
    DeleteObject(mask);

    return handle;
}

bool NCursor::cursorInContentArea(NWndWindow *window)
{
    RECT area;
    POINT pos;

    HWND nw = window->getNativeWindow();

    if (!GetCursorPos(&pos))
        return false;

    if (WindowFromPoint(pos) != nw)
        return false;

    GetClientRect(nw, &area);
    ClientToScreen(nw, (POINT *) &area.left);
    ClientToScreen(nw, (POINT *) &area.right);

    return PtInRect(&area, pos);
}

void NCursor::updateCursorImage(NWndWindow *window)
{
    if (window->getCursorMode() == CursorMode::Normal) {
        if (mCursor != nullptr)
            SetCursor(mCursor);
        else
            SetCursor(LoadCursor(nullptr, IDC_ARROW));
    } else {
        SetCursor(nullptr);
    }
}

/** ==== native extern functions ==== **/

NWindow *createNativeWindow()
{
    return new NWndWindow();
}

int nativeInit(AppContext *appCtx)
{
    SetDllDirectoryA(".");
    initStatic();
    return 0;
}

int nativeTerminate(AppContext *appCtx)
{
    if (sGamepadMgr != nullptr) {
        sGamepadMgr->destroy();
        delete sGamepadMgr;
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

std::vector<GamepadStateInfo> nativeGetConnectedGamepadStateInfos()
{
    if (sGamepadMgr) {
        return sGamepadMgr->getConnectedGamepadStateInfos();
    }
    return {};
}

void nativeGetDesktopSize(uint32_t &w, uint32_t &h)
{
    RECT rc;
    HWND deskW = GetDesktopWindow();
    GetWindowRect(deskW, &rc);

    w = rc.right - rc.left;
    h = rc.bottom - rc.top;
}

/** ==== const static functions ==== **/
static void initStatic()
{
    memset(sTranslateKey, 0, sizeof(sTranslateKey));
    sTranslateKey[VK_ESCAPE] = Key::Esc;
    sTranslateKey[VK_RETURN] = Key::Return;
    sTranslateKey[VK_TAB] = Key::Tab;
    sTranslateKey[VK_BACK] = Key::Backspace;
    sTranslateKey[VK_SPACE] = Key::Space;
    sTranslateKey[VK_UP] = Key::Up;
    sTranslateKey[VK_DOWN] = Key::Down;
    sTranslateKey[VK_LEFT] = Key::Left;
    sTranslateKey[VK_RIGHT] = Key::Right;
    sTranslateKey[VK_INSERT] = Key::Insert;
    sTranslateKey[VK_DELETE] = Key::Delete;
    sTranslateKey[VK_HOME] = Key::Home;
    sTranslateKey[VK_END] = Key::End;
    sTranslateKey[VK_PRIOR] = Key::PageUp;
    sTranslateKey[VK_NEXT] = Key::PageDown;
    sTranslateKey[VK_SNAPSHOT] = Key::Print;
    sTranslateKey[VK_OEM_PLUS] = Key::Plus;
    sTranslateKey[VK_OEM_MINUS] = Key::Minus;
    sTranslateKey[VK_OEM_4] = Key::LeftBracket;
    sTranslateKey[VK_OEM_6] = Key::RightBracket;
    sTranslateKey[VK_OEM_1] = Key::Semicolon;
    sTranslateKey[VK_OEM_7] = Key::Quote;
    sTranslateKey[VK_OEM_COMMA] = Key::Comma;
    sTranslateKey[VK_OEM_PERIOD] = Key::Period;
    sTranslateKey[VK_DECIMAL] = Key::Period;
    sTranslateKey[VK_OEM_2] = Key::Slash;
    sTranslateKey[VK_OEM_5] = Key::Backslash;
    sTranslateKey[VK_OEM_3] = Key::Tilde;
    sTranslateKey[VK_CAPITAL] = Key::CapsLock;
    sTranslateKey[VK_NUMLOCK] = Key::NumLock;
    sTranslateKey[VK_APPS] = Key::Menu;
    sTranslateKey[VK_F1] = Key::F1;
    sTranslateKey[VK_F2] = Key::F2;
    sTranslateKey[VK_F3] = Key::F3;
    sTranslateKey[VK_F4] = Key::F4;
    sTranslateKey[VK_F5] = Key::F5;
    sTranslateKey[VK_F6] = Key::F6;
    sTranslateKey[VK_F7] = Key::F7;
    sTranslateKey[VK_F8] = Key::F8;
    sTranslateKey[VK_F9] = Key::F9;
    sTranslateKey[VK_F10] = Key::F10;
    sTranslateKey[VK_F11] = Key::F11;
    sTranslateKey[VK_F12] = Key::F12;
    sTranslateKey[VK_NUMPAD0] = Key::NumPad0;
    sTranslateKey[VK_NUMPAD1] = Key::NumPad1;
    sTranslateKey[VK_NUMPAD2] = Key::NumPad2;
    sTranslateKey[VK_NUMPAD3] = Key::NumPad3;
    sTranslateKey[VK_NUMPAD4] = Key::NumPad4;
    sTranslateKey[VK_NUMPAD5] = Key::NumPad5;
    sTranslateKey[VK_NUMPAD6] = Key::NumPad6;
    sTranslateKey[VK_NUMPAD7] = Key::NumPad7;
    sTranslateKey[VK_NUMPAD8] = Key::NumPad8;
    sTranslateKey[VK_NUMPAD9] = Key::NumPad9;
    sTranslateKey[uint8_t('0')] = Key::Key0;
    sTranslateKey[uint8_t('1')] = Key::Key1;
    sTranslateKey[uint8_t('2')] = Key::Key2;
    sTranslateKey[uint8_t('3')] = Key::Key3;
    sTranslateKey[uint8_t('4')] = Key::Key4;
    sTranslateKey[uint8_t('5')] = Key::Key5;
    sTranslateKey[uint8_t('6')] = Key::Key6;
    sTranslateKey[uint8_t('7')] = Key::Key7;
    sTranslateKey[uint8_t('8')] = Key::Key8;
    sTranslateKey[uint8_t('9')] = Key::Key9;
    sTranslateKey[uint8_t('A')] = Key::KeyA;
    sTranslateKey[uint8_t('B')] = Key::KeyB;
    sTranslateKey[uint8_t('C')] = Key::KeyC;
    sTranslateKey[uint8_t('D')] = Key::KeyD;
    sTranslateKey[uint8_t('E')] = Key::KeyE;
    sTranslateKey[uint8_t('F')] = Key::KeyF;
    sTranslateKey[uint8_t('G')] = Key::KeyG;
    sTranslateKey[uint8_t('H')] = Key::KeyH;
    sTranslateKey[uint8_t('I')] = Key::KeyI;
    sTranslateKey[uint8_t('J')] = Key::KeyJ;
    sTranslateKey[uint8_t('K')] = Key::KeyK;
    sTranslateKey[uint8_t('L')] = Key::KeyL;
    sTranslateKey[uint8_t('M')] = Key::KeyM;
    sTranslateKey[uint8_t('N')] = Key::KeyN;
    sTranslateKey[uint8_t('O')] = Key::KeyO;
    sTranslateKey[uint8_t('P')] = Key::KeyP;
    sTranslateKey[uint8_t('Q')] = Key::KeyQ;
    sTranslateKey[uint8_t('R')] = Key::KeyR;
    sTranslateKey[uint8_t('S')] = Key::KeyS;
    sTranslateKey[uint8_t('T')] = Key::KeyT;
    sTranslateKey[uint8_t('U')] = Key::KeyU;
    sTranslateKey[uint8_t('V')] = Key::KeyV;
    sTranslateKey[uint8_t('W')] = Key::KeyW;
    sTranslateKey[uint8_t('X')] = Key::KeyX;
    sTranslateKey[uint8_t('Y')] = Key::KeyY;
    sTranslateKey[uint8_t('Z')] = Key::KeyZ;
}

}

#endif // ENTRY_CONFIG_USE_NATIVE && GX_PLATFORM_WINDOWS