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

#if ENTRY_CONFIG_USE_NATIVE && GX_PLATFORM_OSX

#import <Cocoa/Cocoa.h>

#include <gxx/app_entry.h>

#include <gx/math/scalar.h>


using namespace gx;

namespace gxx
{
class NCocoaWindow;
}

@interface AppDelegate : NSObject <NSApplicationDelegate>
{
    bool terminated;
}

+ (AppDelegate *)sharedDelegate;

- (id)init;

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;

- (bool)applicationHasTerminated;

@end

@interface MNWindow : NSObject <NSWindowDelegate>
{
    gxx::NCocoaWindow *mNwindow;
}

+ (MNWindow *)sharedDelegate;

- (id)init;

- (void)windowCreated:(NSWindow *)window;

- (void)windowWillClose:(NSNotification *)notification;

- (BOOL)windowShouldClose:(NSWindow *)window;

- (void)windowDidResize:(NSNotification *)notification;

- (void)windowDidMove:(NSNotification *)notification;

- (void)windowDidBecomeKey:(NSNotification *)notification;

- (void)windowDidResignKey:(NSNotification *)notification;

- (void)setGxxNWindow:(gxx::NCocoaWindow *)w;

@end


namespace gxx
{

static uint8_t __translateKey[256];

static void initStaic();

class NCocoaWindow;

class NCursor
{
public:
    explicit NCursor() = default;

    explicit NCursor(const Cursor &cursor);

    ~NCursor();

public:
    void setToCursor(NCocoaWindow *window);

    void updateCursorImage(NCocoaWindow *window);

    static bool cursorInContentArea(NCocoaWindow *window);

    void showCursor();

    void hideCursor();

private:
    void createShapeCursor(Cursor::CursorShape shape);

    void createCustomCursor(const CursorBitmap &bitmap, int32_t hotX, int32_t hotY);

private:
    id mCursor = nil;
    bool mHidden = false;
};


class NCocoaWindow : public NWindow
{
public:
    ~NCocoaWindow() override
    {
        destroyCursor();
    }

    bool init(AppContext *appCtx, WindowHandle *wh) override
    {
        mAppContext = appCtx;
        mWh = wh;

        [NSApplication sharedApplication];

        mDg = [AppDelegate sharedDelegate];
        [NSApp setDelegate:mDg];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        [NSApp activateIgnoringOtherApps:YES];
        [NSApp finishLaunching];

        [[NSNotificationCenter defaultCenter]
                postNotificationName:NSApplicationWillFinishLaunchingNotification
                              object:NSApp];

        [[NSNotificationCenter defaultCenter]
                postNotificationName:NSApplicationDidFinishLaunchingNotification
                              object:NSApp];

        id quitMenuItem = [NSMenuItem new];
        [quitMenuItem
                initWithTitle:@"Quit"
                       action:@selector(terminate:)
                keyEquivalent:@"q"];

        id appMenu = [NSMenu new];
        [appMenu addItem:quitMenuItem];

        id appMenuItem = [NSMenuItem new];
        [appMenuItem setSubmenu:appMenu];

        id menubar = [[NSMenu new] autorelease];
        [menubar addItem:appMenuItem];
        [NSApp setMainMenu:menubar];


        uint32 style = 0
                       | NSWindowStyleMaskTitled
                       | NSWindowStyleMaskResizable
                       | NSWindowStyleMaskClosable
                       | NSWindowStyleMaskMiniaturizable;

        NSRect rect = NSMakeRect(0, 0,
                                 wh->getWindowWidth(), wh->getWindowHeight());
        NSWindow *nwindow = [
                [NSWindow alloc]
                initWithContentRect:rect
                          styleMask:style
                            backing:NSBackingStoreBuffered defer:NO
        ];
        NSString *appName = [NSString stringWithUTF8String:wh->getWindowTitle().c_str()];
        [nwindow setTitle:appName];
        [nwindow makeKeyAndOrderFront:nwindow];
        [nwindow setAcceptsMouseMovedEvents:YES];
        [nwindow setBackgroundColor:[NSColor blackColor]];
        [[MNWindow sharedDelegate] windowCreated:nwindow];
        [[MNWindow sharedDelegate] setGxxNWindow:this];

        mNativeWindow = nwindow;

        setWindowPos(wh->getX(), wh->getY());

        NSRect nwindowFrame = [nwindow frame];

        osxSetNSWindow(wh, nwindow);

        setWindowFlags(wh->getWindowFlags());
        setWindowState(wh->getWindowState());

        wh->init();

        NSRect contentRect = [nwindow contentRectForFrameRect:nwindowFrame];
        mWidth = uint32_t(contentRect.size.width);
        mHeight = uint32_t(contentRect.size.height);
        wh->postWindowSizeEvent(mWidth, mHeight);
        return true;
    }

