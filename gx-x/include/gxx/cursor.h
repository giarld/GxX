/*
 * Copyright (c) 2021 Gxin
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

#ifndef GXX_CURSOR_H
#define GXX_CURSOR_H

#include "bitmap.h"
#include "color.h"


namespace gxx
{

using CursorBitmap = Bitmap<Rgba>;

/**
 * Mouse cursor
 * Only available on platforms that support custom mouse cursors
 */
class GX_API Cursor
{
public:
    /**
     * Cursor style
     */
    enum CursorStyle
    {
        System,     // System style
        Custom      // Custom style
    };

    /**
     * System default cursor shape enumeration
     */
    enum CursorShape
    {
        Arrow = 0,  // Default arrow cursor
        IBeam,      // I-shaped (text selection style)
        Cross,      // Cross shaped
        Hand,       // Hand shape
        SizeVer,    // Adjust the vertical size shape
        SizeHor,    // Adjust horizontal size shape

        Count
    };

public:
    /**
     * Building cursors from system styles
     *
     * @param shape
     */
    explicit Cursor(CursorShape shape = CursorShape::Arrow);

    /**
     * Construct a cursor
     * @param bitmap    The bitmap displayed by the cursor is recommended to use a 32 x 32 RGBA 8888 bitmap. This size is currently supported by all mainstream platforms, and other sizes may not be guaranteed
     * @param hotX      The coordinate X of the hot spot position of the cursor, that is, the position of the cursor indicating the point relative to the bitmap
     * @param hotY      The coordinate Y of the hot spot position of the cursor, that is, the position of the cursor indicating the point relative to the bitmap
     */
    explicit Cursor(CursorBitmap bitmap, int32_t hotX, int32_t hotY);

    Cursor(const Cursor &b) noexcept;

    Cursor(Cursor &&b) noexcept;

    Cursor &operator=(const Cursor &b) noexcept;

    Cursor &operator=(Cursor &&b) noexcept;

public:
    CursorStyle getCursorStyle() const;

    CursorShape getCursorShape() const;

    const CursorBitmap &getBitmap() const;

    int32_t getHotX() const;

    int32_t getHotY() const;

private:
    CursorStyle mStyle = CursorStyle::System;
    CursorShape mShape = CursorShape::Arrow;
    CursorBitmap mBitmap;
    int32_t mHotX = 0;
    int32_t mHotY = 0;
};

}

#endif //GXX_CURSOR_H
