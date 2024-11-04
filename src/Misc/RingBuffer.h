#pragma once

#include "SString.h"

template<uint32_t BufferSize, uint32_t StringSize>
class RingBuffer
{
public:
    using StringType = SString<StringSize>;

    void append(StringType str)
    {
        mBuffer[mHead] = std::move(str);
        mHead = (mHead + 1) % mBuffer.size();

        if (full()) {
            mTail = (mTail + 1) % mBuffer.size();
        } else {
            ++mCurrentSize;
        }
    }

    StringType pop() {
        if(empty()) {
            return {};
        }

        StringType str = mBuffer[mTail];

        mTail = (mTail + 1) % mBuffer.size();
        --mCurrentSize;

        return str;
    }

    bool full() const { return mCurrentSize == mBuffer.size(); }

    bool empty() const { return mCurrentSize == 0; }

private:
    int mHead = 0;
    int mTail = 0;
    size_t mCurrentSize = 0;
    std::array<StringType, BufferSize> mBuffer;
};