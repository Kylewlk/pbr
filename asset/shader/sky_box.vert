#version 430 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 WorldPos;

void main()
{
    WorldPos = aPos;

    mat4 rotView = mat4(mat3(view)); // 去掉位移，将摄像机移动到圆点
    vec4 clipPos = projection * rotView * vec4(WorldPos, 1.0);

    gl_Position = clipPos.xyww; // 将 z 设为 1，防止深度测试遮挡其他绘制
}