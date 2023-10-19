#version 430 core

// HDRI spherical map to cubemap

// atan
// https://en.wikipedia.org/wiki/Atan2#:~:text=In%20computing%20and%20mathematics%2C%20the,angle)%20of%20the%20complex%20number
// https://wikimedia.org/api/rest_v1/media/math/render/svg/d7104969b7dca1df7f408f75550b82731aef57dc

out vec4 FragColor;
in vec3 LocalPos;

uniform sampler2D sphericalMap;

const float pi = 3.1415926;
const vec2 invAtan = vec2(1.0/(2.0 * pi), 1.0/pi);
// 1/(2 * pi) = 0.1591
// 1/pi = 0.3183

// x轴
// sphericalMap 将宽设为 2*pi ，然后映射到[0, 1]之间

// y 轴
// sphericalMap 将高设为 pi ，然后映射到[0, 1]之间

vec2 SampleSphericalMap(vec3 v)
{
    // atan(v.z, v.x) 得到的是 XZ平面 整个圆， atan返回值范围(−π, π]
    // asin(v.y) 得到的是 XY平面 半圆，asin返回值范围[−π/2, π/2]
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    uv.y = 1.0 - uv.y; // flip Y
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(LocalPos));
//    vec3 color = textureLod(sphericalMap, uv, 0.0).rgb; // 不能使用mipmap,否则拼接处容易出现不同的lod, 导致有一条线
    vec3 color = texture(sphericalMap, uv).rgb;

    FragColor = vec4(color, 1.0);
}
