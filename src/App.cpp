//
// Created by DELL on 2022/3/3.
//

#include "App.h"
#include "common/ChineseSimplifiedUnicode.h"
#include "common/TimeSys.h"
#include "common/Texture.h"
#include "GLWindow.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "scene/PictureScene.hpp"
#include "scene/ModelScene.hpp"
#include "scene/01PbrLightScene.h"

#define ADD_SCENE_MENU(scene) {scene::ID, &scene::create}

AppMenu mainMenus[]{
    ADD_SCENE_MENU(PictureScene),
    ADD_SCENE_MENU(ModelScene),
    ADD_SCENE_MENU(PbrLightScene),

};
int mainMenuCount = sizeof(mainMenus)/(sizeof (mainMenus[0]));

#undef ADD_SCENE_MENU

//extern AppMenu mainMenus[];
//extern int mainMenuCount;

void App::run()
{
    auto win = GLWindow::get();
    win->init(winWidth, winHeight, "Ray Tracing");

    std::string titleFormat = "Ray Tracing, Graphic-FPS: ";
    std::string title;
    int32_t frameCount = 0;
    int32_t fps = 0;
    double timeDuration{0.0f};

    this->setupMouseEvent();
    this->init();
    ImGuiIO& io = ImGui::GetIO();

    while (!win->shouldClose())
    {
        TimeSys::update();
        win->update();
        EventSystem::get()->update();
        EventSystem::get()->setEventEnable(kEventKeyboard, mainSceneActive || !io.WantCaptureKeyboard);
        //        EventSystem::get()->setEventEnable(kEventMouse, !io.WantCaptureMouse);

        this->render();

        ++frameCount;
        timeDuration += TimeSys::getDelta();
        if (timeDuration > 1.0f)
        {
            fps = static_cast<int32_t>(std::round(static_cast<float>(frameCount)/timeDuration));
            frameCount = 0;
            timeDuration = 0.0f;
            title = titleFormat + std::to_string(fps);
            win->setTitle(title.c_str());
        }

        win->SwapBuffers();
    }

    EventSystem::get()->dispatchCustomEvent(CustomEvent::exitSystemEvent);
    this->clean();
}

App* App::get()
{
    static App app;
    return &app;
}


void App::setupMouseEvent()
{
    auto win = GLWindow::get();
    this->holdMouseButtons.resize(32, false);

    oldMouseButtonFun = glfwSetMouseButtonCallback(win->getGlfwWindow(), [](GLFWwindow* window, int button, int action, int mods){
        auto app = App::get();
        if (app->oldMouseButtonFun == nullptr)
        {
            return ;
        }
        if (action == GLFW_PRESS)
        {
            if (app->mainSceneHovered)
            {
                app->oldMouseButtonFun(window, button, action, mods);
                app->holdMouseButtons[button] = true;
            }
        }
        else
        {
            if(app->mainSceneHovered || app->holdMouseButtons[button])
            {
                app->oldMouseButtonFun(window, button, action, mods);
            }
            app->holdMouseButtons[button] = false;
        }
    });
    oldCursorPosFun = glfwSetCursorPosCallback(win->getGlfwWindow(), [](GLFWwindow* window, double xPos, double yPos){
        auto app = App::get();
        if (app->oldCursorPosFun != nullptr)
        {
            xPos -= app->mainSceneX;
            yPos -= app->mainSceneY;
            app->oldCursorPosFun(window, xPos, yPos);
        }
    });

    oldScrollFun = glfwSetScrollCallback(win->getGlfwWindow(), [](GLFWwindow* window, double xOffset, double yOffset){
        auto app = App::get();
        if (app->mainSceneHovered)
        {
            app->oldScrollFun(window, xOffset, yOffset);
        }
    });
}

