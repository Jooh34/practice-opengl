#version 430

out vec4 FragColor;

in vec3 dir2camera;
in vec3 o_position;
in vec3 o_normal;
in vec2 o_texcoord;

struct Material {
    vec3 specular;
    float shininess;
}; 
  
uniform sampler2D diffuse;

uniform int blinn;
uniform Material material;

uniform vec3 cameraPos;
uniform vec3 lightColor;
uniform vec3 lightPos;

vec3 getAmbient(vec3 light_color) {
    return light_color;
}

vec3 getDiffuse(vec3 N, vec3 L, vec3 light_color) {
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * light_color;
    return diffuse;
}

vec3 getSpecular(vec3 L, vec3 V, vec3 N, vec3 light_color, float specular_exponent) {
    //vec3 R = reflect(-L, N);
    vec3 R = normalize(2 * max(dot(L, N), 0.) * N - L);
    return pow(max(dot(R, V), 0.), specular_exponent) * light_color;
}

vec3 getSpecularBlinn(vec3 L, vec3 V, vec3 N, vec3 light_color, float specular_exponent) {
    vec3 H = normalize(L + V);
    return pow(max(dot(N, H), 0.0), specular_exponent) * light_color;
}

vec3 phong_BRDF(vec3 L, vec3 V, vec3 N, vec3 light_color, vec3 ambient_color, vec3 diffuse_color, vec3 specular_color, float specular_exponent)
{
    float k_a = 0.1;
    float k_d = 0.7;
    float k_s = 0.7;


    vec3 c_a = k_a * getAmbient(light_color) * ambient_color;
    vec3 c_d = k_d * getDiffuse(N, L, light_color) * diffuse_color;
    vec3 c_s;
    if (blinn == 1)
    {
        c_s = k_s * getSpecularBlinn(L, V, N, light_color, specular_exponent) * specular_color;
    }
    else
    {
        c_s = k_s * getSpecular(L, V, N, light_color, specular_exponent) * specular_color;
    }

    return c_a + c_d + c_s;
}

void main()
{
    vec3 dir2light = normalize(lightPos - o_position);
    vec3 dir2camera = normalize(cameraPos - o_position);
    vec3 diffuse_color = vec3(texture(diffuse, o_texcoord));
    vec3 ambient_color = diffuse_color;

    vec3 phong_color = phong_BRDF(dir2light, dir2camera, o_normal, lightColor, ambient_color, diffuse_color, material.specular, material.shininess);
    float distance = length(lightPos - o_position);

    float atten_constant = 0.5f; 
    float atten_linear = 0.03f; 
    float atten_quadratic = 0.01f; 

    float attenuation = 1.0 / (atten_constant + atten_linear * distance + 
    		    atten_quadratic * (distance * distance));

    FragColor = vec4(phong_color*attenuation, 1.f);
}