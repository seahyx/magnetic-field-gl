#include "field_line_tracer.h"
#include <algorithm>
#include <thread>
#include <mutex>
#include <vector>

FieldLineTracer::FieldLineTracer(const std::vector<BaseMagnet*>& magnets, float bounds_width, float bounds_height, float bounds_depth,
    float step_size, int max_steps, float adaptive_min_step, float adaptive_max_step,
    float adaptive_field_ref, bool use_adaptive_step, bool render_field_lines)
    : mMagnets(magnets) {
    updateBounds(bounds_width, bounds_height, bounds_depth);
    setTraceConfig(step_size, max_steps, adaptive_min_step, adaptive_max_step, adaptive_field_ref, use_adaptive_step, render_field_lines);
}

void FieldLineTracer::updateBounds(float width, float height, float depth) {
    cuboid_bounds_min = glm::vec3(-width / 2.0f, -height / 2.0f, -depth / 2.0f);
    cuboid_bounds_max = glm::vec3(width / 2.0f, height / 2.0f, depth / 2.0f);
}

void FieldLineTracer::setTraceConfig(float step_size, int max_steps, float adaptive_min_step, float adaptive_max_step,
    float adaptive_field_ref, bool use_adaptive_step, bool render_field_lines) {
    m_step_size = step_size;
    m_max_steps = max_steps;
    m_adaptive_min_step = adaptive_min_step;
    m_adaptive_max_step = adaptive_max_step;
    m_adaptive_field_ref = adaptive_field_ref;
    m_use_adaptive_step = use_adaptive_step;
    m_render_field_lines = render_field_lines;
}

glm::vec3 FieldLineTracer::calculateTotalField(const glm::vec3& pos) const {
    glm::vec3 totalField(0.0f);
    for (const auto* magnet : mMagnets) {
        totalField += magnet->calculateMagneticField(pos);
    }
    return totalField;
}

bool FieldLineTracer::isWithinBounds(const glm::vec3& pos) const {
    return pos.x >= cuboid_bounds_min.x && pos.x <= cuboid_bounds_max.x &&
        pos.y >= cuboid_bounds_min.y && pos.y <= cuboid_bounds_max.y &&
        pos.z >= cuboid_bounds_min.z && pos.z <= cuboid_bounds_max.z;
}

float FieldLineTracer::calculateAdaptiveStepSize(const glm::vec3& field) const {
    float fieldStrength = glm::length(field);
    if (fieldStrength < 1e-6f) return m_adaptive_max_step; // Avoid division by zero
    // Scale step size inversely with field strength
    float step = m_adaptive_field_ref / fieldStrength * m_step_size;
    return std::min(std::max(step, m_adaptive_min_step), m_adaptive_max_step);
}

