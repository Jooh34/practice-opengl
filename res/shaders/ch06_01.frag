#version 430

in vec3 o_position;
in vec2 o_texcoord;

layout(location = 0) out vec4 fragColor;

layout(binding = 0) uniform sampler2D tex_sampler;
uniform vec3 cam_pos;

void main()
{   
	vec3 finalColor = texture(tex_sampler, o_texcoord).xyz;
	fragColor = vec4(finalColor, 1.0f);
}