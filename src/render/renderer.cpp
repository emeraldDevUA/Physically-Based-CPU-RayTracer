// Minimal renderer implementation.
// The starter still performs one sample per pixel and delegates all radiance
// computation to the selected integrator. The samples-per-pixel parameter is
// kept only as a forward-compatible hook for later assignments.

#include "render/renderer.h"
#include <omp.h>
#include <random>
#include <iomanip>

#include "io/image.h"
#include "render/integrator.h"
#include "scene/camera.h"
#include "scene/scene.h"

Renderer::Renderer(const int samplesPerPixel)
    : m_samplesPerPixel(samplesPerPixel > 0 ? samplesPerPixel : 1)
{
}

#include <atomic>
#include <iostream>

void Renderer::render(const Scene& scene,
                      const Camera& camera,
                      const Integrator& integrator,
                      Image& image) const
{
    constexpr int kTileSize = 32;

    const int tilesX =
        (image.width() + kTileSize - 1) / kTileSize;

    const int tilesY =
        (image.height() + kTileSize - 1) / kTileSize;

    const int totalTiles =
        tilesX * tilesY;

    std::atomic<int> completedTiles = 0;

#pragma omp parallel for schedule(dynamic)
    for (int tileIdx = 0; tileIdx < totalTiles; ++tileIdx)
    {
        const int tx =
            (tileIdx % tilesX) * kTileSize;

        const int ty =
            (tileIdx / tilesX) * kTileSize;

        std::mt19937_64 rng(
            omp_get_thread_num() * 1000003 +
            tileIdx * 99991
        );

        std::uniform_real_distribution<> dist(0.0, 1.0);

        for (int y = ty; y < std::min(ty + kTileSize, image.height()); ++y)
        {
            for (int x = tx; x < std::min(tx + kTileSize, image.width()); ++x)
            {
                Color accumulated(0.0);

                for (int s = 0; s < m_samplesPerPixel; ++s)
                {
                    Ray ray =
                        camera.generateRay(
                            x + dist(rng),
                            y + dist(rng)
                        );

                    accumulated +=
                        integrator.Li(ray, scene, 0);
                }

                image.setPixel(x, y, accumulated / static_cast<double>(m_samplesPerPixel)
                );
            }
        }

        //--------------------------------
        // progress update
        //--------------------------------

        const int done =
            completedTiles.fetch_add(1) + 1;

        if (done % 10 == 0 || done == totalTiles)
        {
#pragma omp critical
            {
                double percent =
                    100.0 * done / totalTiles;
                constexpr int BAR_WIDTH = 40;

                int filled =
                    percent / 100.0 * BAR_WIDTH;

                std::cout << "\r[";

                for (int i = 0; i < BAR_WIDTH; i++)
                    std::cout << (i < filled ? '=' : ' ');

                std::cout
                    << "] "
                    << std::fixed
                    << std::setprecision(1)
                    << percent
                    << "%   "
                    << std::flush;
            }
        }
    }

    std::cout << std::endl;
}
