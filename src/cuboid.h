#pragma once

#include <glm/glm.hpp>

class Cuboid {
public:
    float edge_vertices[24];
    unsigned int edge_indices[24];

    Cuboid(float width, float height, float depth);

    // Update dimensions and regenerate geometry
    void updateDimensions(float width, float height, float depth);

private:
    float m_width;
    float m_height;
    float m_depth;

    // Internal method to generate geometry
    void generateGeometry();
};