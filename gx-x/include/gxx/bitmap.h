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

#ifndef GXX_BITMAP_H
#define GXX_BITMAP_H

#include <vector>
#include <algorithm>
#include <memory.h>

#include <gx/allocator.h>
#include <gx/debug.h>


namespace gxx
{

template<typename Allocator, typename PIXEL>
class BitmapBase
{
private:
    static constexpr const uint32_t kPixelSize = sizeof(PIXEL);

public:
    explicit BitmapBase()
            : mAllocator(),
              mBuffer(mAllocator),
              mWidth(0),
              mHeight(0)
    {}

    explicit BitmapBase(uint32_t width, uint32_t height);

    BitmapBase(const BitmapBase &b) noexcept;

    BitmapBase(BitmapBase &&b) noexcept;

    BitmapBase<Allocator, PIXEL> &operator=(const BitmapBase<Allocator, PIXEL> &b) noexcept;

    BitmapBase<Allocator, PIXEL> &operator=(BitmapBase<Allocator, PIXEL> &&b) noexcept;

public:
    void reset(uint32_t width, uint32_t height);

    uint32_t width() const;

    uint32_t height() const;

    uint64_t byteSize() const;

    uint32_t pixelBytes() const;

    uint32_t bytesPerLine() const;

    uint32_t pixelIndex(uint32_t x, uint32_t y) const;

    unsigned char *data();

    const unsigned char *data() const;

    void setData(unsigned char *data, uint64_t size);

    void fill(PIXEL pixel);

    void setPixel(uint32_t x, uint32_t y, PIXEL pixel);

    PIXEL getPixel(uint32_t x, uint32_t y) const;

    BitmapBase<Allocator, PIXEL> copy(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const;

private:
    Allocator mAllocator;
    std::vector<PIXEL, gx::STLAllocator<PIXEL, Allocator>> mBuffer;
    uint32_t mWidth;
    uint32_t mHeight;
};


template<typename Allocator, typename PIXEL>
BitmapBase<Allocator, PIXEL>::BitmapBase(uint32_t width, uint32_t height)
        : mAllocator(),
          mBuffer(width * height, mAllocator),
          mWidth(width),
          mHeight(height)
{}

template<typename Allocator, typename PIXEL>
BitmapBase<Allocator, PIXEL>::BitmapBase(const BitmapBase &b) noexcept
        : mAllocator(),
          mBuffer(b.mBuffer, mAllocator),
          mWidth(b.mWidth),
          mHeight(b.mHeight)
{
}

template<typename Allocator, typename PIXEL>
BitmapBase<Allocator, PIXEL>::BitmapBase(BitmapBase &&b) noexcept
        : mAllocator(),
          mBuffer(b.mBuffer, mAllocator),
          mWidth(b.mWidth),
          mHeight(b.mHeight)
{
    b.mBuffer.clear();
    b.mWidth = 0;
    b.mHeight = 0;
}

template<typename Allocator, typename PIXEL>
BitmapBase<Allocator, PIXEL> &BitmapBase<Allocator, PIXEL>::operator=(const BitmapBase<Allocator, PIXEL> &b) noexcept
{
    if (this != &b) {
        mBuffer = b.mBuffer;
        mWidth = b.mWidth;
        mHeight = b.mHeight;
    }
    return *this;
}

template<typename Allocator, typename PIXEL>
BitmapBase<Allocator, PIXEL> &BitmapBase<Allocator, PIXEL>::operator=(BitmapBase<Allocator, PIXEL> &&b) noexcept
{
    if (this != &b) {
        mBuffer = b.mBuffer;
        mWidth = b.mWidth;
        mHeight = b.mHeight;
        b.mBuffer.clear();
        b.mWidth = 0;
        b.mHeight = 0;
    }
    return *this;
}

template<typename Allocator, typename PIXEL>
void BitmapBase<Allocator, PIXEL>::reset(uint32_t width, uint32_t height)
{
    mBuffer.clear();
    mBuffer.resize(width * height);
    mWidth = width;
    mHeight = height;
}

template<typename Allocator, typename PIXEL>
uint32_t BitmapBase<Allocator, PIXEL>::width() const
{
    return mWidth;
}

template<typename Allocator, typename PIXEL>
uint32_t BitmapBase<Allocator, PIXEL>::height() const
{
    return mHeight;
}

template<typename Allocator, typename PIXEL>
uint64_t BitmapBase<Allocator, PIXEL>::byteSize() const
{
    return mBuffer.size() * kPixelSize;
}

template<typename Allocator, typename PIXEL>
uint32_t BitmapBase<Allocator, PIXEL>::pixelBytes() const
{
    return kPixelSize;
}

template<typename Allocator, typename PIXEL>
uint32_t BitmapBase<Allocator, PIXEL>::bytesPerLine() const
{
    return mWidth * kPixelSize;
}

template<typename Allocator, typename PIXEL>
uint32_t BitmapBase<Allocator, PIXEL>::pixelIndex(uint32_t x, uint32_t y) const
{
    return mWidth * y + x;
}

template<typename Allocator, typename PIXEL>
unsigned char *BitmapBase<Allocator, PIXEL>::data()
{
    return (unsigned char *) mBuffer.data();
}

template<typename Allocator, typename PIXEL>
const unsigned char *BitmapBase<Allocator, PIXEL>::data() const
{
    return (unsigned char *) mBuffer.data();
}

template<typename Allocator, typename PIXEL>
void BitmapBase<Allocator, PIXEL>::setData(unsigned char *data, uint64_t size)
{
    uint64_t maxSize = byteSize();
    size = size > maxSize ? maxSize : size;
    memcpy((unsigned char*)mBuffer.data(), data, size);
}

template<typename Allocator, typename PIXEL>
void BitmapBase<Allocator, PIXEL>::fill(PIXEL pixel)
{
    std::fill_n(mBuffer.begin(), mBuffer.size(), pixel);
}

template<typename Allocator, typename PIXEL>
void BitmapBase<Allocator, PIXEL>::setPixel(uint32_t x, uint32_t y, PIXEL pixel)
{
    GX_ASSERT(x < mWidth);
    GX_ASSERT(y < mHeight);

    mBuffer[pixelIndex(x, y)] = pixel;
}

template<typename Allocator, typename PIXEL>
PIXEL BitmapBase<Allocator, PIXEL>::getPixel(uint32_t x, uint32_t y) const
{
    GX_ASSERT(x < mWidth);
    GX_ASSERT(y < mHeight);

    return mBuffer[pixelIndex(x, y)];
}

template<typename Allocator, typename PIXEL>
BitmapBase<Allocator, PIXEL>
BitmapBase<Allocator, PIXEL>::copy(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const
{
    GX_ASSERT(width > 0);
    GX_ASSERT(height > 0);
    GX_ASSERT(x + width < mWidth);
    GX_ASSERT(y + height < mHeight);

    BitmapBase<Allocator, PIXEL> chip(width, height);

    for (uint32_t yi = 0; yi < height; yi++) {
        for (uint32_t xi = 0; xi < width; xi++) {
            chip.setPixel(xi, yi, this->getPixel(x + xi, y + yi));
        }
    }

    return chip;
}


template<typename PIXEL>
using Bitmap = BitmapBase<gx::HeapPond, PIXEL>;

}

#endif //GXX_BITMAP_H
