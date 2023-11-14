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

#ifndef GXX_GXXAPP_H
#define GXX_GXXAPP_H

#include <gxx/application.h>

#if GX_PLATFORM_ANDROID
#include <android_native_app_glue.h>

#define APP_MAIN()    \
    void android_main(struct android_app* state)

#define APP_ARGS state

#define GXX_APP_MAIN(WIN_CLASS, APP_NAME, WIN_TITLE)      \
    void android_main(struct android_app* state)\
    {                                           \
        Application app();                      \
        app.setName(APP_NAME);                  \
        Window *win = new WIN_CLASS(WIN_TITLE); \
        app.addWindow(win);                     \
        app.exec();                             \
    }

#else
#define APP_MAIN()    \
    int main(int argc, char *argv[])

#define APP_ARGS argc, argv

#define GXX_APP_MAIN(WIN_CLASS, APP_NAME, WIN_TITLE)      \
    int main(int argc, char *argv[])            \
    {                                           \
        Application app(argc, argv);            \
        app.setName(APP_NAME);                  \
        Window *win = new WIN_CLASS(WIN_TITLE); \
        app.addWindow(win);                     \
        return app.exec();                      \
    }
#endif

#endif //GXX_GXXAPP_H
