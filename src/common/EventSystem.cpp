//
// Created by Yun on 2021/11/13.
//

#include "EventSystem.h"
#include "TimeSys.h"
#include <algorithm>
#include <iostream>
#include <utility>

namespace
{
    bool sortCompareListener(const ListenerWeekRef& l1, const ListenerWeekRef& l2)
    {
        return l1.lock()->getPriority() < l2.lock()->getPriority();
    }

    bool sortCompareTimer(const TimerRef& t1, const TimerRef& t2) { return t1->getPriority() < t2->getPriority(); }

    CustomEvent updateEvent{CustomEvent::updateEvent};
}// namespace


//////////////////////-Time Listener-/////////////////////////////

std::atomic_int Timer::timeAutoId{0};
int Timer::getNextId()
{
    return timeAutoId++;
}

TimerRef Timer::createUpdate(float delay, int64_t repeat)
{
    auto timer = new Timer();
    timer->id = Timer::getNextId();
    timer->delay = delay;
    timer->repeat = repeat;
    timer->interval = -1.0f;
    return TimerRef(timer);
}

TimerRef Timer::createFixUpdate(float interval, float delay, int64_t repeat)
{
    auto timer = new Timer();
    timer->id = Timer::getNextId();
    timer->delay = delay;
    timer->repeat = repeat;
    timer->interval = interval;
    return TimerRef(timer);
    ;
}

TimerRef Timer::createOnce(float delay)
{
    auto timer = new Timer();
    timer->id = Timer::getNextId();
    timer->delay = delay;
    timer->repeat = 1;
    timer->interval = -1.0f;
    return TimerRef(timer);
    ;
}

////////////////////--Event System--/////////////////////////////

EventSystem* EventSystem::get()
{
    static std::unique_ptr<EventSystem> system;
    if (system == nullptr)
    {
        system.reset(new EventSystem());
    }
    return system.get();
}

void EventSystem::update()
{
    dispatchCustomEvent(&updateEvent);
    updateEvent.isStopped = false;
    if (timersDirty)
    {
        timers.remove_if([](TimerRef& ref) { return ref == nullptr; });
        timers.sort(sortCompareTimer);
        timersDirty = false;
    }

    auto deltaTime = (float)std::min(TimeSys::getDelta(), maxFrameTime);
    for (auto& timer : timers)
    {
        if (timer == nullptr || timer->repeat <= 0 || timer->onTime == nullptr)
        {
            timer.reset();
            timersDirty = true;
            continue;
        }

        if (timer->delay - deltaTime > 0)
        {
            timer->delay -= deltaTime;
            continue;
        }

        if (timer->interval < 0.001f)
        {
            timer->delay = 0.0f;
            timer->onTime();
            timer->repeat--;
        }
        else
        {
            if (timer->delay > 0)
            {
                timer->intervalDuration = deltaTime - timer->delay;
                timer->delay = 0.0f;
            }
            else
            {
                timer->intervalDuration += deltaTime;
            }
            while (timer->intervalDuration >= timer->interval)
            {
                timer->onTime();
                timer->repeat--;
                timer->intervalDuration -= timer->interval;
            }
        }
    }
}

int EventSystem::timerUpdate(TimeCallBack call, float delay, int64_t repeat)
{
    auto listener = Timer::createUpdate(delay, repeat);
    listener->onTime = std::move(call);
    timers.push_back(listener);
    timersDirty = true;
    return listener->id;
}

int EventSystem::timerFixUpdate(TimeCallBack call, float interval, float delay, int64_t repeat)
{
    auto listener = Timer::createFixUpdate(interval, delay, repeat);
    listener->onTime = std::move(call);
    timers.push_back(listener);
    timersDirty = true;
    return listener->id;
}

int EventSystem::timerOnce(TimeCallBack call, float delay)
{
    auto listener = Timer::createOnce(delay);
    listener->onTime = std::move(call);
    timers.push_back(listener);
    timersDirty = true;
    return listener->id;
}

bool EventSystem::removeTimer(int id)
{
    auto it = std::find_if(timers.begin(), timers.end(), [&id](TimerRef& other) { return other->id == id; });
    if (it != timers.end())
    {
        timers.erase(it);
    }
    return false;
}

void EventSystem::subscribe(const ListenerRef& listener)
{
    if (listener->getIsRegistered())
    {
        std::cout << "Duplicate subscription listener\n";
        return;
    }

    EventType group = listener->getEventType();
    listenerGroups[group].emplace_back(listener);
    listenerGroupsDirty[group] = true;
    listener->setIsRegistered(true);
}

void EventSystem::unsubscribe(const ListenerRef& listener)
{

    EventType group = listener->getEventType();
    auto it = std::find_if(listenerGroups[group].begin(), listenerGroups[group].end(),
                           [&listener](const ListenerWeekRef& other) {
                               auto l = other.lock();
                               return l != nullptr && listener == l;
                           });
    if (it != listenerGroups[group].end())
    {
        listenerGroups[group].erase(it);
    }
    else
    {
        std::cout << "No subscription listener\n";
    }
    listener->setIsRegistered(false);
}

