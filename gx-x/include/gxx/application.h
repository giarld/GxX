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

#ifndef GXX_APPLICATION_H
#define GXX_APPLICATION_H

#include <gx/gglobal.h>

#include <gxx/device/device_type.h>

#include <string>


namespace gxx
{

class Window;

class AppContext;

class BaseDeviceHandler;


struct AppARG
{
    AppARG() : argc(0), argv(nullptr)
    {}

    int argc;
    char **argv;
};

class GX_API Application
{
public:
    explicit Application(int argc, char *argv[]);

    /**
     * Constructor for special platforms (ex: Android)
     */
    explicit Application();

    virtual ~Application();

public:
    int exec();

    /**
     * Proactively exiting
     * (Close all windows in sequence)
     */
    void quit();

    void addWindow(Window *window);

    void setName(const std::string &appName);

    std::string getName();

    void getDesktopSize(uint32_t &w, uint32_t &h);

public:
    AppARG *appArg();

    AppContext *getAppContext() const;

    bool deviceSupport(DeviceType::Enum type);

    void registerDeviceHandler(BaseDeviceHandler *deviceHandler);

    void unregisterDeviceHandler(BaseDeviceHandler *deviceHandler);

public:
    static Application *application();

private:
    int init();

private:
    friend class AppContext;

    AppContext *mAppContext = nullptr;

    AppARG mAppARG;

    std::string mName = "Gxx";

private:
    static Application *sApplication;
};

}

#endif //GXX_APPLICATION_H
