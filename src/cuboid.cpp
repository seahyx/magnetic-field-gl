#include "cuboid.h"

Cuboid::Cuboid(float width, float height, float depth)
    : m_width(width), m_height(height), m_depth(depth)
{
    generateGeometry();
}

void Cuboid::updateDimensions(float width, float height, float depth)
{
    m_width = width;
    m_height = height;
    m_depth = depth;
    generateGeometry();
}

void Cuboid::generateGeometry()
{
    float hw = m_width / 2.0f;
    float hh = m_height / 2.0f;
    float hd = m_depth / 2.0f;

    // Vertices for edge rendering (8 corners)
    edge_vertices[0] = -hw; edge_vertices[1] = -hh; edge_vertices[2] = -hd;   // 0: bottom-left-front
    edge_vertices[3] = hw;  edge_vertices[4] = -hh; edge_vertices[5] = -hd;   // 1: bottom-right-front
    edge_vertices[6] = hw;  edge_vertices[7] = hh;  edge_vertices[8] = -hd;   // 2: top-right-front
    edge_vertices[9] = -hw; edge_vertices[10] = hh; edge_vertices[11] = -hd;  // 3: top-left-front
    edge_vertices[12] = -hw; edge_vertices[13] = -hh; edge_vertices[14] = hd; // 4: bottom-left-back
    edge_vertices[15] = hw;  edge_vertices[16] = -hh; edge_vertices[17] = hd; // 5: bottom-right-back
    edge_vertices[18] = hw;  edge_vertices[19] = hh;  edge_vertices[20] = hd; // 6: top-right-back
    edge_vertices[21] = -hw; edge_vertices[22] = hh;  edge_vertices[23] = hd; // 7: top-left-back

    // Indices for edge rendering (12 lines, 24 indices)
    edge_indices[0] = 0; edge_indices[1] = 1;   // Front bottom
    edge_indices[2] = 1; edge_indices[3] = 2;   // Front right
    edge_indices[4] = 2; edge_indices[5] = 3;   // Front top
    edge_indices[6] = 3; edge_indices[7] = 0;   // Front left
    edge_indices[8] = 4; edge_indices[9] = 5;   // Back bottom
    edge_indices[10] = 5; edge_indices[11] = 6; // Back right
    edge_indices[12] = 6; edge_indices[13] = 7; // Back top
    edge_indices[14] = 7; edge_indices[15] = 4; // Back left
    edge_indices[16] = 0; edge_indices[17] = 4; // Left bottom
    edge_indices[18] = 1; edge_indices[19] = 5; // Right bottom
    edge_indices[20] = 2; edge_indices[21] = 6; // Right top
    edge_indices[22] = 3; edge_indices[23] = 7; // Left top
}