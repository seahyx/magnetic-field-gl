#include "dipole.h"
#include <glm/gtc/constants.hpp>

constexpr auto PIXELS_PER_METER = 100.0f;

// Constructor that uses default forward direction (0,0,-1)
MagneticDipole::MagneticDipole(const glm::vec3& position, float moment, Transform* parent)
    : Transform(position, glm::vec3(0.0f), parent), // Default rotation
    mMoment(moment)
{
}

// Constructor that initializes with a specific direction
MagneticDipole::MagneticDipole(const glm::vec3& position, const glm::vec3& initialDirection, float moment, Transform* parent)
    : Transform(position, glm::vec3(0.0f), parent), // Will set proper rotation below
    mMoment(moment)
{
    // Set the direction using the lookAt method
    setDirection(initialDirection);
}

void MagneticDipole::setDirection(const glm::vec3& direction) {
    if (glm::length(direction) > 0.0001f) {
        // Use lookAt to orient the dipole in the direction
        // We look from the current position toward position + direction
        glm::vec3 targetPos = getWorldPosition() + glm::normalize(direction);
        lookAt(targetPos);
    }
}

glm::vec3 MagneticDipole::getDirection() const {
    // Use the forward direction of the transform
    return getForward();
}

void MagneticDipole::setMoment(float moment) {
    mMoment = moment;
}

float MagneticDipole::getMoment() const {
    return mMoment;
}

glm::vec3 MagneticDipole::calculateMagneticField(const glm::vec3& pos) const {
    // Get the world position of the dipole using the Transform functionality
    glm::vec3 dipoleWorldPos = getWorldPosition();

    // Get the dipole direction from the transform's forward vector
    glm::vec3 dipoleDirection = getDirection();

    // Calculate the field based on the world position
    glm::vec3 A = pos - dipoleWorldPos; // Relative position of pos from dipole
    float r = glm::length(A);

    if (r < 0.0001f) return glm::vec3(0.0f); // Avoid divide-by-zero

    glm::vec3 A_rad = A / r; // Radial direction of pos from dipole

    // Create a perpendicular vector in 3D
	// We need to create a tangential plane - first get a non-parallel vector
    // Find a vector in any of the three axes that is the least parallel to A_rad
	// for numerical stability when calculating the cross product
    // (to obtain a truly perpendicular vector)
    glm::vec3 perpAxis;
    if (std::abs(A_rad.x) < std::abs(A_rad.y) && std::abs(A_rad.x) < std::abs(A_rad.z)) {
        perpAxis = glm::vec3(1.0f, 0.0f, 0.0f); // x-axis
    }
    else if (std::abs(A_rad.y) < std::abs(A_rad.z)) {
        perpAxis = glm::vec3(0.0f, 1.0f, 0.0f); // y-axis
    }
    else {
        perpAxis = glm::vec3(0.0f, 0.0f, 1.0f); // z-axis
    }

    // Create an orthogonal basis in the tangential plane
    glm::vec3 A_tan1 = glm::normalize(glm::cross(A_rad, perpAxis));
    glm::vec3 A_tan2 = glm::cross(A_rad, A_tan1); // Another perpendicular vector

    float cos_theta = glm::dot(A_rad, dipoleDirection); // Component of dipole in radial direction

    // Project the dipole direction onto the tangential plane
    glm::vec3 dirTangential = dipoleDirection - cos_theta * A_rad;
    float sin_theta_mag = glm::length(dirTangential);

    // Convert to meters for field calculation
    float r_meters = r / PIXELS_PER_METER;
    float r_cube = r_meters * r_meters * r_meters;

    // Calculate field components
    float B_r = (2.0f * mMoment * cos_theta) / r_cube; // Radial field magnitude

    glm::vec3 B_field = B_r * A_rad; // Start with radial component

    // Add tangential component if it exists
    if (sin_theta_mag > 0.0001f) {
        glm::vec3 A_tan_dir = dirTangential / sin_theta_mag;
        float B_theta = (mMoment * sin_theta_mag) / r_cube; // Tangential field magnitude
        B_field += B_theta * A_tan_dir;
    }

    return B_field;
}