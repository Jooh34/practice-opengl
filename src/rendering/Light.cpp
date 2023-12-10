#include "Light.h"
#include <glm/glm.hpp>

Light::Light(glm::vec4 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic)
	: position(position), ambient(ambient), diffuse(diffuse), specular(specular), constant(constant), linear(linear), quadratic(quadratic)
{

}

glm::mat4 Light::GetWorld2LightNDC() const
{
	glm::mat4 lightProjection;
	glm::mat4 lightView;
	
	float near_plane = 1.0f, far_plane = 10.f;
	lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

	glm::vec3 pos = glm::vec3(position);
	lightView = glm::lookAt(pos, glm::vec3(0.f), glm::vec3(0.0, 1.0, 0.0));


	return lightProjection * lightView;
}
