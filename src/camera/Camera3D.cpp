//
// Created by DELL on 2022/4/1.
//

#include "Camera3D.h"
#include <GLWindow.h>

Camera3D::Camera3D()
{
    const float height = GLWindow::get()->getHeight();
    const float defaultFov = 60.0f;
    const float zPosition = height * 0.5f / std::tan(glm::radians(defaultFov) * 0.5f);
    this->perspective(defaultFov, GLWindow::get()->getWidth(), height, 10.0f, 10000.0f);
    this->lookAt({0, 0, zPosition}, {0, 0, 0});
}

Camera3DRef Camera3D::create()
{
    return Camera3DRef(new Camera3D);
}

void Camera3D::perspective(float fov_, float aspect, float zNear, float zFar)
{
    this->perspective(fov_, aspect, 1.0f, zNear, zFar);
}

void Camera3D::perspective(float fov_, float width, float height, float zNear, float zFar)
{
    this->setViewSize(width, height);
    this->setNearFar(zNear, zFar);

    this->fov = glm::radians(fov_);
    this->isNeedUpdate = true;
}

void Camera3D::update()
{
    if (!this->isNeedUpdate)
    {
        return;
    }

    this->proj = math::perspective(fov, viewWidth/viewHeight, nearPlane, farPlane);
    this->view = math::lookAt(position, target, up);
    this->viewProj = proj * view;

    this->isNeedUpdate = false;
}

void Camera3D::resetView(const math::Vec3& center)
{
    this->setViewPosition({center.x, center.y, center.z + 100});
    this->setViewTarget(center);
}

void Camera3D::round(float xOffset, float yOffset)
{
    if ( abs(xOffset) > 0.1f || abs(yOffset) > 0.1f)
    {
        auto pos = position - target;
        if (!math::equal(xOffset, 0.0f))
        {
            auto matRotateY = math::rotate(xOffset * 0.02f, up);
            auto newPos = matRotateY * math::Vec4(pos, 1.0);
            this->position = math::Vec3(newPos) + target;
            isNeedUpdate = true;
        }

        if (!math::equal(yOffset, 0.0f))
        {
            math::Vec3 const zDir(normalize(position - target));
            math::Vec3 const xDir(normalize(cross(up, zDir)));
            math::Vec3 const yDir(cross(zDir, xDir));

            auto matRotateX = math::rotate(yOffset * 0.01f, xDir);
            math::Vec3 newPos = matRotateX * math::Vec4(pos, 1.0f);
            math::Vec3 newY = matRotateX * math::Vec4(yDir, 0.0f);

            if (lockUp)
            {
                if (glm::dot(glm::normalize(newY), up) > 0.1f)
                {
                    this->position = newPos + target;
                    isNeedUpdate = true;
                }
            }
            else
            {
                this->position = newPos + target;
                this->up = newY;
                isNeedUpdate = true;
            }
        }
    }
}
void Camera3D::forward(float distance)
{
    math::Vec3 dir = normalize(this->target - position);
    this->position += (dir * distance * this->updateSpeed);

    this->isNeedUpdate = true;
}

//void Camera3D::onMouseEvent(const MouseEvent* event)
//{
//    Camera::onMouseEvent(event);
//
//    if(event->mouseEventType == MouseEvent::kMouseMove)
//    {
//        if (holdLeftButton)
//        {
//            round(event->posDelta.x, -event->posDelta.y);
//        }
//        else if (holdRightButton)
//        {
//
//        }
//        else if (holdMidButton)
//        {
//            move({event->posDelta.x, -event->posDelta.y, 0.0f});
//        }
//    }
//    else if(event->mouseEventType == MouseEvent::kMouseScroll)
//    {
//        forward((float)event->scrollY*20.0f);
//    }
//}

void Camera3D::updateViewSize(float width, float height)
{
    if(math::equal(viewWidth, width) && math::equal(viewHeight, height))
    {
        return;
    }

    float zDistance = glm::distance(position, target);
    float zNewDistance = zDistance * height / viewHeight;
    math::Vec3 const zDir(normalize(position - target));
    this->position += (zDir * (zNewDistance - zDistance));

    this->viewWidth = width;
    this->viewHeight = height;
    this->isNeedUpdate = true;
}
