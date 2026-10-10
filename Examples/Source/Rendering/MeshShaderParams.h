#pragma once

#include <glm/glm.hpp>

namespace Vulcant::Rendering
{
    struct MeshShaderParams
    {
        glm::vec4 wireframeColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.5f);
        int       wireframeMode  = 0;
        glm::vec3 padding        = glm::vec3(0.0f);
    };
}
