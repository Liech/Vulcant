#pragma once

#include "Vulcant/Interface/VulcantInputValue.h"
#include "SceneData.h"
#include <glm/glm.hpp>

namespace Vulcant::Rendering
{
    class Camera
    {
      public:
        virtual ~Camera() = default;

        virtual void tick(double delta) = 0;
        virtual bool keyEvent(const Vulcant::VulcantInputValue& key) = 0;
        virtual SceneData getScene() const = 0;

        virtual glm::dvec3 getUp() const = 0;
        virtual void       setUp(const glm::dvec3 v) = 0;
        virtual glm::dvec3 getEye() const = 0;
        virtual void       setEye(const glm::dvec3 v) = 0;
        virtual glm::dvec3 getTarget() const = 0;
        virtual void       setTarget(const glm::dvec3 v) = 0;

        virtual float getZNear() const = 0;
        virtual void  setZNear(const float v) = 0;
        virtual float getZFar() const = 0;
        virtual void  setZFar(const float v) = 0;
        virtual float getAspect() const = 0;
        virtual void  setAspect(const float v) = 0;
        virtual float getFov() const = 0;
        virtual void  setFov(const float v) = 0;

        virtual bool isActive() const = 0;
        virtual void setActive(bool active) = 0;

        virtual double getTranslationSensitivity() const = 0;
        virtual void   setTranslationSensitivity(double s) = 0;
        virtual double getRotationSensitivity() const = 0;
        virtual void   setRotationSensitivity(double s) = 0;
    };
}
