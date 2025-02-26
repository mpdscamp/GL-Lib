#version 460 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D texture1;
uniform mat3 u_homography;

void main()
{
    vec3 tc = u_homography * vec3(TexCoord, 1.0);
    vec2 newTexCoord = tc.xy / tc.z;
    FragColor = texture(texture1, newTexCoord);
}
