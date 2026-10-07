#pragma once

#include "Vulcant/Interface/VulcantWindow.h"
#include "Vulcant/Interface/VulcantInputValue.h"
#include "Camera.h"
#include "SceneData.h"

namespace Vulcant::Rendering
{
    class ArcCam : public Camera
    {
      public:
        ArcCam(Vulcant::VulcantWindow& window, const glm::dvec3 eyeInput, const glm::dvec3 targetInput, const glm::dvec3 upInput = glm::dvec3(0.0, 1.0, 0.0));
        virtual ~ArcCam() override;

        virtual void tick(double delta) override;
        virtual bool keyEvent(const Vulcant::VulcantInputValue& key) override;

        virtual glm::dvec3 getUp() const override;
        virtual void       setUp(const glm::dvec3 v) override;
        virtual glm::dvec3 getEye() const override;
        virtual void       setEye(const glm::dvec3 v) override;
        virtual glm::dvec3 getTarget() const override;
        virtual void       setTarget(const glm::dvec3 v) override;

        virtual float getZNear() const override;
        virtual void  setZNear(const float v) override;
        virtual float getZFar() const override;
        virtual void  setZFar(const float v) override;
        virtual float getAspect() const override;
        virtual void  setAspect(const float v) override;
        virtual float getFov() const override;
        virtual void  setFov(const float v) override;

        virtual bool isActive() const override { return active; }
        virtual void setActive(bool a) override { active = a; }

        virtual double getTranslationSensitivity() const override { return panSpeed; }
        virtual void   setTranslationSensitivity(double s) override { panSpeed = static_cast<float>(s); }
        virtual double getRotationSensitivity() const override { return rotateSensitivity; }
        virtual void   setRotationSensitivity(double s) override { rotateSensitivity = static_cast<float>(s); }

        virtual SceneData getScene() const override;

        // ArcCam specific controls
        double getDistance() const { return distance; }
        void   setDistance(double dist);

        double getYaw() const { return yaw; }
        void   setYaw(double y);

        double getPitch() const { return pitch; }
        void   setPitch(double p);

      private:
        void updateEyeFromAngles();
        void updateAnglesFromEye();

        Vulcant::VulcantWindow& window;

        bool       active = false; // Focus mode: controls movement (WASD/QE/Panning/Zoom)
        glm::dvec2 lastMousePosition;
        bool       firstMouse = true;

        glm::dvec3 target;
        glm::dvec3 eye;
        glm::dvec3 up = glm::dvec3(0.0, 1.0, 0.0);

        // Spherical coordinates around target
        double distance = 5.0;
        double yaw      = 0.0; // in radians
        double pitch    = 0.0; // in radians

        float fovY   = glm::radians(75.0f);
        float aspect = 16.0f / 9.0f;
        float zNear  = 0.05f;
        float zFar   = 2000.0f;

        float rotateSensitivity = 0.005f;
        float panSpeed          = 5.0f;
        float zoomSpeed         = 2.0f;
    };
}
