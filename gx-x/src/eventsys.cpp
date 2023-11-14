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

#include "gxx/eventsys.h"

namespace gxx
{

EventMana::EventMana()
{

}

EventMana::~EventMana()
{
    removeAllEventHandler();
    clearEvent();
}

EventHandler *EventMana::addEventHandler(int eventId, EventHandler *handler)
{
    if (!handler) {
        return handler;
    }
    std::vector<EventHandler *> *vec = &(mEventHandlers[eventId]);
    auto it = vec->begin();
    while (it != vec->end()) {
        if (*it == handler) {
            return handler;
        }
        it++;
    }
    vec->push_back(handler);
    return handler;
}

EventHandler *EventMana::addEventHandler(int eventId, const EventHandler::Runnable &runnable)
{
    EventHandler *handler = new EventHandler();
    handler->mRunnable = runnable;
    return addEventHandler(eventId, handler);
}

int EventMana::removeEventHandler(int eventId, EventHandler *handler)
{
    auto mapIt = mEventHandlers.find(eventId);
    if (mapIt == mEventHandlers.end()) {
        return 0;
    }
    std::vector<EventHandler *> *vec = &(mapIt->second);
    auto it = vec->begin();
    int count = 0;
    while (it != vec->end()) {
        if (*it == handler) {
            it = vec->erase(it);
            count++;
            continue;
        }
        it++;
    }
    if (vec->empty()) {
        mEventHandlers.erase(mapIt);
    }
    return count;
}

int EventMana::removeEventHandler(EventHandler *handler)
{
    auto mapIt = mEventHandlers.begin();
    int count = 0;
    while (mapIt != mEventHandlers.end()) {
        std::vector<EventHandler *> *vec = &(mapIt->second);
        auto it = vec->begin();
        while (it != vec->end()) {
            if (*it == handler) {
                it = vec->erase(it);
                count++;
                continue;
            }
            it++;
        }
        if (vec->empty()) {
            mapIt = mEventHandlers.erase(mapIt);
            continue;
        }
        mapIt++;
    }
    return count;
}

void EventMana::removeAllEventHandler()
{
    mEventHandlers.clear();
}

void EventMana::postEvent(Event *event)
{
    mEvens.push(event);
}

void EventMana::clearEvent()
{
    while (!mEvens.empty()) {
        Event *event = mEvens.front();
        mEvens.pop();
        delete event;
    }
}

void EventMana::processEvents()
{
    while (!mEvens.empty()) {
        Event *event = mEvens.front();
        mEvens.pop();
        int key = event->key();
        auto mapIt = mEventHandlers.find(key);
        if (mapIt != mEventHandlers.end()) {
            std::vector<EventHandler *> &vec = mapIt->second;
            for (auto it = vec.begin(); it != vec.end(); it++) {
                if ((*it)->mRunnable) {
                    (*it)->mRunnable(event);
                } else {
                    (*it)->handleEvent(event);
                }
            }
        }
        delete event;
    }
}

}