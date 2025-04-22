#include "dipole.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

constexpr auto PIXELS_PER_METER = 100.0f;

MagneticDipole::MagneticDipole(const glm::vec2& position, const glm::vec2& direction, float moment)
    : mPosition(position), mDirection(glm::normalize(direction)), mMoment(moment) {}

void MagneticDipole::setPosition(const glm::vec2& position) {
    mPosition = position;
}

void MagneticDipole::setDirection(const glm::vec2& direction) {
    mDirection = glm::normalize(direction);
}

void MagneticDipole::setMoment(float moment) {
    mMoment = moment;
}

glm::vec2 MagneticDipole::getPosition() const {
    return mPosition;
}

glm::vec2 MagneticDipole::getDirection() const {
    return mDirection;
}

float MagneticDipole::getMoment() const {
    return mMoment;
}

glm::vec2 MagneticDipole::calculateMagneticField(const glm::vec2& pos) const {
    glm::vec2 A = pos - mPosition; // Relative position of pos from dipole
    float r = glm::length(A);
    if (r == 0.0) return glm::vec2(0.0f, 0.0f); // Avoid divide-by-zero
    glm::vec2 A_rad = A / r; // Radial direction of pos from dipole
    glm::vec2 A_tan = glm::vec2(A_rad.y, -A_rad.x); // Clockwise tangential direction

    float cos_theta = glm::dot(A_rad, mDirection); // Component of dipole moment in radial unit vector
    float sin_theta = glm::dot(A_tan, mDirection); // Since A_tan is perpendicular to A_rad

    r /= PIXELS_PER_METER;
    float r_cube = r * r * r;

    // We skip the constants, assume pre-multiplied
    float B_r = (2 * mMoment * cos_theta) / r_cube; // Radial field magnitude
    float B_theta = (mMoment * sin_theta) / r_cube; // Tangential field magnitude

    return B_r * A_rad + B_theta * A_tan;
}
