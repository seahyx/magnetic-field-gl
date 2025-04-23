#pragma once

#include "transform.h"
#include "base_magnet.h"
#include <glm/glm.hpp>

// Magnetic dipole is the smallest unit of magnet
class MagneticDipole : public Transform, public BaseMagnet {
public:
    // Constructor now only takes position, moment, and optional parent
    // Direction is handled by Transform's rotation
    MagneticDipole(const glm::vec3& position, float moment, Transform* parent = nullptr, float pixelsPerMeter = 100.0f);
    MagneticDipole(const glm::vec3& position, const glm::vec3& initialDirection, float moment, Transform* parent = nullptr, float pixelsPerMeter = 100.0f);

    // Override Transform methods to update trace points
    void updateWorldTransformMatrix() override;

    // Moment setters/getters (position and direction are handled by Transform)
    void setMoment(float moment);
    float getMoment() const;

    // Direction now uses transform's forward vector
    void setDirection(const glm::vec3& direction);
    glm::vec3 getDirection() const;

    // Method to calculate the magnetic field at a given position
    glm::vec3 calculateMagneticField(const glm::vec3& pos) const override;

private:
    // Helper method to initialize trace start points in a circle
    void initializeDipoleTracePoints();

    float mMoment;          // Magnetic moment
};