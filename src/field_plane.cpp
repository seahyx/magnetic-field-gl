#include "field_plane.h"

FieldPlane::FieldPlane(float width, float height, float z_position)
    : m_width(width), m_height(height), m_z_normalized(z_position)
{
    generateGeometry(width / 2.0f); // Initial depth is irrelevant for default z
}

void FieldPlane::updateZPosition(float z_normalized, float cuboid_depth)
{
    m_z_normalized = std::max(-1.0f, std::min(1.0f, z_normalized));
    generateGeometry(cuboid_depth);
}

void FieldPlane::updateDimensions(float width, float height, float cuboid_depth)
{
    m_width = width;
    m_height = height;
    generateGeometry(cuboid_depth);
}

void FieldPlane::generateGeometry(float cuboid_depth)
{
    float hw = m_width / 2.0f;
    float hh = m_height / 2.0f;
    float z = m_z_normalized * (cuboid_depth / 2.0f); // Map [-1, 1] to [-hd, hd]

    // Vertices (XY plane at z)
    vertices[0] = -hw; vertices[1] = -hh; vertices[2] = z; // Bottom-left
    vertices[3] = hw;  vertices[4] = -hh; vertices[5] = z; // Bottom-right
    vertices[6] = -hw; vertices[7] = hh;  vertices[8] = z; // Top-left
    vertices[9] = hw;  vertices[10] = hh; vertices[11] = z; // Top-right

    // Indices (two triangles)
    indices[0] = 0; indices[1] = 1; indices[2] = 2; // First triangle: 0-1-2
    indices[3] = 2; indices[4] = 1; indices[5] = 3; // Second triangle: 2-1-3
}