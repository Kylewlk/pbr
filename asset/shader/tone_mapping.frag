#version 430 core

layout(location = 0) uniform sampler2D tex;
layout(location = 1) uniform float exposure; // 曝光
in vec2 vTexCoord;

out vec4 fragColor;

vec3 Uncharted2Tonemap(highp vec3 x)
{
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

void main()
{
    vec3 color = texture(tex, vTexCoord).rgb;
    color *= exposure;

    // tone mapping
    color = Uncharted2Tonemap(color * 4.5f);
    color = color * (1.0f / Uncharted2Tonemap(vec3(11.2f)));

    // Gamma correct
    color = pow(color, vec3(1.0 / 2.2));

    fragColor = vec4(color, 1.0f);
}

