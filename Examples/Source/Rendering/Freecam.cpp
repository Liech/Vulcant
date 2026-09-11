#include "Freecam.h"

#include <glm/gtc/matrix_transform.hpp>
#include "Vulcant/Interface/VulcantInput.h"

namespace Vulcant::Rendering
{
    Freecam::Freecam(Vulcant::VulcantWindow& windowInput, const glm::dvec3 eyeInput, const glm::dvec3 targetInput, const glm::dvec3 upInput)
      : window(windowInput)
      , up(upInput)
      , eye(eyeInput)
      , target(targetInput)
    {
        lastMousePosition = glm::dvec2(window.getResolution()) * 0.5;
        window.getInput().setMousePosition(lastMousePosition);
    }

    Freecam::~Freecam() {}

    SceneData Freecam::getScene() const
    {
        Vulcant::Rendering::SceneData scene;

        scene.viewMatrix    = glm::lookAt(eye, target, up);
        scene.invViewMatrix = glm::inverse(scene.viewMatrix);

        float fovY   = glm::radians(75.0f);
        float aspect = 16.0f / 9.0f;
        float zNear  = 0.05f;
        float zFar   = 100.0f;

        scene.projectionMatrix    = glm::perspective(fovY, aspect, zNear, zFar);
        scene.invProjectionMatrix = glm::inverse(scene.projectionMatrix);
        scene.cameraPos           = glm::vec4(eye, 1.0f);
        scene.resolution          = window.getResolution();
        scene.farPlane            = zFar;
        scene.nearPlane           = zNear;
        return scene;
    }

    void Freecam::tick(double deltaTime)
    {
        if (!active)
            return;

        auto& input       = window.getInput();

        auto  currentPos  = window.getInput().getMousePosition();
        auto  diff        = lastMousePosition - currentPos;
        float sensitivity = 0.0002f;

        glm::dvec3 lookDir  = glm::normalize(target - eye);
        glm::dvec3 right    = glm::normalize(glm::cross(lookDir, up));
        glm::dvec3 actualUp = glm::normalize(glm::cross(right, lookDir));

        glm::dmat4 rotY = glm::rotate(glm::dmat4(1.0), diff.x * sensitivity, actualUp);
        glm::dmat4 rotX = glm::rotate(glm::dmat4(1.0), -diff.y * sensitivity, right);

        lookDir = glm::mat3(rotY * rotX) * lookDir;
        target  = eye + lookDir;

        double velocity = 5.0 * (double)deltaTime;
        if (input.isPressed(Vulcant::VulcantInputValue::W))
        {
            eye += lookDir * velocity;
            target += lookDir * velocity;
        }
        else if (input.isPressed(Vulcant::VulcantInputValue::S))
        {
            eye -= lookDir * velocity;
            target -= lookDir * velocity;
        }
        else if (input.isPressed(Vulcant::VulcantInputValue::D))
        {
            eye += right * velocity;
            target += right * velocity;
        }
        else if (input.isPressed(Vulcant::VulcantInputValue::A))
        {
            eye -= right * velocity;
            target -= right * velocity;
        }
        else if (input.isPressed(Vulcant::VulcantInputValue::Q))
        {
            eye += up * velocity;
            target += up * velocity;
        }
        else if (input.isPressed(Vulcant::VulcantInputValue::E))
        {
            eye -= up * velocity;
            target -= up * velocity;
        }

         input.setMousePosition(glm::dvec2(window.getResolution()) * 0.5);
    }

    bool Freecam::keyEvent(const Vulcant::VulcantInputValue& key)
    {
        if (key == Vulcant::VulcantInputValue::Esc)
        {
            active = !active;
            return true;
        }
        return false;
    }

    glm::dvec3 Freecam::getUp() const
    {
        return up;
    }

    void Freecam::setUp(const glm::dvec3 v)
    {
        up = v;
    }

    glm::dvec3 Freecam::getEye() const
    {
        return eye;
    }

    void Freecam::setEye(const glm::dvec3 v)
    {
        eye = v;
    }

    glm::dvec3 Freecam::getTarget() const
    {
        return target;
    }

    void Freecam::setTarget(const glm::dvec3 v)
    {
        target = v;
    }

    float Freecam::getZNear() const
    {
        return zNear;
    }

    void  Freecam::setZNear(const float v)
    {
        zNear = v;
    }
    
    float Freecam::getZFar() const
    {
        return zFar;
    }
    
    void  Freecam::setZFar(const float v)
    {
        zFar = v;
    }
    
    float Freecam::getAspect() const
    {
        return aspect;
    }
    
    void  Freecam::setAspect(const float v)
    {
        aspect = v;
    }
    
    float Freecam::getFov() const
    {
        return fovY;
    }
    
    void  Freecam::setFov(const float v)
    {
        fovY = v;
    }
}