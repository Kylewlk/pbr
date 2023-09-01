#version 430 core

out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube environmentMap;

void main()
{
    FragColor = texture(environmentMap, WorldPos);
}
