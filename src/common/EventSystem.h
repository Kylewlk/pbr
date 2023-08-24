//
// Created by Yun on 2021/11/13.
//
#pragma once
#include "Define.h"
#include "MathHelp.h"

#undef max
#undef min

enum EventType : int8_t
{
    kEventMouse = 0,
    kEventKeyboard,
    kEventWindow,
    kEventCustom,
    COUNT
};

class Event
{
public:
    virtual ~Event() = default;
    EventType getEventType() const { return type; }

    mutable bool isStopped{false};

protected:
    explicit Event(EventType type) : type(type) {}

private:
    EventType type;
};

class Listener
{
public:
    virtual ~Listener() = default;

    [[nodiscard]] EventType getEventType() const { return type; }
    [[nodiscard]] bool getIsRegistered() const { return isRegistered; };
    void setIsRegistered(bool registered) { this->isRegistered = registered; };
    [[nodiscard]] int getPriority() const { return this->priority; }
    void setPriority(int priority_) { this->priority = priority_; }
    [[nodiscard]] int getIsEnabled() const { return this->isEnabled; }
    void setIsEnabled(bool enabled) { this->isEnabled = enabled; }

protected:
    explicit Listener(EventType type) : type(type) {}

    int priority{0};
    bool isRegistered{false};
    bool isEnabled{true};

private:
    EventType type;
};

class Timer
{
public:
    constexpr static int64_t foreverTimes = std::numeric_limits<int64_t>::max();

    virtual ~Timer() = default;

    static TimerRef createUpdate(float delay = 0.0f, int64_t repeat = foreverTimes);
    static TimerRef createFixUpdate(float interval, float delay = 0.0f, int64_t repeat = foreverTimes);
    static TimerRef createOnce(float delay = 0.0f);

    [[nodiscard]] int getPriority() const { return this->priority; }

    TimeCallBack onTime;

private:
    friend class EventSystem;
    Timer() = default;
    static std::atomic_int timeAutoId;
    static int getNextId();

    int id{};
    int priority{0};
    int64_t repeat{foreverTimes};
    float delay{};
    float intervalDuration{};
    float interval{};
};

class EventSystem
{
public:
    EventSystem(const EventSystem&) = delete;
    EventSystem& operator=(const EventSystem&) = delete;
    ~EventSystem() = default;

    static EventSystem* get();

    void update();

    int timerUpdate(TimeCallBack call, float delay = 0.0f, int64_t repeat = Timer::foreverTimes);
    int timerFixUpdate(TimeCallBack call, float interval, float delay = 0.0f,
                      int64_t repeat = Timer::foreverTimes);
    int timerOnce(TimeCallBack call, float delay = 0.0f);
    bool removeTimer(int id);

    void subscribe(const ListenerRef& listener);
    void unsubscribe(const ListenerRef& listener);
    void dispatch(Event* event);
    void dispatchCustomEvent(const std::string& eventName, void* userData = nullptr);

    [[nodiscard]] bool eventIsEnable(EventType type) const { return this->listenerEnable[type]; }
    void setEventEnable(EventType type, bool enable) { this->listenerEnable[type] = enable; }

private:
    EventSystem() = default;
    void sortListenerGroup(EventType group);
    void dispatchMouseEvent(const MouseEvent* event);
    void dispatchKeyEvent(const KeyEvent* event);
    void dispatchWindowEvent(const WindowEvent* event);
    void dispatchCustomEvent(const CustomEvent* event);

    std::list<TimerRef> timers;
    bool timersDirty{false};

    std::list<ListenerWeekRef> listenerGroups[EventType::COUNT];
    bool listenerGroupsDirty[EventType::COUNT]{false};
    bool listenerEnable[EventType::COUNT]{true, true, true, true};

    double maxFrameTime{1.0f};
};

class MouseEvent : public Event
{
public:
    enum MouseEventType : int8_t
    {
        kEventNone = -1,
        kMouseRelease = 0,// GLFW_RELEASE
        kMousePress = 1,  // GLFW_PRESS
        kMouseMove = 2,
        kMouseScroll = 3,
    };

