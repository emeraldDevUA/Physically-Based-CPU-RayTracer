#include "app/gui.h"

// ── palette ────────────────────────────────────────────────────────────────
// Primary:   #1B5E20  (deep forest, toolbar bg)
// Accent:    #66BB6A  (mid-green, button face)
// Hover:     #43A047  (slightly deeper on hover)
// Press:     #2E7D32  (pressed state)
// Surface:   #F1F8E9  (light tint, label bg)
// On-dark:   #FFFFFF  (text on toolbar)
// On-light:  #1B5E20  (text on surface)
// ───────────────────────────────────────────────────────────────────────────

static constexpr auto kGlobalStyle = R"(
    QMainWindow {
        background-color: #263238;
    }
)";

static constexpr auto kToolbarStyle = R"(
    QToolBar {
        background-color: #1B5E20;
        border-bottom: 2px solid #2E7D32;
        padding: 6px 10px;
        spacing: 8px;
    }
)";

static constexpr auto kSaveButtonStyle = R"(
    QPushButton {
        background-color: #66BB6A;
        color: #FFFFFF;
        font-family: 'Roboto', 'Segoe UI', sans-serif;
        font-size: 13px;
        font-weight: 600;
        letter-spacing: 0.5px;
        border: none;
        border-radius: 4px;
        padding: 7px 18px;
        text-transform: uppercase;
    }
    QPushButton:hover {
        background-color: #43A047;
    }
    QPushButton:pressed {
        background-color: #2E7D32;
        padding-top: 8px;
        padding-bottom: 6px;
    }
    QPushButton:disabled {
        background-color: #A5D6A7;
        color: #E8F5E9;
    }
)";

static constexpr auto kLabelStyle = R"(
    QLabel {
        color: #C8E6C9;
        font-family: 'Roboto Mono', 'Consolas', monospace;
        font-size: 12px;
        letter-spacing: 0.3px;
        padding: 4px 10px;
        background-color: rgba(0, 0, 0, 0.20);
        border-radius: 3px;
    }
)";

// ── constructor ─────────────────────────────────────────────────────────────

Gui::Gui(const int width,
         const int height,
         const Scene& scene,
         const Camera& camera,
         const Integrator& integrator,
         const int samplesPerPixel,
         const QWindow* parent)
    : QMainWindow(nullptr)
{
    Q_UNUSED(parent);

    setStyleSheet(kGlobalStyle);

    // ── toolbar ─────────────────────────────────────────────────────────────
    auto* toolbar = new QToolBar(this);
    toolbar->setMovable(false);
    toolbar->setFloatable(false);
    toolbar->setStyleSheet(kToolbarStyle);

    // Save button
    m_saveButton = new QPushButton("Save image", this);
    m_saveButton->setStyleSheet(kSaveButtonStyle);
    m_saveButton->setCursor(Qt::PointingHandCursor);
    m_saveButton->setFixedHeight(34);
    toolbar->addWidget(m_saveButton);

    // Push label to the right
    auto* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolbar->addWidget(spacer);

    // Status label
    m_durationText = new QLabel("Rendering…", this);
    m_durationText->setStyleSheet(kLabelStyle);
    m_durationText->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    toolbar->addWidget(m_durationText);

    addToolBar(toolbar);

    // ── render viewer ────────────────────────────────────────────────────────
    m_viewer = new Viewer(
        width, height,
        scene, camera, integrator, samplesPerPixel,
        m_durationText, this
    );
    m_viewer->resize(width, height);
    setCentralWidget(m_viewer);

    // ── save dialog ──────────────────────────────────────────────────────────
    connect(m_saveButton, &QPushButton::clicked, [this]() {
        const QString filename = QFileDialog::getSaveFileName(
            this,
            tr("Save Render"),
            "render.png",
            tr("PNG Image (*.png);;All Files (*)")
        );
        if (filename.isEmpty()) return;

        QFile file(filename);
        if (file.open(QIODevice::WriteOnly))
            m_viewer->getImage().save(&file, "PNG");
    });

    resize(width, height);
    setWindowTitle("Raytracer");
}

Gui::~Gui() {
    if (m_viewer) {
        m_viewer->stopRaytrace();
    }
}