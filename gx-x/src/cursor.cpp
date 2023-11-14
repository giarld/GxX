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

#include <utility>

#include "gxx/cursor.h"

namespace gxx
{

Cursor::Cursor(Cursor::CursorShape shape)
        : mStyle(CursorStyle::System),
          mShape(shape),
          mBitmap(),
          mHotX(0),
          mHotY(0)
{
}

Cursor::Cursor(CursorBitmap bitmap, int32_t hotX, int32_t hotY)
        : mStyle(CursorStyle::Custom),
          mShape(CursorShape::Arrow),
          mBitmap(std::move(bitmap)),
          mHotX(hotX),
          mHotY(hotY)
{
}

Cursor::Cursor(const Cursor &b) noexcept
= default;

Cursor::Cursor(Cursor &&b) noexcept
        : mStyle(b.mStyle),
          mShape(b.mShape),
          mBitmap(std::move(b.mBitmap)),
          mHotX(b.mHotX),
          mHotY(b.mHotY)
{
}

Cursor &Cursor::operator=(const Cursor &b) noexcept
{
    if (this != &b) {
        this->mStyle = b.mStyle;
        this->mShape = b.mShape;
        this->mBitmap = b.mBitmap;
        this->mHotX = b.mHotX;
        this->mHotY = b.mHotY;
    }
    return *this;
}

Cursor &Cursor::operator=(Cursor &&b) noexcept
{
    if (this != &b) {
        this->mStyle = b.mStyle;
        this->mShape = b.mShape;
        this->mBitmap = std::move(b.mBitmap);
        this->mHotX = b.mHotX;
        this->mHotY = b.mHotY;
    }
    return *this;
}

Cursor::CursorStyle Cursor::getCursorStyle() const
{
    return mStyle;
}

Cursor::CursorShape Cursor::getCursorShape() const
{
    return mShape;
}

const CursorBitmap &Cursor::getBitmap() const
{
    return mBitmap;
}

int32_t Cursor::getHotX() const
{
    return mHotX;
}

int32_t Cursor::getHotY() const
{
    return mHotY;
}

}