    bool isInited() override
    {
        return mNativeWindow != nullptr;
    }

    bool frame() override
    {
        @autoreleasepool {
            while (dispatchEvent(peekEvent())) {
            }
        }
        mExit = [mDg applicationHasTerminated];
        return !mExit;
    }

    void destroy() override
    {
        mNativeWindow = nullptr;
    }

public:
    void exit() override
    {
        [mNativeWindow close];
        [NSApp terminate:mNativeWindow];
    }

    void setWindowSize(uint32_t w, uint32_t h) override
    {
        NSSize size = {float(w), float(h)};
        [mNativeWindow setContentSize:size];
        setWindowPos(mWh->getX(), mWh->getY());
    }

    void setWindowPos(int x, int y) override
    {
        NSWindow *window = mNativeWindow;
        NSScreen *screen = [NSScreen mainScreen];

        NSRect screenRect = [screen frame];
        CGFloat menuBarHeight = [[[NSApplication sharedApplication] mainMenu] menuBarHeight];

        NSPoint position = {float(x), screenRect.size.height - menuBarHeight - float(y)};

        [window setFrameTopLeftPoint:position];
    }

    void setWindowTitle(const std::string &title) override
    {
        const char *title_s = title.c_str();
        NSString *ntitle = [[NSString alloc] initWithCString:title_s encoding:1];
        [mNativeWindow setTitle:ntitle];
        [ntitle release];
    }

    void setWindowState(WindowState::Enum state) override
    {
        auto isFullScreen = [this] () {
            uint32 sw, sh;
            nativeGetDesktopSize(sw, sh);
            return mWidth == sw && mHeight == sh;
        };

        switch (state) {
            case WindowState::Normal:
                if (isFullScreen()) {
                    [mNativeWindow toggleFullScreen: mNativeWindow];
                } else if ([mNativeWindow isMiniaturized]) {
                    [mNativeWindow deminiaturize: mNativeWindow];
                } else if ([mNativeWindow isZoomed]) {
                    [mNativeWindow zoom: mNativeWindow];
                }
                break;
            case WindowState::Minimized:
                if (![mNativeWindow isMiniaturized]) {
                    [mNativeWindow miniaturize: mNativeWindow];
                }
                break;
            case WindowState::Maximized:
                if (![mNativeWindow isZoomed]) {
                    [mNativeWindow zoom:mNativeWindow];
                }
                break;
            case WindowState::FullScreen:
                if (!isFullScreen()) {
                    [mNativeWindow toggleFullScreen:mNativeWindow];
                }
                break;
        }
    }

    void setWindowFlags(WindowFlags flags) override
    {
        uint32 winStyle = 0;

        if (flags & WindowFlag::ShowBorder) {
            winStyle = winStyle
                       | NSWindowStyleMaskTitled
                       | NSWindowStyleMaskClosable
                       | NSWindowStyleMaskMiniaturizable;
        }

        if (flags & WindowFlag::Resizable) {
            winStyle = winStyle | NSWindowStyleMaskResizable;
        }

        mNativeWindow.styleMask = winStyle;
    }

