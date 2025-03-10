#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aOffset;
layout (location = 3) in vec3 aScale;
layout (location = 4) in vec3 aColor;

out vec2 TexCoords;
out vec3 Color;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    // Apply instance scaling
    vec3 scaledPos = aPos * aScale;
    
    // Transform to world space
    vec3 worldPos = scaledPos + aOffset;
    
    // Apply camera transformations
    gl_Position = projection * view * vec4(worldPos, 1.0);
    
    // Pass through texture coordinates and color
    TexCoords = aTexCoords;
    Color = aColor;
}