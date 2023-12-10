#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;

uniform mat4 world2lightNDC;
uniform mat4 modelMatrix;
	
void main()
{
    gl_Position = world2lightNDC * modelMatrix * vec4(position, 1.0f);
}