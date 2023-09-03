#version 430 core

uniform samplerCube cubeMap;
uniform float lod;

out vec4 FragColor;
in vec3 CubePos;

void main()
{
    FragColor = textureLod(cubeMap, CubePos, lod);
}
