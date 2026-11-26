#pragma once

#include "Examples/SphereRasterizerScenes/SphereScene.h"

namespace Vulcant
{
    namespace Examples
    {
        class FloatingSpheresScene : public SphereScene
        {
          public:
            virtual ~FloatingSpheresScene() override;

            virtual std::string getName() const override
            {
                return "Floating Spheres";
            }

            virtual void generate(size_t count, float baseRadius, std::vector<Vulcant::Toolkit::SphereData>& outSpheres, std::vector<SphereInitData>& outInitData) override;
        };
    }
}
