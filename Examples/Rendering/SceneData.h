#pragma once

#include <glm/glm.hpp>

namespace Vulcant::Rendering
{
    struct SceneData
    {
        glm::mat4 viewMatrix;          // World to View
        glm::mat4 projectionMatrix;    // View to Clip
        glm::mat4 invProjectionMatrix; // Clip to View (Crucial for Raycasting)
        glm::mat4 invViewMatrix;       // View to World (Camera Transform)

        glm::vec4 cameraPos; // Ray Origin (w can be 1.0)

        glm::vec2 resolution; // Viewport width/height
        float     nearPlane;
        float     farPlane;
    };
}