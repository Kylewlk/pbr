//
// Created by DELL on 2022/4/1.
//

#pragma once

#include "Camera.h"

class Camera2D : public Camera
{
public:
    ~Camera2D() override = default;
    Camera2D(const Camera2D&) = delete;
    Camera2D& operator=(const Camera2D&) = delete;

    static Camera2DRef create();

    void orgho(float width, float height, float zNear, float zFar);

    void update() override;
    void setViewScale(float scale);
    [[nodiscard]] float getViewScale() const { return this->viewScale; }
    void resetView(const math::Vec3& center = {0, 0, 0});

private:
    Camera2D();

    float viewScale{1.0f};
};
