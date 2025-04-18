#ifndef MAGNETICDIPOLE_H
#define MAGNETICDIPOLE_H

#include <glm/glm.hpp>
#include <vector>

class MagneticDipole {
public:
    MagneticDipole(const glm::vec2& position, const glm::vec2& direction, float moment);

    // Setters
    void setPosition(const glm::vec2& position);
    void setDirection(const glm::vec2& direction);
    void setMoment(float moment);

    // Getters
    glm::vec2 getPosition() const;
    glm::vec2 getDirection() const;
    float getMoment() const;

    // Method to calculate the magnetic field at a given position
    glm::vec2 calculateMagneticField(const glm::vec2& pos) const;

    // Method to trace the field line of the dipole
    std::vector<float> traceFieldLine(const MagneticDipole& dipole, glm::vec2 start, float stepSize, int steps) const;


private:
    glm::vec2 sel_pos;   // Position of the dipole
    glm::vec2 sel_dir;   // Direction of the dipole
    float moment;        // Magnetic moment
};

#endif