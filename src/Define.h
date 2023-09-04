//
// Created by wlk12 on 2023/8/6.
//
#pragma once

#include <iostream>
#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <functional>
#include <filesystem>
#include <numeric>
#include <utility>
#include <atomic>
#include <condition_variable>
#include <thread>


#ifndef NOMINMAX
#define NOMINMAX 1// prevent windows redefining min/max
#endif
#include <Windows.h>
//#undef far
//#undef near

#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
//#define GLFW_EXPOSE_NATIVE_WIN32
//#include <GLFW/glfw3native.h>

#include <imgui.h>

#include <nlohmann.h>
using Json = nlohmann::json;

#include <tiny_obj_loader.h>

#include <stb/stb_image.h>
#include <stb/stb_image_resize.h>
#include <stb/stb_image_write.h>

class Listener;
using ListenerRef = std::shared_ptr<Listener>;
using ListenerWeekRef = std::weak_ptr<Listener>;

class Timer;
using TimerRef = std::shared_ptr<Timer>;
using TimeCallBack = std::function<void()>;

class MouseEvent;
class MouseListener;
using MouseListenerRef = std::shared_ptr<MouseListener>;

class KeyEvent;
class KeyListener;
using KeyListenerRef = std::shared_ptr<KeyListener>;

class WindowEvent;
class WindowEventListener;
using WindowEventListenerRef = std::shared_ptr<WindowEventListener>;

class CustomEvent;
class CustomEventListener;
using CustomEventListenerRef = std::shared_ptr<CustomEventListener>;

class Texture;
using TextureRef = std::shared_ptr<Texture>;

class RenderTarget;
using RenderTargetRef = std::shared_ptr<RenderTarget>;

class FrameBuffer;
using FrameBufferRef = std::shared_ptr<FrameBuffer>;

class Shader;
using ShaderRef = std::shared_ptr<Shader>;

class Model;
using ModelRef = std::shared_ptr<Model>;

class Scene;
using SceneRef = std::shared_ptr<Scene>;

class Camera2D;
using Camera2DRef = std::shared_ptr<Camera2D>;

class Camera3D;
using Camera3DRef = std::shared_ptr<Camera3D>;


