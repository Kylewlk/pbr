//
// Created by Yun on 2021/11/13.
//

#include "GLWindow.h"
#include "common/Logger.h"


GLWindow* GLWindow::get()
{
    static std::unique_ptr<GLWindow> win;
    if (win == nullptr)
    {
        win.reset(new GLWindow());
    }
    return win.get();
}

void GLWindow::update()
{
    glfwPollEvents();
}

void GLWindow::setSize(int width, int height)
{
    this->size.x = (float)width;
    this->size.y = (float)height;
    glfwSetWindowSize(glfwWindow, (int)width, (int)height);
}

GLWindow::~GLWindow()
{
    // Free the windows callbacks and destroy the windows.
    glfwDestroyWindow(glfwWindow);

    // Terminate GLFW.
    glfwTerminate();
    glfwWindow = nullptr;
}

bool GLWindow::init(int width, int height, const std::string& pTitle)
{
    glfwSetErrorCallback([](int error, const char* description){
        LOGE("Glfw Error: {}, description:", error, description);
    });

    if (!glfwInit())
    {
        std::cerr<< "Glfw init failed!" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    glfwWindowHint(GLFW_SAMPLES, multisamplingCount);
    glfwWindowHint(GLFW_RESIZABLE, resizable);

    this->size.x = (float)width;
    this->size.y = (float)height;
    glfwWindow = glfwCreateWindow(width, height, pTitle.c_str(), nullptr, nullptr);
    if (nullptr == glfwWindow)
    {
        std::cerr << "Create GLFW windows failed" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(glfwWindow);

    if (!gladLoadGL())
    {
        std::cerr << "Glew initialize failed!" << std::endl;
        glfwTerminate();
        return false;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(GLWindow::debugOutputCallback, this);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION , 0, nullptr, GL_FALSE);

    glfwSetWindowSizeLimits(glfwWindow, 200, 200, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSwapInterval(1);

    glfwSetWindowUserPointer(glfwWindow, this);
    glfwSetKeyCallback(glfwWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods){
        auto win = reinterpret_cast<GLWindow*>(glfwGetWindowUserPointer(window));
        win->keyEvent.setKey((KeyEvent::Key)key, (KeyEvent::KeyEventType)action);
        EventSystem::get()->dispatch(&win->keyEvent);
    });
    glfwSetCursorPosCallback(glfwWindow, [](GLFWwindow* window, double xPos, double yPos){
        auto win = reinterpret_cast<GLWindow*>(glfwGetWindowUserPointer(window));
        win->mouseEvent.setMove((float)xPos, (float)yPos);
        EventSystem::get()->dispatch(&win->mouseEvent);
    });
    glfwSetMouseButtonCallback(glfwWindow, [](GLFWwindow* window, int button, int action, int mods){
        auto win = reinterpret_cast<GLWindow*>(glfwGetWindowUserPointer(window));
        win->mouseEvent.setButton((MouseEvent::MouseButton)button, (MouseEvent::MouseEventType)action);
        EventSystem::get()->dispatch(&win->mouseEvent);
    });
    glfwSetScrollCallback(glfwWindow, [](GLFWwindow* window, double xOffset, double yOffset){
        auto win = reinterpret_cast<GLWindow*>(glfwGetWindowUserPointer(window));
        win->mouseEvent.setScroll((float)xOffset, (float)yOffset);
        EventSystem::get()->dispatch(&win->mouseEvent);
    });

    glfwSetFramebufferSizeCallback(glfwWindow, [](GLFWwindow* window, int width, int height){
        auto win = reinterpret_cast<GLWindow*>(glfwGetWindowUserPointer(window));
        win->size.x = (float)width;
        win->size.y = (float)height;
        glViewport(0, 0, width, height);
        win->windowEvent.eventType = WindowEvent::kWindowResize;
        win->windowEvent.width = win->size.x;
        win->windowEvent.height = win->size.y;
        EventSystem::get()->dispatch(&win->windowEvent);
    });

    glfwSetWindowFocusCallback(glfwWindow, [](GLFWwindow* window, int focused){
        auto win = reinterpret_cast<GLWindow*>(glfwGetWindowUserPointer(window));
        win->windowEvent.eventType = WindowEvent::kWindowFocus;
        win->windowEvent.isFocus = focused == GLFW_TRUE;
        EventSystem::get()->dispatch(&win->windowEvent);
    });

    glfwSetWindowCloseCallback(glfwWindow, [](GLFWwindow* window){
        auto win = reinterpret_cast<GLWindow*>(glfwGetWindowUserPointer(window));
        win->windowEvent.eventType = WindowEvent::kWindowClose;
        EventSystem::get()->dispatch(&win->windowEvent);
    });

    EventSystem::get()->dispatchCustomEvent(CustomEvent::initSystemEvent);
    return true;
}


void APIENTRY GLWindow::debugOutputCallback(GLenum source, GLenum type, unsigned int id, GLenum severity,
                                              GLsizei length, const char *message, const void *userParam)
{
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204 || id == 8)
    {
        return;
    }

    const char* srcStr = "";
    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             srcStr = "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   srcStr = "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: srcStr = "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     srcStr = "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     srcStr = "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           srcStr = "Source: Other"; break;
    default: srcStr = "";
    }

    const char* typeStr = "";
    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               typeStr = "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr = "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeStr = "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         typeStr = "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         typeStr = "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              typeStr = "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          typeStr = "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           typeStr = "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               typeStr = "Type: Other"; break;
    default: typeStr = "";
    }

    const char* severityStr = "";
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         severityStr = "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       severityStr = "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          severityStr = "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: severityStr = "Severity: notification"; break;
    default: severityStr = "";
    }

    LOGE("Debug message (0x{:x}): {}, {}, {}, {}", id, message, srcStr, typeStr, severityStr);
}
