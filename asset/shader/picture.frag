#version 430

layout(location = 2) uniform vec4 color;
layout(location = 3) uniform sampler2D tex;

in vec2 v_texCoord;

out vec4 fragColor;

void main()
{
//    	fragColor = vec4(0.0f, 1.0f, 1.0f, 1.0f);

    vec4 c = texture(tex, v_texCoord);
    c.rgb = pow(c.rgb, vec3(2.2)); // gamma, to linner

    fragColor = c * color;
    fragColor.rgb *= color.a; // premultiply alpha
}