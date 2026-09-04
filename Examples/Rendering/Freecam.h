#pragma once

#include "Library/Vulcant/Interface/VulcantWindow.h"
#include "Library/Vulcant/Interface/VulcantInputValue.h"
#include "SceneData.h"

namespace Vulcant::Rendering
{
    class SceneData;

    class Freecam
    {
      public:
        Freecam(Vulcant::VulcantWindow& window, const glm::dvec3 eyeInput, const glm::dvec3 targetInput, const glm::dvec3 upInput);
        virtual ~Freecam();

        void tick(double delta);
        bool keyEvent(const Vulcant::VulcantInputValue&);

        glm::dvec3 getUp() const;
        void       setUp(const glm::dvec3 v);
        glm::dvec3 getEye() const;
        void       setEye(const glm::dvec3 v);
        glm::dvec3 getTarget() const;
        void       setTarget(const glm::dvec3 v);
        
        float      getZNear() const;
        void       setZNear(const float v);
        float      getZFar() const;
        void       setZFar(const float v);
        float      getAspect() const;
        void       setAspect(const float v);
        float      getFov() const;
        void       setFov(const float v);

        SceneData getScene() const;

      private:
        void updateScene();

        bool       active = false;
        glm::dvec2 lastMousePosition;

        glm::dvec3 up;
        glm::dvec3 target;
        glm::dvec3 eye;
        float      fovY   = glm::radians(75.0f);
        float      aspect = 16.0f / 9.0f;
        float      zNear  = 0.05f;
        float      zFar   = 100.0f;

        Vulcant::VulcantWindow& window;
    };
}