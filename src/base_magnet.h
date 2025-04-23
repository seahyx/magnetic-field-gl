#pragma once

#include <glm/glm.hpp>
#include <vector>

enum class TraceDirection {
	Forward,
	Backward,
	Both
};

typedef struct {
	glm::vec3 position; // Start position of the magnetic field line trace
	TraceDirection direction; // Direction of the trace (forward, backward, or both)
} TraceStartPoint;

class BaseMagnet {
public:
	BaseMagnet(float pixelsPerMeter) : mPixelsPerMeter(pixelsPerMeter) {}
    virtual ~BaseMagnet() = default;

    // Pure virtual function for calculating magnetic field
    virtual glm::vec3 calculateMagneticField(const glm::vec3& pos) const = 0;

    // Get the trace start points
    const std::vector<TraceStartPoint>& getTraceStartPoints() const { return mTraceStartPoints; }

	float mPixelsPerMeter = 100.0f; // Conversion factor from meters to pixels

protected:
    std::vector<TraceStartPoint> mTraceStartPoints;
};