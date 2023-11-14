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

#if ENTRY_CONFIG_USE_NATIVE && GX_PLATFORM_ANDROID

#include "gxx/app_entry.h"
#include "gxx/application.h"

#include <android_native_app_glue.h>
#include <android/window.h>

#include <gx/debug.h>


using namespace gx;

namespace gxx
{

class NAndroidWindow : public NWindow
{
public:
    ~NAndroidWindow() override
    = default;

    bool init(AppContext *appCtx, WindowHandle *wh) override
    {
        mAnApp = (android_app *)Application::application()->appArg()->argd;
        if (!mAnApp) {
            return false;
        }
        mAppContext = appCtx;
        mWh = wh;

        mAnApp->userData = (void *) this;
        mAnApp->onAppCmd = onAppCmdCB;
        mAnApp->onInputEvent = onInputEventCB;

        ANativeActivity_setWindowFlags(mAnApp->activity,
                                       0 | AWINDOW_FLAG_FULLSCREEN | AWINDOW_FLAG_KEEP_SCREEN_ON,
                                       0
        );

        return true;
    }

    bool isInited() override
    {
        return mAnApp != nullptr;
    }

    bool frame() override
    {
        int32_t num;
        android_poll_source *source;
        /*int32_t id =*/ ALooper_pollAll(0, NULL, &num, (void **) &source);

        if (NULL != source) {
            source->process(mAnApp, source);
        }
        return mAnApp->destroyRequested == 0;
    }

    void destroy() override
    {
//        if (mAnApp) {
//            mAnApp->activity->vm->DetachCurrentThread();
//        }
        mAnApp = nullptr;
        mNativeWindow = nullptr;
    }

public:
    void exit() override
    {
        ANativeActivity_finish(mAnApp->activity);
    }

    void setWindowSize(uint32_t w, uint32_t h) override
    {

    }

    void setWindowPos(int x, int y) override
    {

    }

    void setWindowState(WindowState::Enum state) override
    {

    }

    void setWindowFlags(WindowFlags flags) override
    {

    }

    void setWindowTitle(const std::string &title) override
    {

    }

    void showInfoDialog(const std::string &title, const std::string &msg) override
    {
        showAlert(title.c_str(), msg.c_str());
    }

    void setCursor(const Cursor &cursor) override
    {

    }

    void setCursorMode(CursorMode::Enum mode) override
    {

    }

    void setCursorPosition(int32_t x, int32_t y) override
    {

    }

