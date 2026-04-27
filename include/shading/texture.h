// Texture interface and a minimal constant texture.
// Extend this file when adding procedural or image textures.

#pragma once

#include <glm/glm.hpp>
#include <memory>

#include "core/color.h"
#include <QImage>
#include <stdexcept>

class Texture {
public:
    virtual ~Texture() = default;

    virtual Color value(const glm::dvec2& uv,
                        const glm::dvec3& position) const = 0;
};

class ConstantTexture : public Texture {
public:
    explicit ConstantTexture(const Color& color) : m_color(color) {}

    Color value(const glm::dvec2&, const glm::dvec3&) const override {
        return m_color;
    }

private:
    Color m_color;
};


class ImageTexture : public Texture {
public:
    explicit ImageTexture(const std::string& path) {
        m_image = QImage(QString::fromStdString(path));
        if (m_image.isNull()) {
            throw std::runtime_error("Failed to load texture: " + path);
        }
        // Ensure consistent format for pixel reads
        m_image = m_image.convertToFormat(QImage::Format_RGB32);
    }

    Color value(const glm::dvec2& uv, const glm::dvec3&) const override {
        // Clamp UVs to [0, 1]
        const double u = glm::clamp(uv.x, 0.0, 1.0);
        const double v = glm::clamp(uv.y, 0.0, 1.0);

        // Convert to pixel coords, flip V (image Y=0 is top, UV Y=0 is bottom)
        const int x = static_cast<int>(u * (m_image.width()  - 1));
        const int y = static_cast<int>((1.0 - v) * (m_image.height() - 1));

        const QRgb pixel = m_image.pixel(x, y);

        return {
            qRed(pixel)   / 255.0,
            qGreen(pixel) / 255.0,
            qBlue(pixel)  / 255.0
        };
    }

private:
    QImage m_image;
};