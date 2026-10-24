#include "ArcCam.h"

#include <algorithm>
#include <cmath>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Vulcant/Interface/VulcantInput.h"

namespace Vulcant::Toolkit
{
    ArcCam::ArcCam(Vulcant::VulcantWindow& windowInput, const glm::dvec3 eyeInput, const glm::dvec3 targetInput, const glm::dvec3 upInput)
      : window(windowInput)
      , target(targetInput)
      , eye(eyeInput)
      , up(upInput)
    {
        lastMousePosition = glm::dvec2(window.getResolution()) * 0.5;
        updateAnglesFromEye();
    }

    ArcCam::~ArcCam() {}

    void ArcCam::getBasis(glm::dvec3& r, glm::dvec3& u, glm::dvec3& f) const
    {
        u = glm::length(up) > 0.0001 ? glm::normalize(up) : glm::dvec3(0.0, 1.0, 0.0);
        if (std::abs(u.y) < 0.9999)
        {
            r = glm::normalize(glm::cross(glm::dvec3(0.0, 1.0, 0.0), u));
            f = glm::cross(u, r);
        }
        else
        {
            if (u.y > 0.0)
            {
                r = glm::dvec3(1.0, 0.0, 0.0);
                f = glm::dvec3(0.0, 0.0, 1.0);
            }
            else
            {
                r = glm::dvec3(-1.0, 0.0, 0.0);
                f = glm::dvec3(0.0, 0.0, 1.0);
            }
        }
    }

    void ArcCam::updateAnglesFromEye()
    {
        glm::dvec3 dir = eye - target;
        distance       = glm::length(dir);
        if (distance < 0.0001)
        {
            distance = 1.0;
            dir      = glm::dvec3(0.0, 0.0, 1.0);
        }

        glm::dvec3 r, u, f;
        getBasis(r, u, f);

        glm::dvec3 normDir = dir / distance;
        double projU = glm::dot(normDir, u);
        double projR = glm::dot(normDir, r);
        double projF = glm::dot(normDir, f);

        pitch = std::asin(std::clamp(projU, -0.9999, 0.9999));
        yaw   = std::atan2(projR, projF);
    }

    void ArcCam::updateEyeFromAngles()
    {
        // Clamp pitch to avoid gimbal lock flip
        const double maxPitch = glm::radians(89.0);
        pitch = std::clamp(pitch, -maxPitch, maxPitch);

        glm::dvec3 r, u, f;
        getBasis(r, u, f);

        glm::dvec3 offset = distance * (std::cos(pitch) * std::sin(yaw) * r +
                                        std::sin(pitch) * u +
                                        std::cos(pitch) * std::cos(yaw) * f);

        eye = target + offset;
    }

    SceneData ArcCam::getScene() const
    {
        Vulcant::Toolkit::SceneData scene;

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
        // Without focus, do not rotate or move
        if (!active)
        {
            firstMouse = true;
            return;
        }

        auto& input = window.getInput();

        const glm::dvec2 mouseCenter = glm::dvec2(window.getResolution()) * 0.5;
        auto             currentPos  = input.getMousePosition();

        if (firstMouse)
        {
            lastMousePosition = mouseCenter;
            firstMouse        = false;
        }

        auto diff         = currentPos - mouseCenter;
        lastMousePosition = mouseCenter;

        // ArcCam ROTATION: Enabled ONLY with FOCUS (active == true)!
        yaw   += diff.x * rotateSensitivity;
        pitch += diff.y * rotateSensitivity;

        const double twoPi = glm::two_pi<double>();
        while (yaw > glm::pi<double>())  yaw -= twoPi;
        while (yaw < -glm::pi<double>()) yaw += twoPi;

        updateEyeFromAngles();

        // ArcCam MOVEMENT (target pan / translation)
        glm::dvec3 forward = glm::normalize(target - eye);
        glm::dvec3 r, u, f;
        getBasis(r, u, f);

        // Safe right vector that never degenerates to zero even at poles
        glm::dvec3 right = std::cos(yaw) * r - std::sin(yaw) * f;
        glm::dvec3 camUp = glm::normalize(glm::cross(right, forward));

        double moveSpeed = panSpeed * deltaTime;
        if (input.isPressed(Vulcant::VulcantInputValue::Shift))
        {
            moveSpeed *= 2.5;
        }

        glm::dvec3 moveDir(0.0);
        if (input.isPressed(Vulcant::VulcantInputValue::W))
            moveDir += forward;
        if (input.isPressed(Vulcant::VulcantInputValue::S))
            moveDir -= forward;
        if (input.isPressed(Vulcant::VulcantInputValue::A))
            moveDir -= right;
        if (input.isPressed(Vulcant::VulcantInputValue::D))
            moveDir += right;
        if (input.isPressed(Vulcant::VulcantInputValue::Q))
            moveDir += camUp;
        if (input.isPressed(Vulcant::VulcantInputValue::E))
            moveDir -= camUp;

        if (glm::length(moveDir) > 0.0001)
        {
            glm::dvec3 translation = glm::normalize(moveDir) * moveSpeed;
            target += translation;
            eye    += translation;
        }

        input.setMousePosition(mouseCenter);
    }

    bool ArcCam::keyEvent(const Vulcant::VulcantInputValue& key)
    {
        if (key == Vulcant::VulcantInputValue::MouseWheelUp)
        {
            distance = std::max(0.01, distance * 0.9);
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
                firstMouse = true;
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
