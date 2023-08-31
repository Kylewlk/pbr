#version 430 core

layout (location = 0) in vec3 aPos;

out vec3 LocalPos;

uniform mat4 viewProj;

void main()
{
    LocalPos = aPos;
    gl_Position =  viewProj * vec4(LocalPos, 1.0);
}