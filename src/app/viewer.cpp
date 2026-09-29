#include "app/viewer.h"
#include <QPainter>
#include <chrono>

#include "io/image.h"
#include "render/renderer.h"

Viewer::Viewer(const int width, const int height,
               const Scene& scene, const Camera& camera,
               const Integrator& integrator, const int samplesPerPixel,
               QLabel* durationLabel, QWidget* parent)
    : QWidget(parent),
      m_width(width), m_height(height),
      m_scene(scene), m_camera(camera), m_integrator(integrator),
      m_samplesPerPixel(samplesPerPixel),
      m_durationLabel(durationLabel),
      m_image(width, height, QImage::Format_RGB32)
{
    connect(this, &Viewer::frameReady, this, &Viewer::onFrameReady, Qt::QueuedConnection);

    m_renderThread = std::thread([this]() {
        Image image(m_width, m_height);
        const Renderer renderer(m_samplesPerPixel);

        renderer.render(m_scene, m_camera, m_integrator, image,
            [this](const Image& img, const int samplesDone, const double elapsedSeconds) {
                if (m_cancelled.load()) return;
                emit frameReady(img.qimage().copy(), samplesDone, elapsedSeconds);
            });
    });
}

Viewer::~Viewer() {
    stopRaytrace();
    if (m_renderThread.joinable())
        m_renderThread.join();
}

void Viewer::onFrameReady(QImage image, int samplesDone, double elapsedSeconds) {
    m_image = std::move(image);

    if (m_durationLabel) {
        m_durationLabel->setText(
            QString("Resolution:%1 x %2 Render time: %3 seconds %4 samples per pixel)")
                .arg(m_image.width()).arg(m_image.height())
                .arg(static_cast<float>(elapsedSeconds))
                .arg(samplesDone)
        );
    }
    update();
}

const QImage& Viewer::getImage() const { return m_image; }

void Viewer::stopRaytrace() {
    m_cancelled.store(true);
}

void Viewer::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);
    if (!m_image.isNull())
        painter.drawImage(rect(), m_image);
}

QSize Viewer::sizeHint() const { return {m_width, m_height}; }