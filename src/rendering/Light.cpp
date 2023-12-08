#include "Light.h"

Light::Light(glm::vec4 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic)
	: position(position), ambient(ambient), diffuse(diffuse), specular(specular), constant(constant), linear(linear), quadratic(quadratic)
{

}
