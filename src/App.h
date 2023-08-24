//
// Created by DELL on 2022/3/3.
//
#pragma once
#include "Define.h"
#include "scene/Scene.hpp"
#include "common/EventSystem.h"
#include "common/MathHelp.h"
#include "GLWindow.h"

struct AppMenu
{
    const char* name;
    SceneRef (*function)();
};

class App
{
public:

    static App* get();

    [[nodiscard]] const ImFont* getFont() const { return this->defaultFont; }

    virtual void run();
    static void resetOpenGLStates();

    [[nodiscard]] math::Vec2 getMainScenePos() const { return {mainSceneX, mainSceneY}; }

    template<class T>
    void mainScene2Screen(T& p) const
    {
        p.x += static_cast<float>(mainSceneX);
        p.y += static_cast<float>(mainSceneY);
    }

    [[nodiscard]] ImFont* getDefaultFont() const { return this->defaultFont; }

protected:
    virtual void init();
    virtual void clean();
    virtual void render();
    virtual void showDockSpace();
    virtual void setupMouseEvent();

    int winWidth = 1350;
    int winHeight = 700;

    ImFont* defaultFont{};

    GLFWmousebuttonfun oldMouseButtonFun{nullptr};
    GLFWcursorposfun oldCursorPosFun{nullptr};
    GLFWscrollfun oldScrollFun{nullptr};
    bool openMenu{true};
    bool mainSceneActive{false};
    bool mainSceneHovered{false};
    int currentMenuIndex{-1};
    int mainSceneX{0}, mainSceneY{0};
    std::vector<bool> holdMouseButtons;

    SceneRef mainScene;
};
