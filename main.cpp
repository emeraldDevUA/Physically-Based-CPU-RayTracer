// Entry point for the minimal Global Illumination starter project.
// This version builds a reusable preset scene through SceneFactory.

#include <QApplication>

#include "app/gui.h"
#include "app/scene_factory.h"
#include "geometry/sphere.h"

#include "render/whitted_integrator.h"


constexpr int width = 640;
constexpr int height = 480;
constexpr int samplesPerPixel = 1;

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    const auto [scene, camera] = SceneFactory::createStarterScene( width, height);

    const WhittedIntegrator integrator(35
        , Color(0.08, 0.08, 0.10));

    Gui window(width, height, scene, camera, integrator, samplesPerPixel);
    window.show();



    return QApplication::exec();
}