    enum MouseButton : int8_t
    {
        kButtonNone = -1,
        kButtonLeft = 0,  // GLFW_MOUSE_BUTTON_LEFT
        kButtonRight = 1, // GLFW_MOUSE_BUTTON_RIGHT
        kButtonMiddle = 2,// GLFW_MOUSE_BUTTON_MIDDLE
        kButton4 = 3,
        kButton5 = 4,
        kButton6 = 5,
        kButton7 = 6,
        kButton8 = 7
    };

    MouseEvent() : Event(EventType::kEventMouse) {}
    ~MouseEvent() override = default;

    void setButton(MouseButton button, MouseEventType type);
    void setScroll(float xOffset, float yOffset);
    void setMove(float x, float y);

    MouseEventType mouseEventType{MouseEventType::kEventNone};
    MouseButton mouseButton{MouseButton::kButtonNone};
    float scrollX{0};
    float scrollY{0};

    math::Vec2 point{};
    math::Vec2 posDelta{}; // 与上一个点相比，移动的距离
};

class MouseListener : public Listener
{
public:
    ~MouseListener() override = default;

    static std::shared_ptr<MouseListener> create();

    std::function<void(const MouseEvent* event)> onMouseEvent;

private:
    MouseListener() : Listener(EventType::kEventMouse) {}
};

class KeyEvent : public Event
{
public:
    enum KeyEventType : int8_t
    {
        kEventNone = -1,
        kKeyRelease = 0,// GLFW_RELEASE
        kKeyPress = 1,  // GLFW_PRESS
        kKeyRepeat = 2, // GLFW_REPEAT
    };

    enum Key : int16_t
    {
        kKeyNone = -1,// GLFW_KEY_UNKNOWN            -1

        kKeySpace = 32,       // GLFW_KEY_SPACE              32
        kkeyApostrophe = 39,  // GLFW_KEY_APOSTROPHE         39  /* ' */
        kKeyComma = 44,       // GLFW_KEY_COMMA              44  /* , */
        kKeyMinus = 45,       // GLFW_KEY_MINUS              45  /* - */
        kKeyPeriod = 46,      // GLFW_KEY_PERIOD             46  /* . */
        kKeySlash = 47,       // GLFW_KEY_SLASH              47  /* / */
        kKey0 = 48,           // GLFW_KEY_0                  48
        kKey1,                // GLFW_KEY_1                  49
        kKey2,                // GLFW_KEY_2                  50
        kKey3,                // GLFW_KEY_3                  51
        kKey4,                // GLFW_KEY_4                  52
        kKey5,                // GLFW_KEY_5                  53
        kKey6,                // GLFW_KEY_6                  54
        kKey7,                // GLFW_KEY_7                  55
        kKey8,                // GLFW_KEY_8                  56
        kKey9,                // GLFW_KEY_9                  57
        kKeySemicolon = 59,   // GLFW_KEY_SEMICOLON          59  /* ; */
        kKeyEqual = 61,       // GLFW_KEY_EQUAL              61  /* = */
        kKeyA = 65,           // GLFW_KEY_A                  65
        kKeyB,                // GLFW_KEY_B                  66
        kKeyC,                // GLFW_KEY_C                  67
        kKeyD,                // GLFW_KEY_D                  68
        kKeyE,                // GLFW_KEY_E                  69
        kKeyF,                // GLFW_KEY_F                  70
        kKeyG,                // GLFW_KEY_G                  71
        kKeyH,                // GLFW_KEY_H                  72
        kKeyI,                // GLFW_KEY_I                  73
        kKeyJ,                // GLFW_KEY_J                  74
        kKeyK,                // GLFW_KEY_K                  75
        kKeyL,                // GLFW_KEY_L                  76
        kKeyM,                // GLFW_KEY_M                  77
        kKeyN,                // GLFW_KEY_N                  78
        kKeyO,                // GLFW_KEY_O                  79
        kKeyP,                // GLFW_KEY_P                  80
        kKeyQ,                // GLFW_KEY_Q                  81
        kKeyR,                // GLFW_KEY_R                  82
        kKeyS,                // GLFW_KEY_S                  83
        kKeyT,                // GLFW_KEY_T                  84
        kKeyU,                // GLFW_KEY_U                  85
        kKeyV,                // GLFW_KEY_V                  86
        kKeyW,                // GLFW_KEY_W                  87
        kKeyX,                // GLFW_KEY_X                  88
        kKeyY,                // GLFW_KEY_Y                  89
        kKeyZ,                // GLFW_KEY_Z                  90
        kKeyLeftBracket = 91, // GLFW_KEY_LEFT_BRACKET       91  /* [ */
        kKeyBackslash = 92,   // GLFW_KEY_BACKSLASH          92  /* \ */
        kKeyRightBracket = 93,// GLFW_KEY_RIGHT_BRACKET      93  /* ] */
        kKeyGraveAccent = 96, // GLFW_KEY_GRAVE_ACCENT       96  /* ` */
        kKeyWorld1 = 161,     // GLFW_KEY_WORLD_1            161 /* non-US #1 */
        kKeyWorld2 = 162,     // GLFW_KEY_WORLD_2            162 /* non-US #2 */

