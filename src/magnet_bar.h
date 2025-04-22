#pragma once

#include "dipole.h"
#include "base_magnet.h"
#include <vector>
#include <glm/glm.hpp>

class BarMagnet : public Transform, public BaseMagnet {
public:
    // Constructor initializes bar magnet with size (length, width, height) and dipole density (dipoles per meter)
    BarMagnet(const glm::vec3& position, const glm::vec3& size, float dipoleDensity, float momentPerDipole, Transform* parent = nullptr);

    // Destructor to clean up dipoles
    ~BarMagnet();

    // Setters for size and density that update the dipole list
    void setSize(const glm::vec3& size);
    void setDipoleDensity(float density);

    // Getters
    glm::vec3 getSize() const { return mSize; }
    float getDipoleDensity() const { return mDipoleDensity; }
    float getMomentPerDipole() const { return mMomentPerDipole; }
    const std::vector<MagneticDipole*>& getDipoles() const { return mDipoles; }

    // Calculate the total magnetic field at a given position by summing contributions from all dipoles
    glm::vec3 calculateMagneticField(const glm::vec3& pos) const override;

private:
    // Helper method to rebuild the dipole list based on current size and density
    void updateDipoles();
    // Helper method to initialize trace start points across width
    void initializeTraceStartPoints();

    glm::vec3 mSize;                   // Size of the bar magnet (length, width, height)
    float mDipoleDensity;              // Dipoles per meter
    float mMomentPerDipole;            // Magnetic moment for each dipole
    std::vector<MagneticDipole*> mDipoles; // List of dipoles
};