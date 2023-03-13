#pragma once

#include <glm/mat4x4.hpp>

struct uniform_buffer_object
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

