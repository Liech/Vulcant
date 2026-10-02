#pragma once

#include "Examples/SphereRasterizerScenes/SphereScene.h"

namespace Vulcant
{
    namespace Examples
    {
        class CubeGridScene : public SphereScene
        {
          public:
            virtual ~CubeGridScene() override = default;

            virtual std::string getName() const override
            {
                return "3D Cube Grid";
            }

            virtual void generate(size_t count, float baseRadius, std::vector<SphereData>& outSpheres, std::vector<SphereInitData>& outInitData) override;
        };
    }
}