    void getCursorPosition(int32_t &x, int32_t &y) override
    {

    }

private:
    void onAppCmd(int32_t cmd)
    {
        switch (cmd) {
            case APP_CMD_INPUT_CHANGED:
                break;
            case APP_CMD_INIT_WINDOW: {
                mNativeWindow = mAnApp->window;
                androidSetWindow(mNativeWindow);

//                char *argv[1] = {"android.so"};
//                mApp->appArg()->_argc = 1;
//                mApp->appArg()->_argv = argv;

                mWh->init();

                int32_t width = ANativeWindow_getWidth(mNativeWindow);
                int32_t height = ANativeWindow_getHeight(mNativeWindow);
                Log("ANativeWindow width %d, height %d", width, height);
                mWh->postWindowSizeEvent((uint32_t) width, (uint32_t) height);
            }
                break;
            case APP_CMD_TERM_WINDOW:
                // Command from main thread: the existing ANativeWindow needs to be
                // terminated.  Upon receiving this command, android_app->window still
                // contains the existing window; after calling android_app_exec_cmd
                // it will be set to NULL.
                break;

            case APP_CMD_WINDOW_RESIZED:
                // Command from main thread: the current ANativeWindow has been resized.
                // Please redraw with its new size.
                break;

            case APP_CMD_WINDOW_REDRAW_NEEDED:
                // Command from main thread: the system needs that the current ANativeWindow
                // be redrawn.  You should redraw the window before handing this to
                // android_app_exec_cmd() in order to avoid transient drawing glitches.
                break;

            case APP_CMD_CONTENT_RECT_CHANGED:
                // Command from main thread: the content area of the window has changed,
                // such as from the soft input window being shown or hidden.  You can
                // find the new content rect in android_app::contentRect.
                break;

            case APP_CMD_GAINED_FOCUS:
            {
                // Command from main thread: the app's activity window has gained
                // input focus.
//                m_eventQueue.postSuspendEvent(defaultWindow, Suspend::WillResume);
                break;
            }

            case APP_CMD_LOST_FOCUS:
            {
                // Command from main thread: the app's activity window has lost
                // input focus.
//                m_eventQueue.postSuspendEvent(defaultWindow, Suspend::WillSuspend);
                break;
            }

            case APP_CMD_CONFIG_CHANGED:
                // Command from main thread: the current device configuration has changed.
                break;

            case APP_CMD_LOW_MEMORY:
                // Command from main thread: the system is running low on memory.
                // Try to reduce your memory use.
                break;

            case APP_CMD_START:
                // Command from main thread: the app's activity has been started.
                break;

            case APP_CMD_RESUME:
            {
                // Command from main thread: the app's activity has been resumed.
//                m_eventQueue.postSuspendEvent(defaultWindow, Suspend::DidResume);
                break;
            }

            case APP_CMD_SAVE_STATE:
                // Command from main thread: the app should generate a new saved state
                // for itself, to restore from later if needed.  If you have saved state,
                // allocate it with malloc and place it in android_app.savedState with
                // the size in android_app.savedStateSize.  The will be freed for you
                // later.
                break;

            case APP_CMD_PAUSE:
            {
                // Command from main thread: the app's activity has been paused.
//                m_eventQueue.postSuspendEvent(defaultWindow, Suspend::DidSuspend);
                break;
            }

            case APP_CMD_STOP:
                // Command from main thread: the app's activity has been stopped.
                break;
            case APP_CMD_DESTROY:
                // Command from main thread: the app's activity is being destroyed,
                // and waiting for the app thread to clean up and exit before proceeding.
                mWh->postExitEvent();
                break;
            default:
                break;
        }
    }

