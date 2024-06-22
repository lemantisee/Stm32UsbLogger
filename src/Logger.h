#pragma once

#include <array>
#include <stdio.h>
#include <cstring>

#define FILENAME strrchr("\\" __FILE__, '\\') + 1

#define LOG(msg, ...) Logger::log(Logger::Info, FILENAME, __LINE__, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) Logger::log(Logger::Error, FILENAME, __LINE__, msg, ##__VA_ARGS__)

class Printer {
public:
    virtual ~Printer() = default;
    virtual void print(const char *str) = 0;
};

class Logger
{
private:
    inline static std::array<char, 256> mString;
    inline static Printer *mPrinter = nullptr;

public:
    enum Type { Info, Error };

    template<typename... Args>
    static void log(Type type, const char *file, int line, const char *fmt, Args... args)
    {
        mPrinter->print(stringFormat(fmt, args...));
    }

    static void setPrinter(Printer *printer) {
        mPrinter = printer;
    }

private:
    Logger() = default;
    ~Logger() = default;

    template<typename... Args>
    static const char *stringFormat(const char *fmt, Args... args)
    {
        int size = snprintf(nullptr, 0, fmt, args...) + 1; // Extra space for '\0'
        if (size <= 0) {
            return nullptr;
        }

        mString.fill(0);
        snprintf(mString.data(), size, fmt, args...);
        return mString.data();
    }
};
