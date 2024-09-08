#pragma once

#include <stdint.h>

#include "StringBuffer.h"

 class JsonObject
 {
 public:
    JsonObject();

    void add(const char *key, int value);
    void add(const char *key, const char *value);

    StringBuffer<64> &dump();

private:
    void addString(const char *value);
    StringBuffer<64> mBuffer;
 };
 