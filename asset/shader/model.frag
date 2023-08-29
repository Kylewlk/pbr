#version 430 core

uniform vec3 lightColor;
uniform vec3 lightDir;
uniform vec3 cameraPos;

uniform vec3 albedo;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

out vec4 fragColor;

void main()
{
//    fragColor = vec4(0.0, 1.0, 1.0, 1.0);

    vec3 N = normalize(Normal);
    vec3 L = lightDir;
    vec3 V = normalize(cameraPos - WorldPos);
    vec3 H = normalize(L + V);

    float ambient = 0.1;

    float NdotL = dot(N, L);
    float diffuse = (NdotL + 1) * 0.5;

    float NdotH = max(dot(N, H), 0);
    float specular = pow(NdotH, 20) * 0.3;

    vec3 color = ((ambient + diffuse) * albedo + specular) * lightColor;

    fragColor = vec4(color, 1.0);
}