void FieldLineTracer::traceFieldLineFromPoint(const glm::vec3& startPos, TraceDirection direction, FieldLine& line) {
    glm::vec3 pos = startPos;
    int steps = 0;
    bool forward = (direction == TraceDirection::Forward || direction == TraceDirection::Both);
    float dirMultiplier = forward ? 1.0f : -1.0f;

    while (steps < m_max_steps && isWithinBounds(pos)) {
        // Calculate magnetic field
        glm::vec3 field = calculateTotalField(pos);
        float fieldMagnitude = glm::length(field);
        if (fieldMagnitude < 1e-6f) break; // Stop if field is too weak
        glm::vec3 fieldDir = field / fieldMagnitude;

        // Determine step size
        float dt = m_use_adaptive_step ? calculateAdaptiveStepSize(field) : m_step_size;

        // 4th-order Runge-Kutta integration
        glm::vec3 k1 = dirMultiplier * fieldDir;
        glm::vec3 pos2 = pos + (dt / 2.0f) * k1;
        if (!isWithinBounds(pos2)) break;
        glm::vec3 field2 = calculateTotalField(pos2);
        if (glm::length(field2) < 1e-6f) break;
        glm::vec3 k2 = dirMultiplier * field2 / glm::length(field2);

        glm::vec3 pos3 = pos + (dt / 2.0f) * k2;
        if (!isWithinBounds(pos3)) break;
        glm::vec3 field3 = calculateTotalField(pos3);
        if (glm::length(field3) < 1e-6f) break;
        glm::vec3 k3 = dirMultiplier * field3 / glm::length(field3);

        glm::vec3 pos4 = pos + dt * k3;
        if (!isWithinBounds(pos4)) break;
        glm::vec3 field4 = calculateTotalField(pos4);
        if (glm::length(field4) < 1e-6f) break;
        glm::vec3 k4 = dirMultiplier * field4 / glm::length(field4);

        // Update position
        glm::vec3 delta = (dt / 6.0f) * (k1 + 2.0f * k2 + 2.0f * k3 + k4);
        pos += delta;

        // Store point
        FieldLinePoint point;
        point.position = pos;
        point.field = field;
        if (forward) {
            line.points.push_back(point);
        }
        else {
            line.points.insert(line.points.begin(), point);
        }

        steps++;
    }
}

std::vector<FieldLine> FieldLineTracer::traceFieldLines() {
    std::vector<FieldLine> fieldLines;
    std::mutex fieldLinesMutex;

    // Collect all start points
    struct StartPointInfo {
        glm::vec3 position;
        TraceDirection direction;
    };
    std::vector<StartPointInfo> allStartPoints;
    for (const auto* magnet : mMagnets) {
        const auto& startPoints = magnet->getTraceStartPoints();
        for (const auto& startPoint : startPoints) {
            allStartPoints.push_back({ startPoint.position, startPoint.direction });
        }
    }

    if (allStartPoints.empty()) {
        return fieldLines;
    }

    // Determine number of threads (minimum of hardware concurrency and number of start points)
    unsigned int numThreads = std::min(std::thread::hardware_concurrency(), (unsigned int)allStartPoints.size());
    numThreads = std::max(1u, numThreads); // Ensure at least one thread
    std::vector<std::thread> threads;
    std::vector<std::vector<FieldLine>> threadResults(numThreads);

    // Divide start points among threads
    size_t pointsPerThread = (allStartPoints.size() + numThreads - 1) / numThreads;
    for (unsigned int i = 0; i < numThreads; ++i) {
        size_t startIdx = i * pointsPerThread;
        size_t endIdx = std::min(startIdx + pointsPerThread, allStartPoints.size());

        if (startIdx < allStartPoints.size()) {
            threads.emplace_back([this, startIdx, endIdx, &allStartPoints, &threadResults, i, &fieldLinesMutex]() {
                std::vector<FieldLine>& localLines = threadResults[i];
                for (size_t j = startIdx; j < endIdx; ++j) {
                    const auto& startPoint = allStartPoints[j];
                    FieldLine line;
                    // Trace backward if specified
                    if (startPoint.direction == TraceDirection::Backward || startPoint.direction == TraceDirection::Both) {
                        traceFieldLineFromPoint(startPoint.position, TraceDirection::Backward, line);
                    }
                    // Add the starting point
                    FieldLinePoint start;
                    start.position = startPoint.position;
                    start.field = calculateTotalField(startPoint.position);
                    line.points.push_back(start);
                    // Trace forward if specified
                    if (startPoint.direction == TraceDirection::Forward || startPoint.direction == TraceDirection::Both) {
                        traceFieldLineFromPoint(startPoint.position, TraceDirection::Forward, line);
                    }
                    if (!line.points.empty()) {
                        localLines.push_back(std::move(line));
                    }
                }
                });
        }
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Merge results from all threads
    for (const auto& localLines : threadResults) {
        fieldLines.insert(fieldLines.end(), localLines.begin(), localLines.end());
    }

    return fieldLines;
}