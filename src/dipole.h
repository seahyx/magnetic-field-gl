#pragma once

#include "transform.h"
#include <glm/glm.hpp>

// Magnetic dipole is the smallest unit of magnet
class MagneticDipole : public Transform {
public:
    // Constructor now only takes position, moment, and optional parent
    // Direction is handled by Transform's rotation
    MagneticDipole(const glm::vec3& position, float moment, Transform* parent = nullptr);
    MagneticDipole(const glm::vec3& position, const glm::vec3& initialDirection, float moment, Transform* parent = nullptr);

    // Moment setters/getters (position and direction are handled by Transform)
    void setMoment(float moment);
    float getMoment() const;

    // Direction now uses transform's forward vector
    void setDirection(const glm::vec3& direction);
    glm::vec3 getDirection() const;

    // Method to calculate the magnetic field at a given position
    glm::vec3 calculateMagneticField(const glm::vec3& pos) const;

private:
    float mMoment;          // Magnetic moment
};