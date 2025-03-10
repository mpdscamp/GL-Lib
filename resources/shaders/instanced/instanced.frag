#version 460 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Color;

uniform sampler2D texture1;

void main()
{    
    // Sample texture and blend with instance color
    vec4 texColor = texture(texture1, TexCoords);
    FragColor = texColor * vec4(Color, 1.0);
}