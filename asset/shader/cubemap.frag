#version 430 core

uniform samplerCube cubeMap;

out vec4 FragColor;
in vec3 LocalPos;

void main()
{
    FragColor = texture(cubeMap, LocalPos);
}
