#include "JsonObject.h"

#include <stdio.h>

JsonObject::JsonObject()
{
    mBuffer.append('{');
}

void JsonObject::add(const char *key, int value)
{
        if(mBuffer.capacity() > 1) {
            mBuffer += ',';
        }

        addString(key);

        mBuffer += ':';

        snprintf(&mBuffer[mBuffer.capacity()], mBuffer.size() - mBuffer.capacity(), "%i", value);
}

void JsonObject::add(const char *key, const char *value)
{
        if(mBuffer.capacity() > 1) {
            mBuffer += ',';
        }

        addString(key);

        mBuffer += ':';

        addString(value);
}

StringBuffer<64> &JsonObject::dump()
{
    if (mBuffer.capacity() >= mBuffer.size()) {
        return mBuffer;
    }

    mBuffer += '}';

    return mBuffer;
}

void JsonObject::addString(const char *value)
{
    mBuffer += '\"';
    mBuffer += value;
    mBuffer += '\"';
}
