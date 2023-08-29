//
// Created by DELL on 2022/4/1.
//

#pragma once

#include "Camera.h"

class Camera3D : public Camera
{
public:
    ~Camera3D() override = default;
    Camera3D(const Camera3D&) = delete;
    Camera3D& operator=(const Camera3D&) = delete;

    static Camera3DRef create();

    void perspective(float fov, float aspect, float zNear, float zFar);
    void perspective(float fov, float width, float height, float zNear, float zFar);

    void update() override;
    void setFov(float f)
    {
        this->fov = f;
        this->isNeedUpdate = true;
    }
    [[nodiscard]] float getFov() const { return this->fov; }

    void setLockUp(bool lock) { this->lockUp = lock; }
    [[nodiscard]] bool getLockUp() const { return this->lockUp; }

    void updateViewSize(float width, float height);
    void resetView(const math::Vec3& center);
    void resetView();

    void forward(float distance);
    void round(float xOffset, float yOffset); // 绕着target转动.

private:
    Camera3D();


    float fov{60.0f};
    bool lockUp {true}; //round 时锁定Y轴， 不更新up向量这样可以保证 round水平旋转固定.
};

