//
// Created by DELL on 2022/4/1.
//

#include "Camera2D.h"

using namespace math;
using namespace glm;

Camera2D::Camera2D()
{
    this->lookAt({0, 0, 100}, {0, 0, 0});
    this->orgho(500, 500, -10000, 10000);
}

Camera2DRef Camera2D::create()
{
    return Camera2DRef(new Camera2D);
}

void Camera2D::orgho(float width, float height, float zNear, float zFar)
{
    this->setViewSize(width, height);
    this->setNearFar(zNear, zFar);
    this->viewScale = 1.0f;

    this->isNeedUpdate = true;
}

void Camera2D::update()
{
    if (!this->isNeedUpdate)
    {
        return;
    }

    float halfWidth = viewWidth * viewScale * 0.5f;
    float halfHeight = viewHeight * viewScale * 0.5f;
    this->proj = math::orgho(-halfWidth, halfWidth, -halfHeight, halfHeight, nearPlane, farPlane);

    this->view = math::lookAt(position, target, up);
    this->viewProj = proj * view;

    this->isNeedUpdate = false;
}

void Camera2D::setViewScale(float scale)
{
    if (!math::equal(scale, viewScale))
    {
        this->viewScale = scale;
        this->isNeedUpdate = true;
    }
}

void Camera2D::resetView(const math::Vec3& center)
{
    this->setViewPosition({center.x, center.y, center.z + 100});
    this->setViewTarget(center);
    this->setViewScale(1.0f);
}
