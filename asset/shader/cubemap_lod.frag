#version 430 core

uniform samplerCube cubeMap;
uniform float lod;

out vec4 FragColor;
in vec3 LocalPos;

void main()
{
    FragColor = textureLod(cubeMap, LocalPos, lod);
}
