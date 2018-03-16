/**The MIT License (MIT)

Copyright (c) 2018 by AleksanderSergeevich

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once
#include <QString>

#include "json_stream/model/json_item.h"
#include "json_stream/JsonListener.h"

/*Часть реализации JSON parser мной написанная*/

class JSON_Reader_Listener: public JsonListener {
public:
    /*
     * Для обработки события - детектирован пробел
     * For event processing - whitespace detected
    */
    virtual void whitespace(char c);
    /*
     * Для обработки события - начало документа детектировано
     * For event processing - beginning of document detected
    */
    virtual void startDocument();
    /*
     * Для обработки события - ключ обнаружен
     * For event processing - key detected
    */
    virtual void key(QString key);
    /*
     * Для обработки события - значение обнаружено
     * For event processing - value detected
    */
    virtual void value(QString value);
    /*
     * Для обработки события - конец массива обнаружен
     * For event processing - end of array detected
    */
    virtual void endArray();
    /*
     * Для обработки события - конец объекта обнаружен
     * For event processing - end of object detected
    */
    virtual void endObject();
    /*
     * Для обработки события - конец документа обнаружен
     * For event processing - end of document detected
    */
    virtual void endDocument();
    /*
     * Для обработки события - начало массива обнаружено
     * For event processing - beginning of array detected
    */
    virtual void startArray();
    /*
     * Для обработки события - начало объекта обнаружено
     * For event processing - beginning of object detected
    */
    virtual void startObject();
    /*
     * Для очистки объекта, служащего для хранения распарсенного json сообщения
     * For resetting object designed for storing data of parsed json message
    */
    void reset_object();
    /*
     * Для получения указателя объекта, служащего для хранения распарсенного json сообщения
     * Getter to get pointer to data structure designed for storing data of parsed json message
    */
    JSON_item* get_model();
private:
    JSON_item* root_item;
    JSON_item* obj_item;
};
