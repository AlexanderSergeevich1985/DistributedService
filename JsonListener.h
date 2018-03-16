#include <QString>
#pragma once

class JsonListener {
public:
    virtual void whitespace(char c) = 0;
    virtual void startDocument() = 0;
    virtual void key(QString key) = 0;
    virtual void value(QString value) = 0;
    virtual void endArray() = 0;
    virtual void endObject() = 0;
    virtual void endDocument() = 0;
    virtual void startArray() = 0;
    virtual void startObject() = 0;
};

