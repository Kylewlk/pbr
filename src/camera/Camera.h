//
// Created by Kyle on 2021/11/12.
//
#pragma once
#include "common/MathHelp.h"
#include "Define.h"

class Camera
{
public:
    virtual ~Camera() = default;
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    virtual void lookAt(const math::Vec3& eye, const math::Vec3& center, const math::Vec3&up);
    virtual void lookAt(const math::Vec3& eye, const math::Vec3& center);

    virtual void setViewSize(float width, float height);
    virtual void setNearFar(float zNear, float zFar);
    virtual void move(const math::Vec3& step);
    virtual void setViewPosition(const math::Vec3& pos);
    virtual void setViewTarget(const math::Vec3& target);
    virtual void setViewUp(const math::Vec3& up);

    virtual void update() = 0;

    [[nodiscard]] const math::Mat4& getView() const { return this->view; }
    [[nodiscard]] const math::Mat4& getProj() const { return this->proj; }
    [[nodiscard]] const math::Mat4& getViewProj() const { return this->viewProj; }

    [[nodiscard]] const math::Vec3& getViewPosition() const { return this->position; }
    [[nodiscard]] const math::Vec3& getViewTarget() const { return this->target; }
    [[nodiscard]] const math::Vec3& getViewUp() const { return this->up; }

    [[nodiscard]] math::Vec2 getViewSize() const { return {viewWidth, viewHeight}; }
    [[nodiscard]] float getFarPanel() const { return this->farPlane; }
    [[nodiscard]] float getNearPanel() const { return this->nearPlane; }

    float updateSpeed{1.0f};

protected:
    Camera() = default;

    bool isNeedUpdate {false};

    float viewWidth{800.0f};
    float viewHeight{600.0f};
    float nearPlane{-1000.0f};
    float farPlane{1000.0f};

    // view transform
    math::Vec3 position {};
    math::Vec3 target {};
    math::Vec3 up {math::Y_Axis};

    math::Mat4 view{};
    math::Mat4 proj{};
    math::Mat4 viewProj{};
};