void EventSystem::dispatch(Event* event)
{
    if (event->getEventType() == EventType::kEventMouse)
    {
        auto mouseEvent = dynamic_cast<MouseEvent*>(event);
        dispatchMouseEvent(mouseEvent);
    }
    else if (event->getEventType() == EventType::kEventKeyboard)
    {
        auto keyEvent = dynamic_cast<KeyEvent*>(event);
        dispatchKeyEvent(keyEvent);
    }
    else if (event->getEventType() == EventType::kEventWindow)
    {
        auto windowEvent = dynamic_cast<WindowEvent*>(event);
        dispatchWindowEvent(windowEvent);
    }
    else if (event->getEventType() == EventType::kEventCustom)
    {
        auto customEvent = dynamic_cast<CustomEvent*>(event);
        dispatchCustomEvent(customEvent);
    }
    event->isStopped = false;
}

void EventSystem::sortListenerGroup(EventType group)
{
    auto& listeners = listenerGroups[group];
    listeners.remove_if([](ListenerWeekRef& ref) { return ref.expired(); });
    listeners.sort(sortCompareListener);
    listenerGroupsDirty[group] = false;
}

void EventSystem::dispatchMouseEvent(const MouseEvent* event)
{
    if (listenerGroupsDirty[EventType::kEventMouse])
    {
        sortListenerGroup(EventType::kEventMouse);
    }

    if(!this->listenerEnable[EventType::kEventMouse])
    {
        return;
    }

    for (auto& listener : listenerGroups[EventType::kEventMouse])
    {
        if (!listener.expired())
        {
            auto l = dynamic_cast<MouseListener*>(listener.lock().get());
            if (l->getIsEnabled() && l->onMouseEvent)
            {
                l->onMouseEvent(event);
                if (event->isStopped)
                {
                    break;
                }
            }
        }
        else
        {
            listenerGroupsDirty[kEventMouse] = true;
        }
    }
}

void EventSystem::dispatchKeyEvent(const KeyEvent* event)
{
    if (listenerGroupsDirty[EventType::kEventKeyboard])
    {
        sortListenerGroup(EventType::kEventKeyboard);
    }
    if(!this->listenerEnable[EventType::kEventKeyboard])
    {
        return;
    }
    for (auto& listener : listenerGroups[EventType::kEventKeyboard])
    {
        if (!listener.expired())
        {
            auto l = dynamic_cast<KeyListener*>(listener.lock().get());
            if (l->getIsEnabled() && l->onKey)
            {
                l->onKey(event);
                if (event->isStopped)
                {
                    break;
                }
            }
        }
        else
        {
            listenerGroupsDirty[kEventKeyboard] = true;
        }
    }
}
void EventSystem::dispatchWindowEvent(const WindowEvent* event)
{
    if (listenerGroupsDirty[EventType::kEventWindow])
    {
        sortListenerGroup(EventType::kEventWindow);
    }
    if(!this->listenerEnable[EventType::kEventWindow])
    {
        return;
    }

    for (auto& listener : listenerGroups[EventType::kEventWindow])
    {
        if (!listener.expired())
        {
            auto l = dynamic_cast<WindowEventListener*>(listener.lock().get());
            if (l->getIsEnabled() && l->onWindowEvent)
            {
                l->onWindowEvent(event);
                if (event->isStopped)
                {
                    break;
                }
            }
        }
        else
        {
            listenerGroupsDirty[EventType::kEventWindow] = true;
        }
    }
}

void EventSystem::dispatchCustomEvent(const CustomEvent* event)
{
    if (listenerGroupsDirty[EventType::kEventCustom])
    {
        sortListenerGroup(EventType::kEventCustom);
    }
    if(!this->listenerEnable[EventType::kEventCustom])
    {
        return;
    }

    for (auto& listener : listenerGroups[EventType::kEventCustom])
    {
        if (!listener.expired())
        {
            auto l = dynamic_cast<CustomEventListener*>(listener.lock().get());
            if (l->getIsEnabled() && event->eventName == l->getEventName() && l->onCustomEvent)
            {
                l->onCustomEvent(event);
                if (event->isStopped)
                {
                    break;
                }
            }
        }
        else
        {
            listenerGroupsDirty[EventType::kEventCustom] = true;
        }
    }
}

void EventSystem::dispatchCustomEvent(const std::string& eventName, void* userData)
{
    CustomEvent event{eventName, userData};
    dispatchCustomEvent(&event);
}


//////////////////////-Mouse Event-/////////////////////////////

void MouseEvent::setButton(MouseEvent::MouseButton button, MouseEvent::MouseEventType type)
{
    this->mouseButton = button;
    this->mouseEventType = type;
}

void MouseEvent::setScroll(float xOffset, float yOffset)
{
    this->mouseEventType = MouseEventType::kMouseScroll;
    this->scrollX = xOffset;
    this->scrollY = yOffset;
}

void MouseEvent::setMove(float x, float y)
{
    this->mouseButton =  MouseButton::kButtonNone;
    this->mouseEventType = MouseEventType::kMouseMove;
    this->posDelta.x = x - this->point.x;
    this->posDelta.y = y - this->point.y;
    this->point.x = x;
    this->point.y = y;
}

MouseListenerRef MouseListener::create()
{
    return MouseListenerRef(new MouseListener());
}


//////////////////////-Key Event-/////////////////////////////

KeyListenerRef KeyListener::create()
{
    return KeyListenerRef(new KeyListener());
}


//////////////////////-Window Event-/////////////////////////////

WindowEventListenerRef WindowEventListener::create()
{
    return WindowEventListenerRef(new WindowEventListener());
}


//////////////////////-Window Event-/////////////////////////////

CustomEventListenerRef CustomEventListener::create(const std::string& eventName)
{
    return CustomEventListenerRef(new CustomEventListener(eventName));
}
