#include "ArcCam.h"

#include <algorithm>
#include <cmath>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Vulcant/Interface/VulcantInput.h"

namespace Vulcant::Rendering
{
    ArcCam::ArcCam(Vulcant::VulcantWindow& windowInput, const glm::dvec3 eyeInput, const glm::dvec3 targetInput, const glm::dvec3 upInput)
      : window(windowInput)
      , target(targetInput)
      , eye(eyeInput)
      , up(upInput)
    {
        lastMousePosition = window.getInput().getMousePosition();
        updateAnglesFromEye();
    }

    ArcCam::~ArcCam() {}

    void ArcCam::updateAnglesFromEye()
    {
        glm::dvec3 dir = eye - target;
        distance       = glm::length(dir);
        if (distance < 0.0001)
        {
            distance = 1.0;
            dir      = glm::dvec3(0.0, 0.0, 1.0);
        }

        glm::dvec3 normDir = dir / distance;
        pitch = std::asin(std::clamp(normDir.y, -0.9999, 0.9999));
        yaw   = std::atan2(normDir.x, normDir.z);
    }

    void ArcCam::updateEyeFromAngles()
    {
        // Clamp pitch to avoid gimbal lock flip
        const double maxPitch = glm::radians(89.0);
        pitch = std::clamp(pitch, -maxPitch, maxPitch);

        glm::dvec3 offset;
        offset.x = distance * std::cos(pitch) * std::sin(yaw);
        offset.y = distance * std::sin(pitch);
        offset.z = distance * std::cos(pitch) * std::cos(yaw);

        eye = target + offset;
    }

    SceneData ArcCam::getScene() const
    {
        Vulcant::Rendering::SceneData scene;

        scene.viewMatrix    = glm::lookAt(eye, target, up);
        scene.invViewMatrix = glm::inverse(scene.viewMatrix);

        scene.projectionMatrix    = glm::perspective(fovY, aspect, zNear, zFar);
        scene.projectionMatrix[1][1] *= -1.0f;
        scene.invProjectionMatrix = glm::inverse(scene.projectionMatrix);
        scene.cameraPos           = glm::vec4(eye, 1.0f);
        scene.resolution          = window.getResolution();
        scene.farPlane            = zFar;
        scene.nearPlane           = zNear;
        return scene;
    }

    void ArcCam::tick(double deltaTime)
    {
        auto& input = window.getInput();

        // 1. Mouse Wheel ZOOM: Always available or when active/interacting
        // If MouseWheel events arrive, adjust distance
        if (input.isPressed(Vulcant::VulcantInputValue::MouseWheelUp))
        {
            distance = std::max(0.1, distance - zoomSpeed * (double)deltaTime * 10.0);
            updateEyeFromAngles();
        }
        else if (input.isPressed(Vulcant::VulcantInputValue::MouseWheelDown))
        {
            distance += zoomSpeed * (double)deltaTime * 10.0;
            updateEyeFromAngles();
        }

        // Without focus, do not rotate or move
        if (!active)
        {
            firstMouse = true;
            return;
        }

        const glm::dvec2 mouseCenter = glm::dvec2(window.getResolution()) * 0.5;
        auto             currentPos  = input.getMousePosition();
        auto             diff        = (lastMousePosition == mouseCenter) ? (currentPos - lastMousePosition) : glm::dvec2(0.0);
        lastMousePosition            = mouseCenter;

        // 2. ArcCam ROTATION: Enabled ONLY with FOCUS (active == true)!
        // Moving mouse right increases yaw (rotates eye right), moving mouse up increases pitch (looks up)
        yaw   += diff.x * rotateSensitivity;
        pitch += diff.y * rotateSensitivity;
        updateEyeFromAngles();

        // 3. ArcCam MOVEMENT (target pan / translation): Enabled ONLY with FOCUS (active == true)!
        glm::dvec3 forward = glm::normalize(target - eye);
        glm::dvec3 right   = glm::normalize(glm::cross(forward, up));
        glm::dvec3 camUp   = glm::normalize(glm::cross(right, forward));

        double moveSpeed = panSpeed * (double)deltaTime;

        glm::dvec3 translation(0.0);

        if (input.isPressed(Vulcant::VulcantInputValue::W))
        {
            translation += forward * moveSpeed;
        }
        if (input.isPressed(Vulcant::VulcantInputValue::S))
        {
            translation -= forward * moveSpeed;
        }
        if (input.isPressed(Vulcant::VulcantInputValue::A))
        {
            translation -= right * moveSpeed;
        }
        if (input.isPressed(Vulcant::VulcantInputValue::D))
        {
            translation += right * moveSpeed;
        }
        if (input.isPressed(Vulcant::VulcantInputValue::Q))
        {
            translation += up * moveSpeed;
        }
        if (input.isPressed(Vulcant::VulcantInputValue::E))
        {
            translation -= up * moveSpeed;
        }

        if (glm::length(translation) > 0.0)
        {
            target += translation;
            eye    += translation;
        }

        input.setMousePosition(mouseCenter);
    }

    bool ArcCam::keyEvent(const Vulcant::VulcantInputValue& key)
    {
        if (key == Vulcant::VulcantInputValue::MouseWheelUp)
        {
            distance = std::max(0.1, distance * 0.9);
            updateEyeFromAngles();
            return true;
        }
        if (key == Vulcant::VulcantInputValue::MouseWheelDown)
        {
            distance = distance * 1.1;
            updateEyeFromAngles();
            return true;
        }

        if (key == Vulcant::VulcantInputValue::Esc)
        {
            active = !active;
            if (active)
            {
                lastMousePosition = glm::dvec2(window.getResolution()) * 0.5;
                window.getInput().setMousePosition(lastMousePosition);
            }
            return true;
        }
        return false;
    }

    glm::dvec3 ArcCam::getUp() const
    {
        return up;
    }

    void ArcCam::setUp(const glm::dvec3 v)
    {
        up = v;
        updateEyeFromAngles();
    }

    glm::dvec3 ArcCam::getEye() const
    {
        return eye;
    }

    void ArcCam::setEye(const glm::dvec3 v)
    {
        eye = v;
        updateAnglesFromEye();
    }

    glm::dvec3 ArcCam::getTarget() const
    {
        return target;
    }

    void ArcCam::setTarget(const glm::dvec3 v)
    {
        target = v;
        updateEyeFromAngles();
    }

    void ArcCam::setDistance(double dist)
    {
        distance = std::max(0.01, dist);
        updateEyeFromAngles();
    }

    void ArcCam::setYaw(double y)
    {
        yaw = y;
        updateEyeFromAngles();
    }

    void ArcCam::setPitch(double p)
    {
        pitch = p;
        updateEyeFromAngles();
    }

    float ArcCam::getZNear() const
    {
        return zNear;
    }

    void ArcCam::setZNear(const float v)
    {
        zNear = v;
    }

    float ArcCam::getZFar() const
    {
        return zFar;
    }

    void ArcCam::setZFar(const float v)
    {
        zFar = v;
    }

    float ArcCam::getAspect() const
    {
        return aspect;
    }

    void ArcCam::setAspect(const float v)
    {
        aspect = v;
    }

    float ArcCam::getFov() const
    {
        return fovY;
    }

    void ArcCam::setFov(const float v)
    {
        fovY = v;
    }
}
