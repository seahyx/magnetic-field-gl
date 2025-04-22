#pragma once

#include <glm/glm.hpp>

class Cuboid {
public:
    float field_vertices[12];
    unsigned int field_indices[6];
    float edge_vertices[24];
    unsigned int edge_indices[24];

    Cuboid(float width, float height, float depth) {
        float hw = width / 2.0f;
        float hh = height / 2.0f;
        float hd = depth / 2.0f;

        // Vertices for field rendering (two triangles covering XY plane at z=-hd)
        field_vertices[0] = -hw; field_vertices[1] = -hh; field_vertices[2] = -hd;
        field_vertices[3] = hw; field_vertices[4] = hh; field_vertices[5] = -hd;
        field_vertices[6] = -hw; field_vertices[7] = hh; field_vertices[8] = -hd;
        field_vertices[9] = hw; field_vertices[10] = -hh; field_vertices[11] = -hd;

        field_indices[0] = 0; field_indices[1] = 1; field_indices[2] = 2;
        field_indices[3] = 0; field_indices[4] = 3; field_indices[5] = 1;

        // Vertices for edge rendering (8 corners)
        edge_vertices[0] = -hw; edge_vertices[1] = -hh; edge_vertices[2] = -hd;
        edge_vertices[3] = hw; edge_vertices[4] = -hh; edge_vertices[5] = -hd;
        edge_vertices[6] = hw; edge_vertices[7] = hh; edge_vertices[8] = -hd;
        edge_vertices[9] = -hw; edge_vertices[10] = hh; edge_vertices[11] = -hd;
        edge_vertices[12] = -hw; edge_vertices[13] = -hh; edge_vertices[14] = hd;
        edge_vertices[15] = hw; edge_vertices[16] = -hh; edge_vertices[17] = hd;
        edge_vertices[18] = hw; edge_vertices[19] = hh; edge_vertices[20] = hd;
        edge_vertices[21] = -hw; edge_vertices[22] = hh; edge_vertices[23] = hd;

        // Indices for edge rendering (12 lines)
        edge_indices[0] = 0; edge_indices[1] = 1;
        edge_indices[2] = 1; edge_indices[3] = 2;
        edge_indices[4] = 2; edge_indices[5] = 3;
        edge_indices[6] = 3; edge_indices[7] = 0;
        edge_indices[8] = 4; edge_indices[9] = 5;
        edge_indices[10] = 5; edge_indices[11] = 6;
        edge_indices[12] = 6; edge_indices[13] = 7;
        edge_indices[14] = 7; edge_indices[15] = 4;
        edge_indices[16] = 0; edge_indices[17] = 4;
        edge_indices[18] = 1; edge_indices[19] = 5;
        edge_indices[20] = 2; edge_indices[21] = 6;
        edge_indices[22] = 3; edge_indices[23] = 7;
    }
};