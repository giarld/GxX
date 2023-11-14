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

#ifndef GXX_WINDOW_H
#define GXX_WINDOW_H

#include <string>
#include <vector>

#include <gxx/gui.h>
#include <gxx/guicontext.h>

#include <memory>
#include <string>


namespace gxx
{

class WindowContext;

class Keyboard;

class Mouse;

class Cursor;

class GX_API Window : public GUIContext
{
public:
    explicit Window(const std::string &title = "gxx", WindowFlags = WindowFlag::ShowBorder | WindowFlag::Resizable);

    virtual ~Window();

public:
    void setTitle(const std::string &title);

    std::string title() const;

    void setWindowState(WindowState::Enum state);

    void setWindowFlags(WindowFlags flags);

    void setSize(uint32_t w, uint32_t h);

    void setWidth(uint32_t w);

    void setHeight(uint32_t h);

    WindowState::Enum getWindowState() const;

    std::pair<uint32_t, uint32_t> size() const;

    uint32_t width() const;

    uint32_t height() const;

    std::pair<int32_t, int32_t> position() const;

    int32_t x() const;

    int32_t y() const;

    void setWindowPos(int32_t x, int32_t y);

    void close();

    bool windowFocused();

    void showInfoDialog(const std::string &title, const std::string &msg);

    /**
     * 设置光标
     *
     * @param cursor
     */
    void setCursor(const Cursor &cursor);

    /**
     * 重设光标为指针状态
     */
    void resetCursor();

    /**
     * 设置光标状态
     * 当设置为CursorMode::Disabled时，光标将不可见且鼠标事件返回的坐标将使用基于窗口空间的虚拟坐标
     * （可以包含负值和超出窗口空间的坐标值）
     *
     * @param mode
     */
    void setCursorMode(CursorMode::Enum mode);

    CursorMode::Enum getCursorMode() const;

    void setCursorPosition(int32_t x, int32_t y);

    void getCursorPosition(int32_t &x, int32_t &y) const;

public: // GUIContext functions
    Application *getApplication() const override;

protected:
    virtual void init();

    virtual bool update(double delta);

    virtual void resetSize(int32_t w, int32_t h);

    virtual void onDestroy();

    /**
     * not run on window thread
     */
    virtual void nativeLoop();

    /**
     * not run on window thread
     */
    virtual void nativeEndWait();

protected:
    virtual void winMoveEvent(int32_t x, int32_t y);

    virtual void winFocusChangeEvent(bool focused);

    virtual void keyPressEvent(Key::Enum key, uint8_t modifier);

    virtual void keyReleaseEvent(Key::Enum key, uint8_t modifier);

    virtual void mouseMoveEvent(int32_t x, int32_t y);

    virtual void mousePressEvent(MouseButton::Enum button);

    virtual void mouseReleaseEvent(MouseButton::Enum button);

    virtual void mouseScrollEvent(double xoffset, double yoffset);

    virtual void dropEvent(const std::vector<std::string> &dropFiles);

protected:
    WindowContext *getWinContext();

    uint32_t getWindowId() const;

private:
    friend class AppContext;

    friend class WindowHandle;

    std::shared_ptr<WindowContext> mWinContext;

    std::shared_ptr<Keyboard> mKeyboard;
    std::shared_ptr<Mouse> mMouse;
};

}
#endif //GXX_WINDOW_H
