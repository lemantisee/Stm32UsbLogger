#pragma once

#include <array>
#include <cstring>
#include <cstdio>
#include <optional>

template<uint32_t N>
class SString
{
public:
    SString() = default;
    SString(const char *data, uint32_t size)
    {
        size_t sizeToCopy = std::min<size_t>(capacity(), size);
        std::memcpy(mBuffer.data(), data, sizeToCopy);
        mCurrentByte = sizeToCopy;
    }

    SString(const char *str)
    {
        *this = str;
    }

    SString(const SString &rvl)
    {
        *this = rvl;
    }

    SString(SString &&rvl)
    {
        *this = std::move(rvl);
    }

    SString &operator=(SString &&rvl)
    {
        if (this == &rvl) {
            return *this;
        }
        mBuffer = std::move(rvl.mBuffer);
        mCurrentByte = rvl.mCurrentByte;

        rvl.mCurrentByte = 0;
        rvl.mBuffer.fill(0);

        return *this;
    }

    SString &operator=(const SString &rvl)
    {
        if (this == &rvl) {
            return *this;
        }

        mBuffer = rvl.mBuffer;
        mCurrentByte = rvl.mCurrentByte;

        return *this;
    }

    SString &operator=(const char *str)
    {
        if (mBuffer.data() == str) {
            return *this;
        }

        clear();
        append(str);
        return *this;
    }

    char &operator[](uint32_t index) { return mBuffer[index]; }

    SString &operator+(const SString &rvl)
    {
        if (size() + rvl.size() >= capacity()) {
            return *this;
        }

        std::memcpy(mBuffer.data() + mCurrentByte, rvl.mBuffer.data(), rvl.mCurrentByte);

        mCurrentByte += rvl.mCurrentByte;

        return *this;
    }

    void operator+=(char c)
    {
        append(c);
    }

    void operator+=(const char *str)
    {
        append(str);
    }

    SString &append(char c)
    {
        if (mCurrentByte < capacity() - 1) {
            mBuffer[mCurrentByte] = c;
            ++mCurrentByte;
        }

        return *this;
    }

    SString &append(const char *str)
    {
        const uint32_t size = strlen(str);
        if (mCurrentByte + size >= capacity() - 1) {
            return *this;
        }

        for (uint32_t i = 0; i < size; ++i) {
            append(char(str[i]));
        }

        return *this;
    }

    bool append(uint16_t c)
    {
        const uint8_t low8bits = c & 0xFF;
        if (c < 255) {
            return append(char(low8bits));
        }

        const uint8_t high8bits = (c >> 8) & 0xFF;
        return append(char(low8bits)) && append(char(high8bits));
        return append(char(high8bits)) && append(char(low8bits));
    }

    bool append(char *data, uint32_t size)
    {
        if (mCurrentByte + size >= capacity() - 1) {
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

    SString &appendNumber(int value)
    {
        int size = std::snprintf(nullptr, 0, "%i", value) + 1; // Extra space for '\0'
        if (size > 0 || size + mCurrentByte < capacity()) {
            snprintf(mBuffer.data() + mCurrentByte, size, "%i", value);
            mCurrentByte += size - 1;
        }

        return *this;
    }

    void clear()
    {
        mBuffer.fill(0);
        mCurrentByte = 0;
    }

    char pop()
    {
        char c = mBuffer[mCurrentByte - 1];
        mBuffer[mCurrentByte - 1] = 0;
        --mCurrentByte;
        return c;
    }

    char back() const { return mBuffer[mCurrentByte - 1]; }

    void resize(uint32_t size) { mCurrentByte = size; }

    const char *c_str() const { return mBuffer.data(); }

    char *data() { return mBuffer.data(); }

    uint32_t capacity() const { return mBuffer.size() - 1; }

    uint32_t size() const { return mCurrentByte; }

    bool empty() const { return mCurrentByte == 0; }

    bool contains(const char *substr) const
    {
        return std::strstr(mBuffer.data(), substr) != nullptr;
    }

    std::optional<uint32_t> find(const char *substr, uint32_t fromPos = 0) const
    {
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
    std::array<char, N + 1> mBuffer = {};
};
