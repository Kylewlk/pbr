#version 430
layout(location= 1) uniform mat4 mvp;

layout(location= 0) in vec4 pos;

void main()
{
    gl_Position = mvp*pos;
}
