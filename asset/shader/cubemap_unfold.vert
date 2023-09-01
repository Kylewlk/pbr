#version 430 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aCubePos;

out vec3 CubePos;

uniform mat4 mvp;

void main()
{
    CubePos = aCubePos;
    gl_Position = mvp*vec4(aPos, 0.0, 1.0);
}