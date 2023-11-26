#version 430

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

out vec3 o_position;
out vec3 o_normal;
out vec3 dir2camera;
	
uniform vec3 cameraPos;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
	
void main()
{
	o_position = vec3(modelMatrix * vec4(position, 1.0f));
    o_normal = normalize(mat3(inverse(transpose(modelMatrix))) * normal);
	dir2camera = normalize(cameraPos - o_position);

    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0f);
}