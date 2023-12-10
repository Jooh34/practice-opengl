#version 430

out vec4 FragColor;

in vec3 dir2camera;
in vec3 o_position;
in vec3 o_normal;
in vec2 o_texcoord;
in vec4 o_position_in_light_space;

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

uniform sampler2D shadowMap;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 N, vec3 L)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    vec2 shadow_uv = projCoords.xy;
    float closestDepth = texture(shadowMap, shadow_uv).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float bias = max(0.05 * (1.0 - dot(N, L)), 0.005);

    float shadow = 0.0;

    if (projCoords.z > 1.0) { // out of shadow map
        return 0.0;
    }
    
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    int num_pcf = 3;
    for(int x = -num_pcf/2; x <= num_pcf/2; ++x)
    {
        for(int y = -num_pcf/2; y <= num_pcf/2; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= (num_pcf * num_pcf);
    // float shadow = (currentDepth - closestDepth > bias) ? 1.f : 0.f;

    return shadow;
}

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

    float shadow = ShadowCalculation(o_position_in_light_space, N, L);

    vec3 c_a = k_a * getAmbient(light.ambient) * ambient_color;
    vec3 c_d = k_d * getDiffuse(N, L, light.diffuse) * diffuse_color;
    vec3 c_s = k_s * getSpecular(L, V, N, light.specular, specular_color, specular_exponent);

    return c_a + (1.0 - shadow) * (c_d + c_s);
}

void main()
{
    vec3 ambient  = vec3(texture(material.diffuse, o_texcoord));
    vec3 diffuse  = ambient;  
    vec3 specular = vec3(texture(material.specular, o_texcoord));

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