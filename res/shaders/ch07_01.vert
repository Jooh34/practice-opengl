#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 texcoord;

out vec3 o_position;
out vec2 o_texcoord;
	
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
	
void main()
{
	o_position = vec3(modelMatrix * vec4(position, 1.0f));
    o_texcoord = texcoord.xy;
	
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
}