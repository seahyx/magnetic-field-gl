#include "MagneticDipole.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#define PIXELS_PER_METER 100.0f

MagneticDipole::MagneticDipole(const glm::vec2& position, const glm::vec2& direction, float moment)
    : sel_pos(position), sel_dir(glm::normalize(direction)), moment(moment) {}

void MagneticDipole::setPosition(const glm::vec2& position) {
    sel_pos = position;
}

void MagneticDipole::setDirection(const glm::vec2& direction) {
    sel_dir = glm::normalize(direction);
}

void MagneticDipole::setMoment(float moment) {
    this->moment = moment;
}

glm::vec2 MagneticDipole::getPosition() const {
    return sel_pos;
}

glm::vec2 MagneticDipole::getDirection() const {
    return sel_dir;
}

float MagneticDipole::getMoment() const {
    return moment;
}

glm::vec2 MagneticDipole::calculateMagneticField(const glm::vec2& pos) const {
    glm::vec2 A = pos - sel_pos;
    glm::vec2 A_rad = glm::normalize(A);
    glm::vec2 A_tan = glm::vec2(A_rad.y, -A_rad.x); // Clockwise tangent
    float r = glm::length(A);
    float theta = acos(glm::dot(A, sel_dir) / r);
    r /= PIXELS_PER_METER;
    float r_cube = pow(r, 3);

    // We skip the constants, assume pre-multiplied
    float B_r = (2 * moment * cos(theta)) / r_cube; // Radial field vector
    float B_theta = (moment * sin(theta)) / r_cube; // Tangential field vector

    return B_r * A_rad + B_theta * A_tan;
}

std::vector<float> MagneticDipole::traceFieldLine(const MagneticDipole& dipole, glm::vec2 start, float stepSize, int steps) const
{
    std::vector<float> vertices; // x, y per point
    glm::vec2 pos = start;
    for (int i = 0; i < steps; i++) {
        vertices.push_back(pos.x);
        vertices.push_back(pos.y);
        glm::vec2 B = dipole.calculateMagneticField(pos);
        float magB = glm::length(B);
        if (magB < 1e-6f) break; // Avoid tiny fields
        pos += stepSize * (B / magB); // Normalized step
    }
    return vertices;
}
