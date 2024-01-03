#version 430

out vec4 FragColor;

in vec3 o_position;
in vec3 o_normal;
in vec2 o_texcoord;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float     shininess;
}; 

struct Light {
    vec4 position; // directional light if w = 0.
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	
    float constant;
    float linear;
    float quadratic;
};

uniform Material material;
uniform Light light;

uniform vec3 cameraPos;

vec3 getAmbient(vec3 light_color) {
    return light_color;
}

vec3 getDiffuse(vec3 N, vec3 L, vec3 light_color) {
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * light_color;
    return diffuse;
}

vec3 getSpecular(vec3 L, vec3 V, vec3 N, vec3 light_color, vec3 specular_color, float specular_exponent) {
    vec3 R = normalize(2 * max(dot(L, N), 0.) * N - L);
    return specular_color * pow(max(dot(R, V), 0.), specular_exponent) * light_color;
}

vec3 phong_BRDF(vec3 L, vec3 V, vec3 N, vec3 ambient_color, vec3 diffuse_color, vec3 specular_color, float specular_exponent)
{
    float k_a = 0.1;
    float k_d = 0.7;
    float k_s = 0.7;

    vec3 c_a = k_a * getAmbient(light.ambient) * ambient_color;
    vec3 c_d = k_d * getDiffuse(N, L, light.diffuse) * diffuse_color;
    vec3 c_s = k_s * getSpecular(L, V, N, light.specular, specular_color, specular_exponent);

    return c_a + c_d + c_s;
}

void main()
{
    vec3 ambient  = vec3(texture(material.diffuse, o_texcoord));
    vec3 diffuse  = ambient;  
    vec3 specular = vec3(texture(material.specular, o_texcoord));

	vec3 dir2camera = normalize(cameraPos - o_position);

    vec3 dir2light;
    if (light.position.w == 0) { // directional light
        dir2light = -normalize(light.position.xyz);
    } else { // point light
        dir2light = normalize(light.position.xyz - o_position);
    }

    vec3 phong_color = phong_BRDF(dir2light, dir2camera, o_normal, ambient, diffuse, specular, material.shininess);

    if (light.position[3] == 1.f) { // point light
        float distance    = length(light.position.xyz - o_position);
        float attenuation = 1.0 / (light.constant + light.linear * distance + 
    		    light.quadratic * (distance * distance));

        phong_color = phong_color * attenuation;
    }

    FragColor = vec4(phong_color, 1.f);
}