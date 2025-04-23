#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "base_magnet.h"

struct FieldLinePoint {
    glm::vec3 position;
    glm::vec3 field;
};

struct FieldLine {
    std::vector<FieldLinePoint> points;
};

class FieldLineTracer {
public:
    FieldLineTracer(const std::vector<BaseMagnet*>& magnets, float bounds_width, float bounds_height, float bounds_depth,
        float step_size, int max_steps, float adaptive_min_step, float adaptive_max_step,
        float adaptive_field_ref, bool use_adaptive_step, bool render_field_lines);

    // Trace field lines from all start points of all magnets
    std::vector<FieldLine> traceFieldLines();

    // Update cuboid bounds
    void updateBounds(float width, float height, float depth);

    // Set trace configuration
    void setTraceConfig(float step_size, int max_steps, float adaptive_min_step, float adaptive_max_step,
        float adaptive_field_ref, bool use_adaptive_step, bool render_field_lines);

private:
    // Calculate total magnetic field at a position
    glm::vec3 calculateTotalField(const glm::vec3& pos) const;

    // Trace a single field line from a start point in one direction
    void traceFieldLineFromPoint(const glm::vec3& startPos, TraceDirection direction, FieldLine& line);

    // Check if a position is within cuboid bounds
    bool isWithinBounds(const glm::vec3& pos) const;

    // Adaptive step size calculation based on field strength
    float calculateAdaptiveStepSize(const glm::vec3& field) const;

    const std::vector<BaseMagnet*>& mMagnets;

    // Trace settings
    float m_step_size;
    int m_max_steps;
    float m_adaptive_min_step;
    float m_adaptive_max_step;
    float m_adaptive_field_ref;
    bool m_use_adaptive_step;
    bool m_render_field_lines;

    glm::vec3 cuboid_bounds_min;
    glm::vec3 cuboid_bounds_max;
};