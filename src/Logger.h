#pragma once

#include <array>
#include <cstdio>
#include <cstring>

#include "SString.h"
#include "RingBuffer.h"

#define FILENAME strrchr("/" __FILE__, '/') + 1

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

        String256 header = getInstance().createHeader(file, line);
        String256 &str = getInstance().stringFormat(fmt, args...);
        if (str.size() + header.size() + 1 >= str.capacity()) {
            return;
        }

        str = header + str + "\n";

        if (str.size() <= getInstance().mBufferStringSize) {
            getInstance().mBuffer.append(String48(str.c_str(), str.size()));
            return;
        }

        for (String48 &str : getInstance().splitString(str)) {
            if (!str.empty()) {
                getInstance().mBuffer.append(std::move(str));
            }
        }
    }

    static void enable(bool state) { getInstance().mEnabled = state; }

    static String48 pop()
    {
        if (!getInstance().mEnabled) {
            return {};
        }

        if (getInstance().mBuffer.empty()) {
            return {};
        }

        return getInstance().mBuffer.pop();
    }

    static bool empty() { return getInstance().mBuffer.empty(); }

private:
    Logger() = default;
    ~Logger() = default;

    static Logger &getInstance()
    {
        static Logger logger;
        return logger;
    }

    template<typename... Args>
    String256 &stringFormat(const char *fmt, Args... args)
    {
        mString.clear();
        int size = std::snprintf(nullptr, 0, fmt, args...) + 1; // Extra space for '\0'
        if (size > 0 || size < mString.capacity()) {
            mString.resize(size - 1);
            std::snprintf(mString.data(), size, fmt, args...);
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

            token = String48(ptr, sizeToCopy);
            strSize -= sizeToCopy;
            ptr += sizeToCopy;
        }

        return strings;
    }

    String256 createHeader(const char *file, int line) const
    {
        String256 str;
        str.append("[").append(file).append(":").appendNumber(line).append("]");
        return str;
    }

    const uint8_t mBufferStringSize = 48;

    RingBuffer<128, 48> mBuffer;
    String256 mString;
    bool mEnabled = true;
};
