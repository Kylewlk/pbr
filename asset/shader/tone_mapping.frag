#version 430 core

layout(location = 0) uniform sampler2D tex;
layout(location = 1) uniform float exposure; // 曝光
in vec2 vTexCoord;

out vec4 fragColor;


vec3 LinearToneMapping( vec3 color ) {

    color = clamp(color, 0.0, 1.0);

    // Gamma correct
    color = pow(color, vec3(1.0 / 2.2));

    return color;
}

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

vec3 uncharted2ToneMapping(vec3 color)
{
    float white = 11.2;

    // tone mapping
    color = Uncharted2Tonemap(color *4.5);
    color = color * (1.0f / Uncharted2Tonemap(vec3(white)));

    // Gamma correct
    color = pow(color, vec3(1.0 / 2.2));

    return color;
}

vec3 whitePreservingLumaBasedReinhardToneMapping(vec3 color)
{
    float white = 2.;
    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    float toneMappedLuma = luma * (1. + luma / (white*white)) / (1. + luma);
    color *= toneMappedLuma / luma;

    // Gamma correct
    color = pow(color, vec3(1. / 2.2));

    return color;
}

vec3 filmicToneMapping(vec3 color)
{
    color = max(vec3(0.), color - vec3(0.004));
    color = (color * (6.2 * color + .5)) / (color * (6.2 * color + 1.7) + 0.06);
    return color;
}

vec3 acesToneMapping(vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;

    vec3 color = clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);

    // Gamma correct
    color = pow(color, vec3(1.0 / 2.2));

    return color;
}

void main()
{
    vec3 color = texture(tex, vTexCoord).rgb;
    color *= exposure;

    // tone mapping
    color = uncharted2ToneMapping(color);

    fragColor = vec4(color, 1.0f);
}