void App::init()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //    ImGui::StyleColorsClassic();
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = ".data/imgui.ini";
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    this->defaultFont = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/msyh.ttc", // 微软雅黑
                                                     18, nullptr, ChineseSimplifiedUnicode::getImguiCharRange());

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(GLWindow::get()->getGlfwWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 430");

    EventSystem::get()->dispatchCustomEvent(CustomEvent::initSystemEvent);

    this->currentMenuIndex = mainMenuCount - 1;
    this->mainScene = mainMenus[currentMenuIndex].function();
}

void App::clean()
{
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    this->mainScene.reset();
}

void App::render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    const auto& winSize = GLWindow::get()->getSize();
    if (winSize.x < 2 || winSize.y < 2)
    {
        return;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    this->showDockSpace();

     ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.13f, 0.17f, 0.20f, 1.00f));
    if(ImGui::Begin("MainMenu", &openMenu, ImGuiWindowFlags_NoCollapse))
    {
        for (int i = 0; i < mainMenuCount; ++i)
        {
            if (ImGui::MenuItem(mainMenus[i].name, nullptr, i == this->currentMenuIndex))
            {
                this->mainScene = mainMenus[i].function();
                this->currentMenuIndex = i;
                App::resetOpenGLStates();
            }
        }
    }
    ImGui::End();
    ImGui::PopStyleColor();

    if (mainScene != nullptr)
    {
        ImGui::Begin("MainScene");

        auto originPosition = ImGui::GetCursorScreenPos();
        auto contentSize = ImGui::GetContentRegionAvail();
        this->mainSceneX = (int)originPosition.x;
        this->mainSceneY = (int)originPosition.y;
        this->mainSceneActive = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
        this->mainSceneHovered = ImGui::IsWindowHovered();

        mainScene->resize((int)contentSize.x, (int)contentSize.y);
        mainScene->render();
        auto colorTexture = mainScene->getColorTexture();
        if (colorTexture != nullptr)
        {
            auto drawList = ImGui::GetWindowDrawList();
            drawList->AddImage((ImTextureID*)(int64_t)colorTexture->getHandle(),
                               {originPosition.x, originPosition.y + contentSize.y},
                               {originPosition.x + contentSize.x, originPosition.y});
        }

        ImGui::End();
    }

    ImGui::Render(); // Rendering
    glViewport(0, 0, (int)winSize.x, (int)winSize.y);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void App::showDockSpace()
{
    const auto& winSize = GLWindow::get()->getSize();
    bool openDockSpace = true;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", &openDockSpace, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockSpaceId = ImGui::GetID("MainDockSpace");
    if (ImGui::DockBuilderGetNode(dockSpaceId) == nullptr)
    {
        ImGui::DockBuilderRemoveNode(dockSpaceId);

        ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodePos(dockSpaceId,ImVec2(viewport->WorkPos.x, viewport->WorkPos.y));
        ImGui::DockBuilderSetNodeSize(dockSpaceId, {winSize.x, winSize.y});

        ImGuiID center = dockSpaceId;
        ImGuiID right;
        ImGuiID left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.2f, nullptr, &right);

        ImGuiDockNode* rightNode = ImGui::DockBuilderGetNode(right);
        rightNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoDocking;

        ImGuiDockNode* leftNode = ImGui::DockBuilderGetNode(left);
        leftNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoDocking;

        ImGui::DockBuilderDockWindow("MainMenu", left);
        ImGui::DockBuilderDockWindow("MainScene", right);
        ImGui::DockBuilderFinish(dockSpaceId);
    }

    ImGuiDockNodeFlags dockSpaceFlags = ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode;
    ImGui::DockSpace(dockSpaceId, ImVec2(0.0f, 0.0f), dockSpaceFlags);

    ImGui::End();
}

void App::resetOpenGLStates()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    const auto& winSize = GLWindow::get()->getSize();
    glScissor(0, 0, (int)winSize.x, (int)winSize.y);
    glClearDepth(1.0);
    glPointSize(1.0f);

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDisable(GL_MULTISAMPLE);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_LINE_SMOOTH);
    glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
