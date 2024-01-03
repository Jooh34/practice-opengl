#version 430
out vec4 FragColor;

in vec2 o_texCoords;

uniform sampler2D texture1;

void main()
{             
    vec4 texColor = texture(texture1, o_texCoords);
    FragColor = texColor;
}