        kKeyEscape = 256,   // GLFW_KEY_ESCAPE             256
        kKeyEnter = 257,    // GLFW_KEY_ENTER              257
        kKeyTab = 258,      // GLFW_KEY_TAB                258
        kKeyBackspace = 259,// GLFW_KEY_BACKSPACE          259
        kKeyInsert = 260,   // GLFW_KEY_INSERT             260
        kKeyDelete = 261,   // GLFW_KEY_DELETE             261
        kKeyRight = 262,    // GLFW_KEY_RIGHT              262
        kKeyLeft,           // GLFW_KEY_LEFT               263
        kKeyDown,           // GLFW_KEY_DOWN               264
        kKeyUp,             // GLFW_KEY_UP                 265
        kKeyPageUp,         // GLFW_KEY_PAGE_UP            266
        kKeyPageDown,       // GLFW_KEY_PAGE_DOWN          267
        kKeyHome,           // GLFW_KEY_HOME               268
        kKeyEnd,            // GLFW_KEY_END                269
        kKeyCapLock = 280,  // GLFW_KEY_CAPS_LOCK          280
        kKeyScrollLock,     // GLFW_KEY_SCROLL_LOCK        281
        kKeyNumLock,        // GLFW_KEY_NUM_LOCK           282
        kKeyScreen,         // GLFW_KEY_PRINT_SCREEN       283
        kKeyPause,          // GLFW_KEY_PAUSE              284
        kKeyF1 = 290,       // GLFW_KEY_F1                 290
        kKeyF2,             // GLFW_KEY_F2                 291
        kKeyF3,             // GLFW_KEY_F3                 292
        kKeyF4,             // GLFW_KEY_F4                 293
        kKeyF5,             // GLFW_KEY_F5                 294
        kKeyF6,             // GLFW_KEY_F6                 295
        kKeyF7,             // GLFW_KEY_F7                 296
        kKeyF8,             // GLFW_KEY_F8                 297
        kKeyF9,             // GLFW_KEY_F9                 298
        kKeyF10,            // GLFW_KEY_F10                299
        kKeyF11,            // GLFW_KEY_F11                300
        kKeyF12,            // GLFW_KEY_F12                301
        kKeyF13,            // GLFW_KEY_F13                302
        kKeyF14,            // GLFW_KEY_F14                303
        kKeyF15,            // GLFW_KEY_F15                304
        kKeyF16,            // GLFW_KEY_F16                305
        kKeyF17,            // GLFW_KEY_F17                306
        kKeyF18,            // GLFW_KEY_F18                307
        kKeyF19,            // GLFW_KEY_F19                308
        kKeyF20,            // GLFW_KEY_F20                309
        kKeyF21,            // GLFW_KEY_F21                310
        kKeyF22,            // GLFW_KEY_F22                311
        kKeyF23,            // GLFW_KEY_F23                312
        kKeyF24,            // GLFW_KEY_F24                313
        kKeyF25,            // GLFW_KEY_F25                314
        kKeyKp0 = 320,      // GLFW_KEY_KP_0               320
        kKeyKp1,            // GLFW_KEY_KP_1               321
        kKeyKp2,            // GLFW_KEY_KP_2               322
        kKeyKp3,            // GLFW_KEY_KP_3               323
        kKeyKp4,            // GLFW_KEY_KP_4               324
        kKeyKp5,            // GLFW_KEY_KP_5               325
        kKeyKp6,            // GLFW_KEY_KP_6               326
        kKeyKp7,            // GLFW_KEY_KP_7               327
        kKeyKp8,            // GLFW_KEY_KP_8               328
        kKeyKp9,            // GLFW_KEY_KP_9               329
        kKeyKpDecimal,      // GLFW_KEY_KP_DECIMAL         330
        kKeyKpDivide,       // GLFW_KEY_KP_DIVIDE          331
        kKeyKpMultiply,     // GLFW_KEY_KP_MULTIPLY        332
        kKeyKpSubtract,     // GLFW_KEY_KP_SUBTRACT        333
        kKeyKpAdd,          // GLFW_KEY_KP_ADD             334
        kKeyKpEnter,        // GLFW_KEY_KP_ENTER           335
        kKeyKpEqual,        // GLFW_KEY_KP_EQUAL           336
        kKeyLeftShift = 340,// GLFW_KEY_LEFT_SHIFT         340
        kKeyLeftControl,    // GLFW_KEY_LEFT_CONTROL       341
        kKeyLeftAlt,        // GLFW_KEY_LEFT_ALT           342
        kKeyLeftSuper,      // GLFW_KEY_LEFT_SUPER         343
        kKeyRightShift,     // GLFW_KEY_RIGHT_SHIFT        344
        kKeyRightControl,   // GLFW_KEY_RIGHT_CONTROL      345
        kKeyRightAlt,       // GLFW_KEY_RIGHT_ALT          346
        kKeyRightSuper,     // GLFW_KEY_RIGHT_SUPER        347
        kKeyMenu,           // GLFW_KEY_MENU               348
    };

