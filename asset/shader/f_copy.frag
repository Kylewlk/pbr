#version 430

layout(location = 1) uniform sampler2D tex;

out vec4 fragColor;

void main()
{
    fragColor = texelFetch(tex, ivec2(gl_FragCoord.x, gl_FragCoord.y), 0);
}