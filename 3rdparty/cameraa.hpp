#pragma once

#include <array>
#include<bx/math.h>
#include "vcommon.hpp"

namespace volrend {
static const float CAMERA_DEFAULT_FOCAL_LENGTH = 1111.11f;

struct Camera {
    Camera(float width = 256, float height = 256,
           float fx = CAMERA_DEFAULT_FOCAL_LENGTH, float fy = -1.f);
    ~Camera();

    /** Camera params **/
    // Camera pose model, you can modify these
    bx::Vec3 v_back = { -0.7071068f, 0.0f, 0.7071068f };
    bx::Vec3 v_world_up = { 0.0f, 0.0f, 1.0f };
    bx::Vec3 center = { -3.55f, 0.0, 3.55f };
    // Origin for about-origin rotation
    bx::Vec3 origin = { 0.0f, 0.0f, 0.0f };

    // Vectors below are automatically updated
    bx::Vec3 v_up={0,0,0}, v_right={0,0,0};
    bx::Vec3 cen={0,0,0};

    // 4x3 C2W transform used for volume rendering, automatically updated
    float transform[9];

    // Image size
    float width, height;

    // Focal length
    float fx, fy;

    void _update(bool transform_from_vecs = true);
};

}  // namespace volrend
