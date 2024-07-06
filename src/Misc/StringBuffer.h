#pragma once

#include <array>
#include <cstring>
#include <optional>

template <uint32_t N>
class StringBuffer
{
public:
    StringBuffer() = default;
    StringBuffer(const char *data, uint32_t size)
    {
        size_t sizeToCopy = std::min<size_t>(mBuffer.size(), size);
        std::memcpy(mBuffer.data(), data, sizeToCopy);
        mCurrentByte = sizeToCopy - 1;
    }

    char &operator[](uint32_t index) {
        return mBuffer[index];
    }

    bool append(char c)
    {
        if (mCurrentByte < mBuffer.size() - 1)
        {
            mBuffer[mCurrentByte] = c;
            ++mCurrentByte;
            return true;
        }

        return false;
    }

    bool append(uint16_t c)
    {
        const uint8_t low8bits = c & 0xFF;
        if (c < 255)
        {
            return append(char(low8bits));
        }

        const uint8_t high8bits = (c >> 8) & 0xFF;
        return append(char(low8bits)) && append(char(high8bits));
        return append(char(high8bits)) && append(char(low8bits));
    }

    bool append(char *data, uint32_t size)
    {
        if (mCurrentByte + size >= mBuffer.size() - 1)
        {
            return false;
        }

        for (uint32_t i = 0; i < size; ++i) {
            append(char(data[i]));
        }

        return true;
    }

    bool append(uint8_t *data, uint32_t size)
    {
        return append(reinterpret_cast<char *>(data), size);
    }

    void clear()
    {
        mBuffer.fill(0);
        mCurrentByte = 0;
    }

    const char *c_str() const
    {
        return mBuffer.data();
    }

    char *data()
    {
        return mBuffer.data();
    }

    uint32_t size() const {
        return mBuffer.size();
    }

    uint32_t capacity() const {
        return mCurrentByte;
    }

    bool empty() const {
        return mCurrentByte == 0;
    }

    bool contains(const char *substr) const {
        return std::strstr(mBuffer.data(), substr) != nullptr;
    }

    std::optional<uint32_t> find(const char *substr, uint32_t fromPos = 0) const{
        if (fromPos >= mBuffer.size()) {
            return std::nullopt;
        }

        const char *mBufferStr = mBuffer.data() + fromPos;
        if (const char *ptr = strstr(mBufferStr, substr)) {
            return uint32_t(ptr - mBufferStr);
        }

        return std::nullopt;
    }

private:
    uint32_t mCurrentByte = 0;
    std::array<char, N> mBuffer = {};
};