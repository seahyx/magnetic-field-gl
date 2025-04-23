#pragma once

#include <glm/glm.hpp>
#include <utility>

class FieldPlane {
public:
    float vertices[12]; // 4 vertices * 3 floats (x, y, z)
    unsigned int indices[6]; // 2 triangles * 3 indices

    FieldPlane(float width, float height, float z_position);

    // Update z-position in range [-1, 1], mapped to cuboid depth
    void updateZPosition(float z_normalized, float cuboid_depth);

    // Update dimensions (width and height) while preserving z-position
    void updateDimensions(float width, float height, float cuboid_depth);

private:
    float m_width;
    float m_height;
    float m_z_normalized; // Z-position in [-1, 1]

    // Generate geometry based on current dimensions and z-position
    void generateGeometry(float cuboid_depth);
};