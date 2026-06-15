// Entry point for the minimal Global Illumination starter project.
// This version builds a reusable preset scene through SceneFactory.

#include <QApplication>

#include "app/gui.h"
#include "app/scene_factory.h"
#include "geometry/sphere.h"
#include "render/direct_lighting_integrator.h"
#include "render/whitted_integrator.h"


constexpr int width = 1280;
constexpr int height = 720 ;
constexpr int samplesPerPixel = 3;

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    const SceneSetup setup = SceneFactory::createStarterScene(width, height);

    const WhittedIntegrator integrator(3
        , Color(0.08, 0.08, 0.10));

    Gui window(width, height, setup.scene, setup.camera, integrator, samplesPerPixel);
    window.show();

    return QApplication::exec();
}
