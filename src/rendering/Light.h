/** 
 * Copyright (C) 2023 Jooh
 **/

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Light
{
public:
    Light(glm::vec4 position, glm::vec3 ambient = glm::vec3(1.f, 1.f, 1.f), glm::vec3 diffuse = glm::vec3(1.f, 1.f, 1.f), glm::vec3 specular = glm::vec3(1.f, 1.f, 1.f),
        float constant = 0.5f, float linear = 0.03f, float quadratic = 0.01f);
    
    glm::mat4 GetWorld2LightNDC() const;

    glm::vec4 position;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};