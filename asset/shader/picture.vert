#version 430
layout(location= 1) uniform mat4 mvp;

layout(location= 0) in vec4 pos;
layout(location= 1) in vec2 texCoord;

out vec2 v_texCoord;

void main()
{
    gl_Position = mvp*pos;
    v_texCoord = texCoord;
}