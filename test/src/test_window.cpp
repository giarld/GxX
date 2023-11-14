//
// Created by Gxin on 2023/11/14.
//

#include <gxx/gxxapp.h>

#include <gxx/window.h>
#include <gxx/device/charinput.h>
#include <gxx/device/gamepad.h>

#include <gx/gstring.h>
#include <gx/debug.h>


using namespace gx;
using namespace gxx;

class AWindow : public Window
{
public:
    explicit AWindow(const std::string &title)
            : Window(title)
    {}

protected:
    void init() override
    {
        Window::init();

        mCharInput = std::make_unique<CharInput>(getWindowId());
        mCharInput->setCharInputEventCallback([](const std::string &s) {
            Log("Char input: %s", s.c_str());
        });

        mGamepad = std::make_unique<Gamepad>();
        mGamepad->setGamepadStateEventCallback([](const GamepadStateInfo &stateInfo) {
            Log("Gamepad \"%s\"(%d), %s", stateInfo.name.c_str(), stateInfo.jid,
                (stateInfo.action == GamepadAction::Connected ? "Connected" : "DisConnected"));
        });
//        mGamepad->setGamepadEventCallback(0, [](uint32_t jid, const GamepadInfo &info) {
//
//        });
    }

    bool update(double delta) override
    {
        return Window::update(delta);
    }

    void resetSize(int32_t w, int32_t h) override
    {
        Log("Window resize: (%d, %d)", w, h);
    }

    void onDestroy() override
    {
        Window::onDestroy();

        mCharInput.reset();
    }

    void winMoveEvent(int32_t x, int32_t y) override
    {
        Log("Window move: (%d, %d)", x, y);
    }

    void winFocusChangeEvent(bool focused) override
    {
        Log("Window focus change: %s", (focused ? "Gain focus" : "Loss focus"));
    }

    void keyPressEvent(Key::Enum key, uint8_t modifier) override
    {
        Log("Key press: %d, %x", key, modifier);
    }

    void keyReleaseEvent(Key::Enum key, uint8_t modifier) override
    {
        Log("Key release: %d, %x", key, modifier);
    }

    void mouseMoveEvent(int32_t x, int32_t y) override
    {
        Log("Mouse move: (%d, %d)", x, y);
    }

    void mousePressEvent(MouseButton::Enum button) override
    {
        Log("Mouse press: %d", button);
    }

    void mouseReleaseEvent(MouseButton::Enum button) override
    {
        Log("Mouse release: %d", button);
    }

    void mouseScrollEvent(double xoffset, double yoffset) override
    {
        Log("Mouse scroll: %f", yoffset);
    }

    void dropEvent(const std::vector<std::string> &dropFiles) override
    {
        for (const auto &f: dropFiles) {
            Log("Drop file: %s", f.c_str());
        }
    }

private:
    std::unique_ptr<CharInput> mCharInput;
    std::unique_ptr<Gamepad> mGamepad;
};

GXX_APP_MAIN(AWindow, "TestGxX", "TestGxX")