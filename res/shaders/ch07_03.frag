#version 430

out vec4 FragColor;

in vec3 LightingColor;

void main()
{
    FragColor = vec4(LightingColor, 1.f);
}