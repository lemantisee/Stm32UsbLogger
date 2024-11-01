#pragma once

#include <array>
#include <stdio.h>
#include <cstring>

#include "String.h"
#include "RingBuffer.h"

#define FILENAME strrchr("\\" __FILE__, '\\') + 1

#define LOG(msg, ...) Logger::log(Logger::Info, FILENAME, __LINE__, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) Logger::log(Logger::Error, FILENAME, __LINE__, msg, ##__VA_ARGS__)

class Logger
{
public:
    enum Type { Info, Error };
    using String48 = SString<48>;
    using String256 = SString<256>;

    template<typename... Args>
    static void log(Type type, const char *file, int line, const char *fmt, Args... args)
    {
        if (!getInstance().mEnabled) {
            return;
        }

        const String256 &str = stringFormat(fmt, args...);
        if (str.size() <= getInstance().mBufferStringSize) {
            String48 str(str.c_str(), 48);
            getInstance().mBuffer.append(std::move(str));
            return;
        }

        for (String48 &str : getInstance().splitString(str)) {
            if (!str.empty()) {
                getInstance().mBuffer.append(std::move(str));
            }
        }
    }

    static void enable(bool state) { getInstance().mEnabled = state; }

private:
    Logger() = default;
    ~Logger() = default;

    static Logger &getInstance()
    {
        static Logger logger;
        return logger;
    }

    template<typename... Args>
    const String256 &stringFormat(const char *fmt, Args... args)
    {
        mString.clear();
        int size = snprintf(nullptr, 0, fmt, args...) + 1; // Extra space for '\0'
        if (size > 0 || size < mString.capacity()) {
            snprintf(mString.data(), size, fmt, args...);
        }

        return mString;
    }

    std::array<String48, 6> splitString(const String256 &str) const
    {
        size_t strSize = str.size();
        std::array<String48, 6> strings;

        const char *ptr = str.c_str();
        for(String48 &token: strings) {
            if (strSize == 0) {
                break;
            }

            const size_t sizeToCopy = strSize < mBufferStringSize ? strSize : mBufferStringSize;

            token = String48(str.c_str(), sizeToCopy);
            strSize -= sizeToCopy;
        }

        return strings;
    }

    const uint8_t mBufferStringSize = 48;

    RingBuffer<128, 48> mBuffer;
    String256 mString;
    bool mEnabled = false;
};