    KeyEvent() : Event(EventType::kEventKeyboard) {}
    KeyEvent(Key key, KeyEventType eventType) : Event(EventType::kEventKeyboard), key(key), keyEventType(eventType) {}
    ~KeyEvent() override = default;

    Key key{Key::kKeyNone};
    KeyEventType keyEventType{KeyEventType::kEventNone};

    void setKey(Key key_, KeyEventType eventType_)
    {
        this->key = key_;
        this->keyEventType = eventType_;
    }
};

class KeyListener : public Listener
{
public:
    ~KeyListener() override = default;

    static std::shared_ptr<KeyListener> create();

    std::function<void(const KeyEvent* event)> onKey;

private:
    KeyListener() : Listener(EventType::kEventKeyboard) {}
};

class WindowEvent : public Event
{
public:
    enum WindowEventType : int8_t
    {
        kEventNone = -1,
        kWindowResize = 0,
        kWindowFocus,
        kWindowClose,
    };

    WindowEvent() : Event(EventType::kEventWindow) {}
    ~WindowEvent() override = default;

    WindowEventType eventType{WindowEventType::kEventNone};

    bool isFocus{};
    float width{};
    float height{};
};

class WindowEventListener : public Listener
{
public:
    ~WindowEventListener() override = default;

    static std::shared_ptr<WindowEventListener> create();

    std::function<void(const WindowEvent* event)> onWindowEvent;

private:
    WindowEventListener() : Listener(EventType::kEventWindow) {}
};


class CustomEvent : public Event
{
public:

    static constexpr const char* initSystemEvent = "__Init_System__";
    static constexpr const char* updateEvent = "__Update_Frame__";
    static constexpr const char* exitSystemEvent = "__Exit_System__";

    CustomEvent() : Event(EventType::kEventCustom) {}
    explicit CustomEvent(std::string eventName, void* userData = nullptr)
        : Event(EventType::kEventCustom), eventName(std::move(eventName)), userData(userData)
    {}
    ~CustomEvent() override = default;

    std::string eventName;
    void* userData{};
};

class CustomEventListener : public Listener
{
public:
    ~CustomEventListener() override = default;

    static std::shared_ptr<CustomEventListener> create(const std::string& eventName);

    std::function<void(const CustomEvent* event)> onCustomEvent;

    [[nodiscard]] const std::string& getEventName() const { return eventName; }

private:
    explicit CustomEventListener(std::string eventName)
        : Listener(EventType::kEventCustom), eventName(std::move(eventName))
    {}

    std::string eventName;
};
