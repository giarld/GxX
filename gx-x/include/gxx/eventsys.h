/*
 * Copyright (c) 2019 Gxin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef GXX_EVENTSYS_H
#define GXX_EVENTSYS_H

#include <gxx/eventhandler.h>

#include <queue>
#include <map>
#include <vector>


namespace gxx
{
class Event;

class GX_API EventMana
{
public:
    explicit EventMana();

    ~EventMana();

public:
    EventHandler *addEventHandler(int eventId, EventHandler *handler);

    EventHandler *addEventHandler(int eventId, const EventHandler::Runnable &runnable);

    int removeEventHandler(int eventId, EventHandler *handler);

    int removeEventHandler(EventHandler *handler);

    void removeAllEventHandler();

    void postEvent(Event *event);

    void clearEvent();

    void processEvents();

private:
    std::queue<Event *> mEvens;
    std::map<int, std::vector<EventHandler *>> mEventHandlers;
};

}

#endif //GXX_EVENTSYS_H
