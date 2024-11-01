#pragma once

#include <stdint.h>

#include "SString.h"
#include "lwjson.h"

 class JsonObject
 {
 public:
    JsonObject();
    JsonObject(SString<64> string);

    void add(const char *key, int value);
    void add(const char *key, const char *value);
    SString<64> &dump();

    int getInt(const char *key, int defaultValue);
    bool getBool(const char *key, bool defaultValue);
    SString<64> get(const char *key);

private:
    void addString(const char *value);
    SString<64> mBuffer;

    lwjson_token_t mLwTokens[4];
    lwjson_t mLwJson;
    bool mInParsed = false;
 };
 