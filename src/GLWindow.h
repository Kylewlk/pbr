//
// Created by Yun on 2021/11/13.
//
#pragma once

#include "Define.h"
#include "common/MathHelp.h"
#include "common/EventSystem.h"

class GLWindow;

class GLWindow
{
public:
    ~GLWindow();
    GLWindow(const GLWindow&) = delete;
    GLWindow& operator=(const GLWindow&) = delete;

    bool init(int width, int height, const std::string& pTitle = "");
    static GLWindow* get();

    void update();

    const math::Vec2& getSize() const { return size; }

    float getAspectRatio() const { return size.x/ size.y; }

    float getWidth() const { return size.x; }
    float getHeight() const { return size.y; }
    GLFWwindow* getGlfwWindow() const { return glfwWindow; }
//    HWND getHwnd() const { return glfwGetWin32Window(glfwWindow); }
    bool shouldClose() const { return glfwWindowShouldClose(glfwWindow); }

    void setTitle(const char* title) { glfwSetWindowTitle(glfwWindow, title); }
    void setSize(int width, int height);
    void SwapBuffers() {glfwSwapBuffers(glfwWindow);}

    int multisamplingCount {4};
    bool resizable{true};
private:
    GLWindow() = default;
    static void APIENTRY debugOutputCallback(GLenum source, GLenum type, unsigned int id, GLenum severity,
                                             GLsizei length, const char *message, const void *userParam);

    math::Vec2 size{};
    GLFWwindow* glfwWindow = nullptr;

    MouseEvent mouseEvent;
    KeyEvent keyEvent;
    WindowEvent windowEvent;
};