    void showInfoDialog(const std::string &title, const std::string &msg) override
    {
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
        // Each time the mouse offset is obtained and accumulated into a virtual mouse coordinate.
        // When disabling mode ends, restore the mouse pointer to its previous position.
        if (mode == CursorMode::Disabled) {
            getCursorPosition(mRestoreCursorPosX, mRestoreCursorPosY);
            setMousePos(mWh->getWindowWidth() / 2, mWh->getWindowHeight() / 2);
            mWrapCursorPosX = mRestoreCursorPosX - mWh->getWindowWidth() / 2;
            mWrapCursorPosY = mRestoreCursorPosY - mWh->getWindowHeight() / 2;
            CGAssociateMouseAndMouseCursorPosition(NO);
        } else if (oldMode == CursorMode::Disabled) {
            CGAssociateMouseAndMouseCursorPosition(YES);
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
        if (x < -DBL_MAX || x > DBL_MAX ||
            y < -DBL_MAX || y > DBL_MAX) {
            return;
        }

        if (!mFocused)
            return;

        if (mCursorMode == CursorMode::Disabled) {
            mVirtualCursorPosX = x;
            mVirtualCursorPosY = y;
        } else {
            if (mCursor != nullptr) {
                mCursor->updateCursorImage(this);
            }
            setMousePos(x, y);
        }
    }

    void getCursorPosition(int32_t &x, int32_t &y) override
    {
        if (mCursorMode == CursorMode::Disabled) {
            x = mVirtualCursorPosX;
            y = mVirtualCursorPosY;
        } else {
            getMousePos(mNativeWindow, &x, &y, nullptr);
        }
    }

public:
    void osxSetNSWindow(WindowHandle *window, void *nWindow, void *nsgl = nullptr)
    {
        WindowContext::PlatformData pd;
        pd.nativeDisplayType = nullptr;
        pd.nativeWindowHandle = nWindow;
        pd.context = nsgl;
        pd.backBuffer = nullptr;
        pd.backBufferDS = nullptr;
        window->setPlatformData(pd);
    }

    NSEvent *peekEvent()
    {
        return [NSApp
                nextEventMatchingMask:NSEventMaskAny
                            untilDate:[NSDate distantPast] // do not wait for event
                               inMode:NSDefaultRunLoopMode
                              dequeue:YES
        ];
    }

    bool dispatchEvent(NSEvent *event)
    {
        if (event) {
            NSEventType eventType = [event type];
            switch (eventType) {
                case NSEventTypeMouseMoved:
                case NSEventTypeLeftMouseDragged:
                case NSEventTypeRightMouseDragged:
                case NSEventTypeOtherMouseDragged: {
                    int mx, my;
                    bool out;
                    getMousePos(mNativeWindow, &mx, &my, &out);
                    if (!out) {
                        if (mCursorMode == CursorMode::Disabled) {
                            auto dx = (int32_t)[event deltaX] + mWrapCursorPosX;
                            auto dy = (int32_t)[event deltaY] + mWrapCursorPosY;

                            mWrapCursorPosX = mWrapCursorPosY = 0;

                            inputCursorPos(mVirtualCursorPosX + dx, mVirtualCursorPosY + dy);
                        } else {
                            inputCursorPos(mx, my);
                        }
                    }
                    if (mLastMouseInOut != out) {
                        mLastMouseInOut = out;
                        mouseInOutChange(!out);
                    }
                }
                    break;

                case NSEventTypeLeftMouseDown: {
                    // Command + Left Mouse Button acts as middle! This just a temporary solution!
                    // This is because the average OSX user doesn't have middle mouse click.
                    MouseButton::Enum mb = ([event modifierFlags] & NSEventModifierFlagCommand)
                                           ? MouseButton::Middle
                                           : MouseButton::Left;
                    if (mouseInWindow(mNativeWindow)) {
                        mMouseState[mb] = true;
                        mAppContext->postMouseButtonEvent(mWh->getWindowId(), mb, KeyAction::Press);
                    }
                }
                    break;

                case NSEventTypeLeftMouseUp: {
                    MouseButton::Enum mb = ([event modifierFlags] & NSEventModifierFlagCommand)
                                           ? MouseButton::Middle
                                           : MouseButton::Left;
                    if (mMouseState[mb]) {
                        mMouseState[mb] = false;
                        mAppContext->postMouseButtonEvent(mWh->getWindowId(), mb, KeyAction::Release);
                    }
                }
                    break;

                case NSEventTypeRightMouseDown: {
                    if (mouseInWindow(mNativeWindow)) {
                        mMouseState[MouseButton::Right] = true;
                        mAppContext->postMouseButtonEvent(mWh->getWindowId(), MouseButton::Right,
                                                          KeyAction::Press);
                    }
                }
                    break;

                case NSEventTypeRightMouseUp: {
                    if (mMouseState[MouseButton::Right]) {
                        mMouseState[MouseButton::Right] = false;
                        mAppContext->postMouseButtonEvent(mWh->getWindowId(), MouseButton::Right,
                                                          KeyAction::Release);
                    }
                }
                    break;

                case NSEventTypeOtherMouseDown: {
                    if (mouseInWindow(mNativeWindow)) {
                        mMouseState[MouseButton::Middle] = true;
                        mAppContext->postMouseButtonEvent(mWh->getWindowId(), MouseButton::Middle,
                                                          KeyAction::Press);
                    }
                }
                    break;

                case NSEventTypeOtherMouseUp: {
                    if (mMouseState[MouseButton::Middle]) {
                        mMouseState[MouseButton::Middle] = false;
                        mAppContext->postMouseButtonEvent(mWh->getWindowId(), MouseButton::Middle,
                                                          KeyAction::Release);
                    }
                }
                    break;

                case NSEventTypeScrollWheel: {
                    double scrollX = [event deltaX];
                    double scrollY = [event deltaY];
                    mAppContext->postMouseScrollEvent(mWh->getWindowId(), scrollX, scrollY);
                }
                    break;

                case NSEventTypeKeyDown: {
                    uint8_t modifiers = 0;
                    uint8_t pressedChar[4];
                    Key::Enum key = handleKeyEvent(event, &modifiers, &pressedChar[0]);

                    // Returning false means that we take care of the key (instead of the default behavior)
                    if (key != Key::None) {
                        mAppContext->postKeyEvent(mWh->getWindowId(), key, modifiers, KeyAction::Press);
                        mAppContext->postCharInputEvent(mWh->getWindowId(), std::string((char *) pressedChar, 4));
                    }
                    return true;
                }
                    break;

                case NSEventTypeKeyUp: {
                    uint8_t modifiers = 0;
                    uint8_t pressedChar[4];
                    Key::Enum key = handleKeyEvent(event, &modifiers, &pressedChar[0]);

                    if (key != Key::None) {
                        mAppContext->postKeyEvent(mWh->getWindowId(), key, modifiers, KeyAction::Release);
                    }
                    return true;
                }
                    break;
            }

            [NSApp sendEvent:event];
            [NSApp updateWindows];
            return true;
        }
        return false;
    }

    void getMousePos(NSWindow *window, int *outX, int *outY, bool *out)
    {
        NSRect originalFrame = [window frame];
        NSPoint location = [window mouseLocationOutsideOfEventStream];
        NSRect adjustFrame = [window contentRectForFrameRect:originalFrame];

        int x = int(location.x);
        int y = int(adjustFrame.size.height) - int(location.y);

        bool hasPress = false;
        for (bool i : mMouseState) {
            hasPress |= i;
        }

        if (hasPress) {
            *outX = x;
            *outY = y;
            if (out) {
                *out = false;
            }
        } else {
            if (out) {
                *out = x < 0 || x > adjustFrame.size.width || y < 0 || y > adjustFrame.size.height;
            }
            // clamp within the range of the window
            *outX = math::clamp(x, 0, int(adjustFrame.size.width));
            *outY = math::clamp(y, 0, int(adjustFrame.size.height));
        }
    }

    static bool mouseInWindow(NSWindow *window)
    {
        NSRect originalFrame = [window frame];
        NSPoint location = [window mouseLocationOutsideOfEventStream];
        NSRect adjustFrame = [window contentRectForFrameRect:originalFrame];

        int x = int(location.x);
        int y = int(adjustFrame.size.height) - int(location.y);
        return x >= 0 && x <= int(adjustFrame.size.width)
               && y >= 0 && y <= int(adjustFrame.size.height);
    }

    static uint8_t translateModifiers(int flags)
    {
        uint8_t ret = 0;
        if (flags & NX_DEVICELSHIFTKEYMASK) {
            ret |= Modifier::LeftShift;
        }
        if (flags & NX_DEVICERSHIFTKEYMASK) {
            ret |= Modifier::RightShift;
        }
        if (flags & NX_DEVICELALTKEYMASK) {
            ret |= Modifier::LeftAlt;
        }
        if (flags & NX_DEVICERALTKEYMASK) {
            ret |= Modifier::RightAlt;
        }
        if (flags & NX_DEVICELCTLKEYMASK) {
            ret |= Modifier::LeftCtrl;
        }
        if (flags & NX_DEVICERCTLKEYMASK) {
            ret |= Modifier::RightCtrl;
        }
        if (flags & NX_DEVICELCMDKEYMASK) {
            ret |= Modifier::LeftMeta;
        }
        if (flags & NX_DEVICERCMDKEYMASK) {
            ret |= Modifier::RightMeta;
        }

        return ret;
    }

    static Key::Enum handleKeyEvent(NSEvent *event, uint8_t *specialKeys, uint8_t *pressedChar)
    {
        NSString *key = [event charactersIgnoringModifiers];
        unichar keyChar = 0;
        if ([key length] == 0) {
            return Key::None;
        }

        keyChar = [key characterAtIndex:0];
        *pressedChar = (uint8_t) keyChar;

        int keyCode = keyChar;
        *specialKeys = translateModifiers(int([event modifierFlags]));

        // if this is a unhandled key just return None
        if (keyCode < 256) {
            return (Key::Enum) __translateKey[keyCode];
        }

        switch (keyCode) {
            case NSF1FunctionKey:
                return Key::F1;
            case NSF2FunctionKey:
                return Key::F2;
            case NSF3FunctionKey:
                return Key::F3;
            case NSF4FunctionKey:
                return Key::F4;
            case NSF5FunctionKey:
                return Key::F5;
            case NSF6FunctionKey:
                return Key::F6;
            case NSF7FunctionKey:
                return Key::F7;
            case NSF8FunctionKey:
                return Key::F8;
            case NSF9FunctionKey:
                return Key::F9;
            case NSF10FunctionKey:
                return Key::F10;
            case NSF11FunctionKey:
                return Key::F11;
            case NSF12FunctionKey:
                return Key::F12;

            case NSLeftArrowFunctionKey:
                return Key::Left;
            case NSRightArrowFunctionKey:
                return Key::Right;
            case NSUpArrowFunctionKey:
                return Key::Up;
            case NSDownArrowFunctionKey:
                return Key::Down;

            case NSPageUpFunctionKey:
                return Key::PageUp;
            case NSPageDownFunctionKey:
                return Key::PageDown;
            case NSHomeFunctionKey:
                return Key::Home;
            case NSEndFunctionKey:
                return Key::End;

            case NSPrintScreenFunctionKey:
                return Key::Print;
        }

        return Key::None;
    }

    void windowDidResize(NSWindow *window)
    {
        NSRect originalFrame = [window frame];
        NSRect rect = [window contentRectForFrameRect:originalFrame];
        mWidth = uint32_t(rect.size.width);
        mHeight = uint32_t(rect.size.height);

        mWh->postWindowSizeEvent(mWidth, mHeight);

        // Make sure mouse button state is 'up' after resize.
        mAppContext->postMouseButtonEvent(mWh->getWindowId(), MouseButton::Left, KeyAction::Release);
        mAppContext->postMouseButtonEvent(mWh->getWindowId(), MouseButton::Right, KeyAction::Release);

        mWh->frame();
    }

    void windowDidMove(NSWindow *window)
    {
        NSScreen *screen = [NSScreen mainScreen];
        NSRect windowFrame = [window frame];
        NSRect screenRect = [screen frame];
        CGFloat menuBarHeight = [[[NSApplication sharedApplication] mainMenu] menuBarHeight];

        NSPoint position = {windowFrame.origin.x,
                            screenRect.size.height - windowFrame.origin.y - windowFrame.size.height - menuBarHeight};

        mWh->postWindowPosEvent((int) position.x, (int) position.y);
    }

    void windowDidFocusChange(NSWindow *window, bool focused)
    {
        mFocused = focused;
        if (focused) {
            mouseInOutChange(NCocoaWindow::mouseInWindow(mNativeWindow));
        } else {
            mouseInOutChange(false);
        }
        mWh->postWindowFocusChange(focused);
    }

    void inputCursorPos(int32_t x, int32_t y)
    {
        if (mVirtualCursorPosX == x && mVirtualCursorPosY == y)
            return;

        mVirtualCursorPosX = x;
        mVirtualCursorPosY = y;

        mAppContext->postMouseMoveEvent(mWh->getWindowId(), x, y);
    }

    NSWindow *getNativeWindow() const
    {
        return mNativeWindow;
    }

    CursorMode::Enum getCursorMode() const
    {
        return mCursorMode;
    }

    void mouseInOutChange(bool in)
    {
        if (mCursor == nullptr) {
            mCursor = new NCursor();
        }
        if (mCursorMode == CursorMode::Hidden) {
            if (in) {
                mCursor->hideCursor();
            } else {
                mCursor->showCursor();
            }
        }
        mCursor->setToCursor(this);
    }

    void setMousePos(int32_t x, int32_t y)
    {
        @autoreleasepool {
            NSRect originalFrame = [mNativeWindow frame];
            NSRect adjustFrame = [mNativeWindow contentRectForFrameRect:originalFrame];

            CGFloat titleHeight = originalFrame.size.height - adjustFrame.size.height;
//                if (window->monitor)
//                {
//                    CGDisplayMoveCursorToPoint(window->monitor->ns.displayID,
//                                               CGPointMake(x, y));
//                }
            const NSRect localRect = NSMakeRect(x, originalFrame.size.height - y - 1, 0, 0);
            const NSRect globalRect = [mNativeWindow convertRectToScreen:localRect];
            const NSPoint globalPoint = globalRect.origin;

            CGWarpMouseCursorPosition(CGPointMake(
                    globalPoint.x,
                    CGDisplayBounds(CGMainDisplayID()).size.height - globalPoint.y + titleHeight - 1));
        } // autoreleasepool
    }

    void destroyCursor()
    {
        if (mCursor != nullptr) {
            [[NSCursor arrowCursor] set];
            delete mCursor;
            mCursor = nullptr;
        }
    }

private:
    AppContext *mAppContext = nullptr;
    WindowHandle *mWh = nullptr;
    NSWindow *mNativeWindow = nullptr;

    id mDg = nil;

    uint32_t mWidth = 0;
    uint32_t mHeight = 0;

    bool mMouseState[gxx::MouseButton::Count] = {false};

    bool mExit = false;
    bool mFocused = true;

    bool mLastMouseInOut = false;

    CursorMode::Enum mCursorMode = CursorMode::Normal;
    int32_t mVirtualCursorPosX = 0, mVirtualCursorPosY = 0;
    int32_t mRestoreCursorPosX = 0, mRestoreCursorPosY = 0;
    int32_t mWrapCursorPosX = 0, mWrapCursorPosY = 0;

    NCursor *mCursor = nullptr;
};

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
    showCursor();
    @autoreleasepool {
        if (mCursor)
            [(NSCursor*) mCursor release];
    } // autoreleasepool
}

void NCursor::setToCursor(NCocoaWindow *window)
{
    @autoreleasepool {
        if (cursorInContentArea(window)) {
            updateCursorImage(window);
        } else {
            [[NSCursor arrowCursor] set];
        }
    }
}

void NCursor::updateCursorImage(NCocoaWindow *window)
{
    if (window->getCursorMode() == CursorMode::Normal) {
        showCursor();
        if (mCursor)
            [(NSCursor*) mCursor set];
        else
            [[NSCursor arrowCursor] set];
    } else {
        hideCursor();
    }
}

bool NCursor::cursorInContentArea(NCocoaWindow *window)
{
    return gxx::NCocoaWindow::mouseInWindow(window->getNativeWindow());
}

void NCursor::showCursor()
{
    if (mHidden) {
        [NSCursor unhide];
        mHidden = false;
    }
}

void NCursor::hideCursor()
{
    if (!mHidden) {
        [NSCursor hide];
        mHidden = true;
    }
}

void NCursor::createShapeCursor(Cursor::CursorShape shape)
{
    @autoreleasepool {
        if (shape == Cursor::CursorShape::Arrow)
            mCursor = [NSCursor arrowCursor];
        else if (shape ==  Cursor::CursorShape::IBeam)
            mCursor = [NSCursor IBeamCursor];
        else if (shape == Cursor::CursorShape::Cross)
            mCursor = [NSCursor crosshairCursor];
        else if (shape == Cursor::CursorShape::Hand)
            mCursor = [NSCursor pointingHandCursor];
        else if (shape == Cursor::CursorShape::SizeHor)
            mCursor = [NSCursor resizeLeftRightCursor];
        else if (shape == Cursor::CursorShape::SizeVer)
            mCursor = [NSCursor resizeUpDownCursor];

        if (!mCursor)
        {
            return;
        }

        [mCursor retain];
    } // autoreleasepool
}

void NCursor::createCustomCursor(const CursorBitmap &bitmap, int32_t hotX, int32_t hotY)
{
    @autoreleasepool {

        NSImage* native;
        NSBitmapImageRep* rep;

        rep = [[NSBitmapImageRep alloc]
                initWithBitmapDataPlanes:NULL
                              pixelsWide:bitmap.width()
                              pixelsHigh:bitmap.height()
                           bitsPerSample:8
                         samplesPerPixel:4
                                hasAlpha:YES
                                isPlanar:NO
                          colorSpaceName:NSCalibratedRGBColorSpace
                            bitmapFormat:NSBitmapFormatAlphaNonpremultiplied
                             bytesPerRow:bitmap.width() * 4
                            bitsPerPixel:32];

        if (rep == nil)
            return;

        memcpy([rep bitmapData], bitmap.data(), bitmap.width() * bitmap.height() * 4);

        native = [[NSImage alloc] initWithSize:NSMakeSize(bitmap.width(), bitmap.height())];
        [native addRepresentation:rep];

        mCursor = [[NSCursor alloc] initWithImage:native
                                                    hotSpot:NSMakePoint(hotX, hotY)];

        [native release];
        [rep release];

    } // autoreleasepool
}

/** ==== native extern functions ==== **/

NWindow *createNativeWindow()
{
    //!! Incomplete multi window support
    static bool createOne = false;
    GX_ASSERT_S(!createOne, "The current platform does not support multiple windows");
    if (createOne) {
        return nullptr;
    }
    createOne = true;
    return new NCocoaWindow();
}

int nativeInit(AppContext *appCtx)
{
    initStaic();
    return 0;
}

int nativeTerminate(AppContext *appCtx)
{
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
    return {};
}

void nativeGetDesktopSize(uint32_t &w, uint32_t &h)
{
    NSScreen *screen = [NSScreen mainScreen];
    NSRect screenRect = [screen frame];

    w = (uint32_t)screenRect.size.width;
    h = (uint32_t)screenRect.size.height;
}

static void initStaic()
{
    __translateKey[27] = Key::Esc;
    __translateKey[uint8_t('\r')] = Key::Return;
    __translateKey[uint8_t('\t')] = Key::Tab;
    __translateKey[127] = Key::Backspace;
    __translateKey[uint8_t(' ')] = Key::Space;

    __translateKey[uint8_t('+')] =
    __translateKey[uint8_t('=')] = Key::Plus;
    __translateKey[uint8_t('_')] =
    __translateKey[uint8_t('-')] = Key::Minus;

    __translateKey[uint8_t('~')] =
    __translateKey[uint8_t('`')] = Key::Tilde;

    __translateKey[uint8_t(':')] =
    __translateKey[uint8_t(';')] = Key::Semicolon;
    __translateKey[uint8_t('"')] =
    __translateKey[uint8_t('\'')] = Key::Quote;

    __translateKey[uint8_t('{')] =
    __translateKey[uint8_t('[')] = Key::LeftBracket;
    __translateKey[uint8_t('}')] =
    __translateKey[uint8_t(']')] = Key::RightBracket;

    __translateKey[uint8_t('<')] =
    __translateKey[uint8_t(',')] = Key::Comma;
    __translateKey[uint8_t('>')] =
    __translateKey[uint8_t('.')] = Key::Period;
    __translateKey[uint8_t('?')] =
    __translateKey[uint8_t('/')] = Key::Slash;
    __translateKey[uint8_t('|')] =
    __translateKey[uint8_t('\\')] = Key::Backslash;

    __translateKey[uint8_t('0')] = Key::Key0;
    __translateKey[uint8_t('1')] = Key::Key1;
    __translateKey[uint8_t('2')] = Key::Key2;
    __translateKey[uint8_t('3')] = Key::Key3;
    __translateKey[uint8_t('4')] = Key::Key4;
    __translateKey[uint8_t('5')] = Key::Key5;
    __translateKey[uint8_t('6')] = Key::Key6;
    __translateKey[uint8_t('7')] = Key::Key7;
    __translateKey[uint8_t('8')] = Key::Key8;
    __translateKey[uint8_t('9')] = Key::Key9;

    for (char ch = 'a'; ch <= 'z'; ++ch) {
        __translateKey[uint8_t(ch)] =
        __translateKey[uint8_t(ch - ' ')] = Key::KeyA + (ch - 'a');
    }
}

}


