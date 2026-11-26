#pragma once

#include "Examples/SphereRasterizerScenes/SphereScene.h"

namespace Vulcant
{
    namespace Examples
    {
        class HillLandscapeScene : public SphereScene
        {
          public:
            virtual ~HillLandscapeScene() override;

            virtual std::string getName() const override
            {
                return "Hill Landscape";
            }

            virtual void generate(size_t count, float baseRadius, std::vector<Vulcant::Toolkit::SphereData>& outSpheres, std::vector<SphereInitData>& outInitData) override;
        };
    }
}
