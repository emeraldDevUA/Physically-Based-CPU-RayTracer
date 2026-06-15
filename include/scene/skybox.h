//
// Created by Asus on 5/23/2026.
//

#pragma once
#include "shading/texture.h"

using std::string;

class Skybox {
public:
    explicit Skybox() = default;

    explicit Skybox(const string& skybox_path) {
        x_pos = std::make_unique<ImageTexture>(skybox_path + "/skybox_posx.png");
        x_neg = std::make_unique<ImageTexture>(skybox_path + "/skybox_negx.png");
        y_pos = std::make_unique<ImageTexture>(skybox_path + "/skybox_posy.png");
        y_neg = std::make_unique<ImageTexture>(skybox_path + "/skybox_negy.png");
        z_pos = std::make_unique<ImageTexture>(skybox_path + "/skybox_posz.png");
        z_neg = std::make_unique<ImageTexture>(skybox_path + "/skybox_negz.png");
    }

    Color sample(const glm::dvec3& dir) const;

private:
    std::unique_ptr<ImageTexture> x_pos, x_neg;
    std::unique_ptr<ImageTexture> y_pos, y_neg;
    std::unique_ptr<ImageTexture> z_pos, z_neg;
};