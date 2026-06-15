// Stub for later reflection/refraction work.

#pragma once

#include "render/integrator.h"

class WhittedIntegrator : public Integrator {
public:
    WhittedIntegrator(int maxDepth, const Color& background);

    Color Li(const Ray& ray, const Scene& scene, int depth) const override;


private:
    // Kept for consistency with recursive integrator interfaces.
    int m_maxDepth;
    Color m_background;
};
