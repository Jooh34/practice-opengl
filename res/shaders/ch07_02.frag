#version 430

out vec4 FragColor;

in vec3 dir2camera;
in vec3 o_position;
in vec3 o_normal;

uniform vec3 objectColor;
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

vec3 getSpecular(vec3 L, vec3 V, vec3 N, vec3 light_color, vec3 specular_color, float specular_exponent) {
    vec3 R = normalize(2 * max(dot(L, N), 0.) * N - L);
    return specular_color * pow(max(dot(R, V), 0.), specular_exponent) * light_color;
}

vec3 phong_BRDF(vec3 L, vec3 V, vec3 N, vec3 diffuse_color, vec3 light_color, vec3 specular_color, float specular_exponent)
{
    float k_a = 0.1;
    float k_d = 0.7;
    float k_s = 0.7;


    vec3 c_a = k_a * getAmbient(light_color) * diffuse_color;
    vec3 c_d = k_d * getDiffuse(N, L, light_color) * diffuse_color;
    vec3 c_s = k_s * getSpecular(L, V, N, light_color, specular_color, specular_exponent);

    return c_a + c_d + c_s;
}

void main()
{
    vec3 specular_color = vec3(1.f, 1.f, 1.f);
    float specular_exponent = 32.f;

    vec3 dir2light = normalize(lightPos - o_position);

    vec3 phong_color = phong_BRDF(dir2light, dir2camera, o_normal, objectColor, lightColor, specular_color, specular_exponent);
    FragColor = vec4(phong_color, 1.f);
}