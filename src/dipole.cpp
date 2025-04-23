#include "dipole.h"
#include <glm/gtc/constants.hpp>

// Constructor that uses default forward direction (0,0,-1)
MagneticDipole::MagneticDipole(const glm::vec3& position, float moment, Transform* parent, float pixelsPerMeter)
    : Transform(position, glm::vec3(0.0f), parent) // Default rotation
    , BaseMagnet(pixelsPerMeter)
    , mMoment(moment)
{
    initializeDipoleTracePoints();
}

// Constructor that initializes with a specific direction
MagneticDipole::MagneticDipole(const glm::vec3& position, const glm::vec3& initialDirection, float moment, Transform* parent, float pixelsPerMeter)
    : Transform(position, glm::vec3(0.0f), parent) // Will set proper rotation below
    , BaseMagnet(pixelsPerMeter)
    , mMoment(moment)
{
    setDirection(initialDirection);
}

void MagneticDipole::updateWorldTransformMatrix()
{
    Transform::updateWorldTransformMatrix();
    initializeDipoleTracePoints();
}

void MagneticDipole::initializeDipoleTracePoints() {
    mTraceStartPoints.clear();

    constexpr int NUM_POINTS = 6; // Number of points in circle
    float radius = 0.04f; // Small radius in meters

    glm::vec3 center = getWorldPosition();
    glm::vec3 up = getUp();
    glm::vec3 right = getRight();

    // Create points in a circle perpendicular to the dipole direction
    for (int i = 0; i < NUM_POINTS; ++i) {
        float angle = i * 2.0f * glm::pi<float>() / NUM_POINTS;
        glm::vec3 offset = (cos(angle) * right + sin(angle) * up) * radius;

        TraceStartPoint point;
        point.position = center + offset;
        point.direction = TraceDirection::Both;
        mTraceStartPoints.push_back(point);
    }
}

void MagneticDipole::setDirection(const glm::vec3& direction) {
    if (glm::length(direction) > 0.0001f) {
        // Use lookAt to orient the dipole in the direction
        glm::vec3 targetPos = getWorldPosition() + glm::normalize(direction);
        lookAt(targetPos);
        initializeDipoleTracePoints(); // Update trace points when direction changes
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
    float r_meters = r / mPixelsPerMeter;
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