@implementation AppDelegate

+ (AppDelegate *)sharedDelegate {
    static id delegate = [AppDelegate new];
    return delegate;
}

- (id)init {
    self = [super init];

    if (nil == self) {
        return nil;
    }

    self->terminated = false;
    return self;
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
    self->terminated = true;
    return NSTerminateCancel;
}

- (bool)applicationHasTerminated {
    return self->terminated;
}

@end

@implementation MNWindow

+ (MNWindow *)sharedDelegate {
    static id windowDelegate = [MNWindow new];
    return windowDelegate;
}

- (id)init {
    self = [super init];
    if (nil == self) {
        return nil;
    }

    return self;
}

- (void)windowCreated:(NSWindow *)window {
    assert(window);

    [window setDelegate:self];
}

- (void)windowWillClose:(NSNotification *)notification {
    NSWindow *window = [notification object];

    [window setDelegate:nil];
}

- (BOOL)windowShouldClose:(NSWindow *)window {
    assert(window);
    [NSApp terminate:self];
    return YES;
}

- (void)windowDidResize:(NSNotification *)notification {
    NSWindow *window = [notification object];
    mNwindow->windowDidResize(window);
}

- (void)windowDidMove:(NSNotification *)notification {
    NSWindow *window = [notification object];
    mNwindow->windowDidMove(window);
}

- (void)windowDidBecomeKey:(NSNotification *)notification {
    NSWindow *window = [notification object];
    mNwindow->windowDidFocusChange(window, true);
}

- (void)windowDidResignKey:(NSNotification *)notification {
    NSWindow *window = [notification object];
    mNwindow->windowDidFocusChange(window, false);
}

- (void)setGxxNWindow:(gxx::NCocoaWindow *)w {
    mNwindow = w;
}

@end

#endif // #if ENTRY_CONFIG_USE_NATIVE && GX_PLATFORM_OSX