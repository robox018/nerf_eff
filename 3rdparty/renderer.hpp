#pragma once

#include <memory>
#include "cameraa.hpp"
#include "n3tree.hpp"
#include <crtdbg.h>


namespace volrend
{
    struct RenderOptions {
        // * BASIC RENDERING
        // Epsilon added to steps to prevent hitting current box again

        float step_size = 1e-4f;

        // If a point has sigma < this amount, considers sigma = 0
        float sigma_thresh = 1e-2f;

        // If remaining light intensity/alpha < this amount stop marching
        float stop_thresh = 1e-2f;

        // Background brightness
        float background_brightness = 1.f;

        float render_bbox_min[3] = { 0.f,0.f,0.f };
        float render_bbox_max[3] = { 1.f,1.f,1.f };
        // Range of basis functions to use
        // no effect if RGBA data format
        float basis_minmax[2] = { 0, 24 };

        // Rotation applied to viewdirs for all rays
        float rot_dirs[3] = { 0.f, 0.f, 0.f };

    };

    // Volume renderer using CUDA or compute shader
    struct VolumeRenderer {
        explicit VolumeRenderer();
        ~VolumeRenderer();
        void VolumeRenderer::destroy();
        // Render the currently set tree
        void render();

        // Set volumetric data to render
        void set(N3Tree& tree);

        // Clear the volumetric data
        void clear();

        // Resize the buffer
        void resize(float width, float height);

        // Camera instance
        Camera camera;

        // Rendering options
        RenderOptions options;

    private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
    };

}