    int32_t onInputEvent(AInputEvent *event)
    {
        const int32_t type       = AInputEvent_getType(event);
        const int32_t source     = AInputEvent_getSource(event);
        const int32_t actionBits = AMotionEvent_getAction(event);

        switch (type) {
            case AINPUT_EVENT_TYPE_MOTION:
            {
//                if (0 != (source & (AINPUT_SOURCE_GAMEPAD|AINPUT_SOURCE_JOYSTICK) ) )
//                {
                    // todo gamepad
//                    return 1;
//                }
//                else
                {
                    int mx = (int) AMotionEvent_getX(event, 0);
                    int my = (int) AMotionEvent_getY(event, 0);
                    int32_t count = AMotionEvent_getPointerCount(event);

                    int32_t action = (actionBits & AMOTION_EVENT_ACTION_MASK);
                    int32_t index  = (actionBits & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

                    if (mMouseX != mx || mMouseY != my) {
                        mMouseX = mx;
                        mMouseY = my;
                        mAppContext->postMouseMoveEvent(mWh->getWindowId(), mx, my);
                    }
                    // Simulate left mouse click with 1st touch and right mouse click with 2nd touch. ignore other touchs
                    if (count <= 2)
                    {
                        switch (action)
                        {
                            case AMOTION_EVENT_ACTION_DOWN:
                            case AMOTION_EVENT_ACTION_POINTER_DOWN:
                                mAppContext->postMouseButtonEvent(mWh->getWindowId()
                                        , action == AMOTION_EVENT_ACTION_DOWN ? MouseButton::Left : MouseButton::Right
                                        , KeyAction::Press);
                                break;

                            case AMOTION_EVENT_ACTION_UP:
                            case AMOTION_EVENT_ACTION_POINTER_UP:
                                mAppContext->postMouseButtonEvent(mWh->getWindowId()
                                        , action == AMOTION_EVENT_ACTION_UP ? MouseButton::Left : MouseButton::Right
                                        , KeyAction::Release);
                                break;

                            default:
                                break;
                        }
                    }

                    switch (action)
                    {
                        case AMOTION_EVENT_ACTION_MOVE:
                            if (0 == index)
                            {
                                mAppContext->postMouseMoveEvent(mWh->getWindowId(), mMouseX = mx, mMouseY = my);
                            }
                            break;

                        default:
                            break;
                    }
                }
            }
                break;

            case AINPUT_EVENT_TYPE_KEY:
            {
                // todo key event and Gamepad event
//                int32_t keyCode = AKeyEvent_getKeyCode(event);
//
//                if (0 != (source & (AINPUT_SOURCE_GAMEPAD|AINPUT_SOURCE_JOYSTICK) ) )
//                {
//                    for (uint32_t jj = 0; jj < BX_COUNTOF(s_gamepadRemap); ++jj)
//                    {
//                        if (keyCode == s_gamepadRemap[jj].m_keyCode)
//                        {
//                            m_eventQueue.postKeyEvent(defaultWindow, s_gamepadRemap[jj].m_key, 0, actionBits == AKEY_EVENT_ACTION_DOWN);
//                            break;
//                        }
//                    }
//                }

                return 1;
            }
                break;

            default:
                Log("onInputEvent type %d", type);
                break;
        }
        return 0;
    }

    void androidSetWindow(ANativeWindow *window)
    {
        WindowContext::PlatformData pd;
        pd.nativeDisplayType = nullptr;
        pd.nativeWindowHandle = window;
        pd.context = nullptr;
        pd.backBuffer = nullptr;
        pd.backBufferDS = nullptr;
        mWh->setPlatformData(pd);
    }

    // Displays a native alert dialog using JNI
    void showAlert(const char *title, const char* message) {
        android_app *androidApp = static_cast<android_app *>(Application::application()->appArg()->argd);
        JNIEnv* jni;
        androidApp->activity->vm->AttachCurrentThread(&jni, NULL);

        jstring jtitle = jni->NewStringUTF(title);
        jstring jmessage = jni->NewStringUTF(message);

        jclass clazz = jni->GetObjectClass(androidApp->activity->clazz);
        jmethodID methodID = jni->GetMethodID(clazz, "showAlert", "(Ljava/lang/String;Ljava/lang/String;)V");
        jni->CallVoidMethod(androidApp->activity->clazz, methodID, jtitle, jmessage);
        jni->DeleteLocalRef(jtitle);
        jni->DeleteLocalRef(jmessage);

        androidApp->activity->vm->DetachCurrentThread();
        return;
    }

private:
    static void onAppCmdCB(struct android_app *app, int32_t cmd)
    {
        NAndroidWindow *nw = (NAndroidWindow *) app->userData;
        nw->onAppCmd(cmd);
    }

    static int32_t onInputEventCB(struct android_app *app, AInputEvent *event)
    {
        NAndroidWindow *nw = (NAndroidWindow *) app->userData;
        return nw->onInputEvent(event);
    }

private:
    android_app *mAnApp = nullptr;
    AppContext *mAppContext = nullptr;
    WindowHandle *mWh = nullptr;
    ANativeWindow *mNativeWindow = nullptr;

    int mMouseX = 0;
    int mMouseY = 0;
};

NWindow *createNativeWindow()
{
    static bool created = false;
    if (created) {
        return nullptr;
    }
    created = true;
    return new NAndroidWindow();
}

int nativeInit(AppContext *appCtx)
{
    Application *app = Application::application();
    return app->appArg()->argd == nullptr ? EXIT_FAILURE : EXIT_SUCCESS;
}

int nativeTerminate(AppContext *appCtx)
{
    // If you don't do this, the Android process will not exit.
    exit(0);
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

}

}

#endif // ENTRY_CONFIG_USE_NATIVE && GX_PLATFORM_ANDROID