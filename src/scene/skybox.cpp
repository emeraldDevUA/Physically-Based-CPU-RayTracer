//
// Created by Asus on 5/23/2026.
//

#include "scene/skybox.h"

#include "scene/hit_record.h"


Color Skybox::sample(const glm::dvec3& dir) const {
    const glm::dvec3 d = -glm::normalize(dir);
    const double ax = std::abs(d.x);
    const double ay = std::abs(d.y);
    const double az = std::abs(d.z);

    double u, v;
    const ImageTexture* face;

    if (ax >= ay && ax >= az) {
        face = d.x > 0 ? x_pos.get() : x_neg.get();
        u = d.x > 0 ? -d.z / ax : d.z / ax;
        v = -d.y / ax;
    } else if (ay >= ax && ay >= az) {
        face = d.y > 0 ? y_pos.get() : y_neg.get();
        u = d.x / ay;
        v = d.y > 0 ? d.z / ay : -d.z / ay;
    } else {
        face = d.z > 0 ? z_pos.get() : z_neg.get();
        u = d.z > 0 ? d.x / az : -d.x / az;
        v = -d.y / az;
    }

    if (!face) return Color(0.0); // unloaded face fallback

    u = 0.5 * (u + 1.0);
    v = 0.5 * (v + 1.0);

    return face->value(glm::dvec2(u, v), glm::dvec3(0));
}
