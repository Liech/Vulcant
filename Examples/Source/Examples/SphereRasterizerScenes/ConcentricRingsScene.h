#pragma once

#include "Examples/SphereRasterizerScenes/SphereScene.h"

namespace Vulcant
{
    namespace Examples
    {
        class ConcentricRingsScene : public SphereScene
        {
          public:
            virtual ~ConcentricRingsScene() override;

            virtual std::string getName() const override
            {
                return "Concentric Rings";
            }

            virtual void generate(size_t count, float baseRadius, std::vector<Vulcant::Toolkit::SphereData>& outSpheres, std::vector<SphereInitData>& outInitData) override;
        };
    }
}
