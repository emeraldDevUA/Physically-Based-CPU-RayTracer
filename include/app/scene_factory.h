// SceneFactory keeps scene construction out of main.cpp.

#pragma once

#include "scene/scene.h"
#include "scene/camera.h"

struct SceneSetup {
    Scene scene;
    Camera camera;
};

class SceneFactory {
public:

    static SceneSetup createStarterScene(int width, int height);
    static SceneSetup createDielectricScene(int width, int height);
    static SceneSetup createAircraftScene(int width, int height);
    static SceneSetup createStormShadowScene(int width, int height);
    static SceneSetup createMaterialGridScene(int width, int height);

};
