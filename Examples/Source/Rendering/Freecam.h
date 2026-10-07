#pragma once

#include "Vulcant/Interface/VulcantWindow.h"
#include "Vulcant/Interface/VulcantInputValue.h"
#include "Camera.h"
#include "SceneData.h"

namespace Vulcant::Rendering
{
    class SceneData;

    class Freecam : public Camera
    {
      public:
        Freecam(Vulcant::VulcantWindow& window, const glm::dvec3 eyeInput, const glm::dvec3 targetInput, const glm::dvec3 upInput);
        virtual ~Freecam() override;

        virtual void tick(double delta) override;
        virtual bool keyEvent(const Vulcant::VulcantInputValue&) override;

        virtual glm::dvec3 getUp() const override;
        virtual void       setUp(const glm::dvec3 v) override;
        virtual glm::dvec3 getEye() const override;
        virtual void       setEye(const glm::dvec3 v) override;
        virtual glm::dvec3 getTarget() const override;
        virtual void       setTarget(const glm::dvec3 v) override;
        
        virtual float      getZNear() const override;
        virtual void       setZNear(const float v) override;
        virtual float      getZFar() const override;
        virtual void       setZFar(const float v) override;
        virtual float      getAspect() const override;
        virtual void       setAspect(const float v) override;
        virtual float      getFov() const override;
        virtual void       setFov(const float v) override;

        virtual bool       isActive() const override { return active; }
        virtual void       setActive(bool a) override { active = a; }

        virtual double     getTranslationSensitivity() const override { return translationSpeed; }
        virtual void       setTranslationSensitivity(double s) override { translationSpeed = s; }
        virtual double     getRotationSensitivity() const override { return rotationSensitivity; }
        virtual void       setRotationSensitivity(double s) override { rotationSensitivity = s; }

        virtual SceneData getScene() const override;

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
        float      zFar   = 2000.0f;

        double     translationSpeed    = 5.0;
        double     rotationSensitivity = 0.0002;

        Vulcant::VulcantWindow& window;
    };
}