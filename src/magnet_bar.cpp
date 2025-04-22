#include "magnet_bar.h"
#include <algorithm>
#include <glm/gtc/constants.hpp>

BarMagnet::BarMagnet(const glm::vec3& position, const glm::vec3& size, float dipoleDensity, float momentPerDipole, Transform* parent, float pixelsPerMeter)
    : Transform(position, glm::vec3(0.0f), parent)
	, BaseMagnet(pixelsPerMeter)
    , mSize(size)
    , mDipoleDensity(dipoleDensity)
    , mMomentPerDipole(momentPerDipole)
{
    // Ensure size and density are positive
    mSize = glm::max(size, glm::vec3(0.001f));
    mDipoleDensity = std::max(dipoleDensity, 0.001f);

    // Initialize dipoles
    updateDipoles();

    // Initialize trace start points
    initializeTraceStartPoints();
}

BarMagnet::~BarMagnet() {
    // Clean up dynamically allocated dipoles
    for (auto dipole : mDipoles) {
        delete dipole;
    }
    mDipoles.clear();
}

void BarMagnet::setSize(const glm::vec3& size) {
    // Ensure size is positive
    mSize = glm::max(size, glm::vec3(0.001f));
    updateDipoles();
    initializeTraceStartPoints();
}

void BarMagnet::setDipoleDensity(float density) {
    // Ensure density is positive
    mDipoleDensity = std::max(density, 0.001f);
    updateDipoles();
    initializeTraceStartPoints();
}

void BarMagnet::updateDipoles() {
    // Clean up existing dipoles
    for (auto dipole : mDipoles) {
        delete dipole;
    }
    mDipoles.clear();

    // Convert size to meters
    glm::vec3 sizeMeters = mSize / mPixelsPerMeter;

    // Calculate number of dipoles along each axis
    glm::ivec3 numDipoles = glm::max(glm::ivec3(sizeMeters * mDipoleDensity), glm::ivec3(1));

    // Calculate spacing between dipoles in meters
    glm::vec3 spacing = sizeMeters / glm::vec3(numDipoles);

    // Create dipoles
    for (int x = 0; x < numDipoles.x; ++x) {
        for (int y = 0; y < numDipoles.y; ++y) {
            for (int z = 0; z < numDipoles.z; ++z) {
                // Calculate local position of dipole (centered around origin)
                glm::vec3 localPos = (glm::vec3(x, y, z) + 0.5f) * spacing - sizeMeters * 0.5f;
                // Convert back to pixels
                localPos *= mPixelsPerMeter;

                // Create dipole with forward direction along magnet's forward vector
                MagneticDipole* dipole = new MagneticDipole(localPos, mMomentPerDipole, this);
                mDipoles.push_back(dipole);
            }
        }
    }
}

void BarMagnet::initializeTraceStartPoints() {
    mTraceStartPoints.clear();

    constexpr int NUM_POINTS = 5; // Number of trace points across width
    constexpr float SPACING = 0.2f; // Spacing factor relative to width

    // Calculate normalized width direction (along y-axis)
    glm::vec3 normWidthDir = getRight();

    // Calculate step size for points
    float step = mSize.y * SPACING / (NUM_POINTS - 1);

    // Generate points centered around the magnet's center
    for (int i = 0; i < NUM_POINTS; ++i) {
        float offset = (i - (NUM_POINTS - 1) / 2.0f) * step;
        glm::vec3 position = getWorldPosition() + offset * normWidthDir;

        TraceStartPoint point;
        point.position = position;
        point.direction = TraceDirection::Both;
        mTraceStartPoints.push_back(point);
    }
}

glm::vec3 BarMagnet::calculateMagneticField(const glm::vec3& pos) const {
    glm::vec3 totalField(0.0f);

    // Sum magnetic field contributions from all dipoles
    for (const auto dipole : mDipoles) {
        totalField += dipole->calculateMagneticField(pos);
    }

    return totalField;
}