// Minimal renderer implementation.
// The starter still performs one sample per pixel and delegates all radiance
// computation to the selected integrator. The samples-per-pixel parameter is
// kept only as a forward-compatible hook for later assignments.

#include <omp.h>
#include <random>
#include <iomanip>
#include <thread>
#include <atomic>
#include <iostream>
#include <chrono>

#include "render/integrator.h"
#include "render/renderer.h"
#include "scene/camera.h"
#include "scene/scene.h"
#include "io/image.h"


Renderer::Renderer(const int samplesPerPixel)
    : m_samplesPerPixel(samplesPerPixel > 0 ? samplesPerPixel : 1)
{
}


void Renderer::render(const Scene& scene,
                      const Camera& camera,
                      const Integrator& integrator,
                      Image& image,
                      const ProgressFn& onProgress) const
{
    const int num_threads = static_cast<int>(std::thread::hardware_concurrency());
    constexpr int kTileSize = 32;

    const int tilesX = (image.width() + kTileSize - 1) / kTileSize;
    const int tilesY = (image.height() + kTileSize - 1) / kTileSize;
    const int totalTiles = tilesX * tilesY;

    std::cout << "Program running on " << num_threads << " threads" << std::endl;
    std::cout << "Tile Size: " << kTileSize << std::endl;

    std::vector accumBuffer(
        static_cast<size_t>(image.width()) * image.height(), Color(0.0));

    std::vector<int> sampleCheckpoints;

    for (int s = 2; s < m_samplesPerPixel; s *= 2)
        sampleCheckpoints.push_back(s);

    sampleCheckpoints.push_back(m_samplesPerPixel);

    int samplesDoneSoFar = 0;


    const auto start = std::chrono::high_resolution_clock::now();

    for (int checkpoint : sampleCheckpoints)
    {
        const int samplesThisPass = checkpoint - samplesDoneSoFar;

        std::cout << "\nRendering pass: " << samplesDoneSoFar
            << " -> " << checkpoint << " spp" << std::endl;

        std::atomic completedTiles = 0;

#pragma omp parallel for schedule(dynamic)
        for (int tileIdx = 0; tileIdx < totalTiles; ++tileIdx)
        {
            const int tx = (tileIdx % tilesX) * kTileSize;
            const int ty = (tileIdx / tilesX) * kTileSize;

            std::mt19937_64 rng(
                omp_get_thread_num() * 1000003 +
                tileIdx * 99991 +
                checkpoint * 7919
            );

            std::uniform_real_distribution<double> dist(0.0, 1.0);

            for (int y = ty; y < std::min(ty + kTileSize, image.height()); ++y)
            {
                for (int x = tx; x < std::min(tx + kTileSize, image.width()); ++x)
                {
                    Color accumulated(0.0);

                    for (int s = 0; s < samplesThisPass; ++s)
                    {
                        Ray ray = camera.generateRay(x + dist(rng), y + dist(rng));
                        accumulated += integrator.Li(ray, scene, 0);
                    }

                    const size_t idx = static_cast<size_t>(y) * image.width() + x;
                    accumBuffer[idx] += accumulated;

                    image.setPixel(x, y, accumBuffer[idx] / static_cast<double>(checkpoint));
                }
            }

            if (const int done = completedTiles.fetch_add(1) + 1; done % 10 == 0 || done == totalTiles)
            {
#pragma omp critical
                {
                    double percent = 100.0 * done / totalTiles;
                    constexpr int BAR_WIDTH = 40;
                    int filled = static_cast<int>(percent / 100.0 * BAR_WIDTH);

                    std::cout << "\r[";
                    for (int i = 0; i < BAR_WIDTH; i++)
                        std::cout << (i < filled ? '=' : ' ');

                    std::cout << "] " << std::fixed << std::setprecision(1) << percent << "%   " << std::flush;

                    if (onProgress)
                    {
                        const auto now = std::chrono::high_resolution_clock::now();
                        const double elapsedSeconds =
                            std::chrono::duration<double>(now - start).count();
                        onProgress(image, checkpoint, elapsedSeconds);
                    }

                    samplesDoneSoFar = checkpoint;
                }
            }
        }

        std::cout << std::endl;
    }
}
