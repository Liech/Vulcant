#include "Freecam.h"

#include "Vulcant/Interface/VulcantInput.h"

#include <algorithm>
#include <cmath>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Vulcant::Toolkit
{
    Freecam::Freecam(Vulcant::VulcantWindow& windowInput, const glm::dvec3 eyeInput, const glm::dvec3 targetInput, const glm::dvec3 upInput)
      : window(windowInput)
      , eye(eyeInput)
      , target(targetInput)
      , up(upInput)
    {
        lastMousePosition = glm::dvec2(window.getResolution()) * 0.5;
        updateAnglesFromVectors();
    }

    Freecam::~Freecam() {}

    void Freecam::getBasis(glm::dvec3& r, glm::dvec3& u, glm::dvec3& f) const
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

    void Freecam::updateAnglesFromVectors()
    {
        glm::dvec3 dir = target - eye;
        if (glm::length(dir) < 0.0001)
        {
            dir = glm::dvec3(0.0, 0.0, -1.0);
        }
        dir = glm::normalize(dir);

        glm::dvec3 r, u, f;
        getBasis(r, u, f);

        double projU = glm::dot(dir, u);
        double projR = glm::dot(dir, r);
        double projF = glm::dot(dir, f);

        pitch = std::asin(std::clamp(projU, -0.9999, 0.9999));
        yaw   = std::atan2(projR, projF);
    }

    void Freecam::updateTargetFromAngles()
    {
        glm::dvec3 dir  = getLookDir();
        double     dist = glm::distance(eye, target);
        if (dist < 0.0001)
        {
            dist = 1.0;
        }
        target = eye + dir * dist;
    }

    glm::dvec3 Freecam::getLookDir() const
    {
        glm::dvec3 r, u, f;
        getBasis(r, u, f);

        glm::dvec3 dir = std::cos(pitch) * std::sin(yaw) * r + std::sin(pitch) * u + std::cos(pitch) * std::cos(yaw) * f;
        return glm::normalize(dir);
    }

    SceneData Freecam::getScene() const
    {
        Vulcant::Toolkit::SceneData scene;

        scene.viewMatrix    = glm::lookAt(eye, target, up);
        scene.invViewMatrix = glm::inverse(scene.viewMatrix);

        scene.projectionMatrix = glm::perspective(fovY, aspect, zNear, zFar);
        scene.projectionMatrix[1][1] *= -1.0f;
        scene.invProjectionMatrix = glm::inverse(scene.projectionMatrix);
        scene.cameraPos           = glm::vec4(eye, 1.0f);
        scene.resolution          = window.getResolution();
        scene.farPlane            = zFar;
        scene.nearPlane           = zNear;
        return scene;
    }

    void Freecam::tick(double deltaTime)
    {
        auto&      input      = window.getInput();
        glm::dvec2 currentPos = input.getMousePosition();

        bool isMiddle = input.isPressed(Vulcant::VulcantInputValue::MouseMiddle);
        bool isRight  = input.isPressed(Vulcant::VulcantInputValue::MouseRight);
        bool isShift  = input.isPressed(Vulcant::VulcantInputValue::Shift);

        if (active)
        {
            const glm::dvec2 mouseCenter = glm::dvec2(window.getResolution()) * 0.5;
            if (firstMouse)
            {
                lastMousePosition = mouseCenter;
                firstMouse        = false;
            }

            auto diff         = currentPos - mouseCenter;
            lastMousePosition = mouseCenter;

            yaw += diff.x * rotationSensitivity;
            pitch -= diff.y * rotationSensitivity;

            const double maxPitch = glm::radians(89.5);
            pitch                 = std::clamp(pitch, -maxPitch, maxPitch);

            const double twoPi = glm::two_pi<double>();
            while (yaw > glm::pi<double>())
                yaw -= twoPi;
            while (yaw < -glm::pi<double>())
                yaw += twoPi;

            updateTargetFromAngles();

            glm::dvec3 lookDir = getLookDir();
            glm::dvec3 worldUp = glm::length(up) > 0.0001 ? glm::normalize(up) : glm::dvec3(0.0, 1.0, 0.0);
            glm::dvec3 right   = glm::normalize(glm::cross(lookDir, worldUp));
            glm::dvec3 camUp   = glm::normalize(glm::cross(right, lookDir));

            glm::dvec3 moveDir(0.0);
            if (input.isPressed(Vulcant::VulcantInputValue::W))
                moveDir += lookDir;
            if (input.isPressed(Vulcant::VulcantInputValue::S))
                moveDir -= lookDir;
            if (input.isPressed(Vulcant::VulcantInputValue::D))
                moveDir += right;
            if (input.isPressed(Vulcant::VulcantInputValue::A))
                moveDir -= right;
            if (input.isPressed(Vulcant::VulcantInputValue::Q))
                moveDir += camUp;
            if (input.isPressed(Vulcant::VulcantInputValue::E))
                moveDir -= camUp;

            if (glm::length(moveDir) > 0.0001)
            {
                double speed = translationSpeed;
                if (isShift)
                {
                    speed *= 2.5;
                }
                glm::dvec3 step = glm::normalize(moveDir) * (speed * deltaTime);
                eye += step;
                target += step;
            }

            input.setMousePosition(mouseCenter);
            return;
        }

        bool isDragging = isMiddle || isRight;
        if (!isDragging)
        {
            lastMousePosition = currentPos;
            firstMouse        = true;
            return;
        }

        if (firstMouse)
        {
            lastMousePosition = currentPos;
            firstMouse        = false;
        }

        glm::dvec2 diff   = currentPos - lastMousePosition;
        lastMousePosition = currentPos;

        glm::dvec3 lookDir = getLookDir();
        glm::dvec3 worldUp = glm::length(up) > 0.0001 ? glm::normalize(up) : glm::dvec3(0.0, 1.0, 0.0);
        glm::dvec3 right   = glm::normalize(glm::cross(lookDir, worldUp));
        glm::dvec3 camUp   = glm::normalize(glm::cross(right, lookDir));

        if (isRight || (isMiddle && isShift))
        {
            yaw += diff.x * rotationSensitivity;
            pitch -= diff.y * rotationSensitivity;

            const double maxPitch = glm::radians(89.5);
            pitch                 = std::clamp(pitch, -maxPitch, maxPitch);

            const double twoPi = glm::two_pi<double>();
            while (yaw > glm::pi<double>())
                yaw -= twoPi;
            while (yaw < -glm::pi<double>())
                yaw += twoPi;

            updateTargetFromAngles();
            lookDir = getLookDir();
            right   = glm::normalize(glm::cross(lookDir, worldUp));
            camUp   = glm::normalize(glm::cross(right, lookDir));

            glm::dvec3 moveDir(0.0);
            if (input.isPressed(Vulcant::VulcantInputValue::W))
                moveDir += lookDir;
            if (input.isPressed(Vulcant::VulcantInputValue::S))
                moveDir -= lookDir;
            if (input.isPressed(Vulcant::VulcantInputValue::D))
                moveDir += right;
            if (input.isPressed(Vulcant::VulcantInputValue::A))
                moveDir -= right;
            if (input.isPressed(Vulcant::VulcantInputValue::Q))
                moveDir += camUp;
            if (input.isPressed(Vulcant::VulcantInputValue::E))
                moveDir -= camUp;

            if (glm::length(moveDir) > 0.0001)
            {
                double speed = translationSpeed;
                if (isShift)
                {
                    speed *= 2.5;
                }
                glm::dvec3 step = glm::normalize(moveDir) * (speed * deltaTime);
                eye += step;
                target += step;
            }
        }
        else if (isMiddle)
        {
            double dist = glm::distance(eye, target);
            if (dist < 0.1)
            {
                dist = 5.0;
            }
            double     viewHeight = 2.0 * dist * std::tan(fovY * 0.5);
            double     viewWidth  = viewHeight * aspect;
            glm::dvec2 res        = glm::dvec2(window.getResolution());
            if (res.x > 0.0 && res.y > 0.0)
            {
                glm::dvec3 panOffset = -right * (diff.x * viewWidth / res.x) + camUp * (diff.y * viewHeight / res.y);
                eye += panOffset;
                target += panOffset;
            }
        }
    }

    bool Freecam::keyEvent(const Vulcant::VulcantInputValue& key)
    {
        if (key == Vulcant::VulcantInputValue::MouseWheelUp)
        {
            glm::dvec3 lookDir = getLookDir();
            glm::dvec3 step    = lookDir * (translationSpeed * 0.2);
            eye += step;
            target += step;
            return true;
        }
        if (key == Vulcant::VulcantInputValue::MouseWheelDown)
        {
            glm::dvec3 lookDir = getLookDir();
            glm::dvec3 step    = lookDir * (translationSpeed * 0.2);
            eye -= step;
            target -= step;
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

    glm::dvec3 Freecam::getUp() const
    {
        return up;
    }

    void Freecam::setUp(const glm::dvec3 v)
    {
        up = v;
        updateAnglesFromVectors();
    }

    glm::dvec3 Freecam::getEye() const
    {
        return eye;
    }

    void Freecam::setEye(const glm::dvec3 v)
    {
        eye = v;
        updateAnglesFromVectors();
    }

    glm::dvec3 Freecam::getTarget() const
    {
        return target;
    }

    void Freecam::setTarget(const glm::dvec3 v)
    {
        target = v;
        updateAnglesFromVectors();
    }

    float Freecam::getZNear() const
    {
        return zNear;
    }

    void Freecam::setZNear(const float v)
    {
        zNear = v;
    }

    float Freecam::getZFar() const
    {
        return zFar;
    }

    void Freecam::setZFar(const float v)
    {
        zFar = v;
    }

    float Freecam::getAspect() const
    {
        return aspect;
    }

    void Freecam::setAspect(const float v)
    {
        aspect = v;
    }

    float Freecam::getFov() const
    {
        return fovY;
    }

    void Freecam::setFov(const float v)
    {
        fovY = v;
    }
}
