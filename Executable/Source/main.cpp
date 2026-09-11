#include "Examples/MandelbrotImage.h"
#include "Examples/SphereImage.h"
#include "Examples/CubeCaster.h"
#include "Examples/CubeCasterFreecam.h"
#include "Examples/GraphicPipelineTriangle.h"
#include "Examples/GraphicPipelineCube.h"
#include "Examples/UiExample.h"

const int WIDTH          = 1920; // Size of rendered mandelbrot set.
const int HEIGHT         = 1080; // Size of renderered mandelbrot set.
const int WORKGROUP_SIZE = 32;


//void img()
//{
//    Vulcant::VulcantV::VulcantVDevice dev;
//    auto&                             cmd   = dev.getDefaultCommand();
//    auto                              img   = Library::Gigavox::Gigavoxel();
//    Library::Gigavox::SceneData       scene = Library::Gigavox::Example::create_test_data();
//
//    std::string path = "C:/Users/nicol/Downloads/";
//
//    cmd.startRecord();
//    img.setDevice(dev, cmd);
//    img.setResolution(glm::ivec2(WIDTH, HEIGHT));
//    img.setSceneData(scene);
//    img.render();
//    cmd.endRecord();
//    cmd.runSync();
//
//    img.getColor().saveRenderedImage(path + "color.png");
//    img.getDepth().saveRenderedImage(path + "depth.png");
//    img.getNormal().saveRenderedImage(path + "normal.png");
//}


int main()
{
    //Vulcant::Examples::MandelbrotImage::demo();
    //Vulcant::Examples::SphereImage::demo();
    //Vulcant::Examples::CubeCaster::demo();
    //Vulcant::Examples::CubeCasterFreecam::demo();
    //Vulcant::Examples::GraphicPipelineTriangle::demo();
    //Vulcant::Examples::GraphicPipelineCube::demo();
    Vulcant::Examples::UiExample::demo();

    return EXIT_SUCCESS;
}