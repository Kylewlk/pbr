#version 430 core

// HDRI spherical map to cubemap

out vec4 FragColor;
in vec3 LocalPos;

uniform sampler2D sphericalMap;

// 0.1591
// 0.3183 = 1/pi = 1/ 3.1415926
const vec2 invAtan = vec2(0.1591, 0.3183);

// x轴
//
// y 轴
// sphericalMap 3.1415926 长，然后映射到[0, 1]之间


vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    uv.y = 1.0 - uv.y; // flip Y
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(LocalPos));
    vec3 color = texture(sphericalMap, uv).rgb;
    
    FragColor = vec4(color, 1.0);
}
