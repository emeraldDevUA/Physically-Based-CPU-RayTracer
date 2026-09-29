#pragma once

#include <QImage>
#include <QLabel>
#include <QWidget>
#include <atomic>
#include <thread>

#include "render/integrator.h"
#include "scene/camera.h"
#include "scene/scene.h"

class Viewer : public QWidget {
    Q_OBJECT
public:
    Viewer(int width, int height,
           const Scene& scene, const Camera& camera,
           const Integrator& integrator, int samplesPerPixel,
           QLabel* durationLabel, QWidget* parent = nullptr);
    ~Viewer() override;

    const QImage& getImage() const;
    void stopRaytrace();

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override;

    signals:
        void frameReady(QImage image, int samplesDone, double elapsedSeconds);

private slots:
    void onFrameReady(QImage image, int samplesDone, double elapsedSeconds);

private:
    int m_width = 0, m_height = 0;
    const Scene& m_scene;
    const Camera& m_camera;
    const Integrator& m_integrator;
    int m_samplesPerPixel = 1;

    QLabel* m_durationLabel = nullptr;
    QImage m_image;

    std::thread m_renderThread;
    std::atomic<bool> m_cancelled{false};
};