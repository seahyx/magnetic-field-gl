#pragma once

#include "transform.h"
#include <glm/glm.hpp>

class Camera : public Transform {
public:
    enum class ProjectionMode {
        Perspective,
        Orthographic
    };

    // Constructor initializes camera with position, target, and projection parameters
    Camera(const glm::vec3& position, const glm::vec3& target, float nearPlane = 0.1f, float farPlane = 100.0f, Transform* parent = nullptr);

    // Set projection mode and associated parameters
    void setPerspective(float fovY, float aspectRatio, bool transition = false);
    void setOrthographic(float orthoSize, float aspectRatio, bool transition = false);
    void setProjectionMode(ProjectionMode mode);

    // Transition methods
    void startTransition(ProjectionMode targetMode, float duration);
    void updateTransition(float deltaTime);

    // Getters for projection parameters
    ProjectionMode getProjectionMode() const { return mProjectionMode; }
    float getFovY() const { return mFovY; }
    float getOrthoSize() const { return mOrthoSize; }
    float getAspectRatio() const { return mAspectRatio; }
    float getNearPlane() const { return mNearPlane; }
    float getFarPlane() const { return mFarPlane; }
    bool isTransitioning() const { return mIsTransitioning; }
    float getTransitionFactor() const { return mTransitionFactor; }

    // Setters for clipping planes
    void setNearPlane(float near) { mNearPlane = near; }
    void setFarPlane(float far) { mFarPlane = far; }

    // Compute view and projection matrices
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

    // Utility to set camera to look at a target
    void lookAt(const glm::vec3& target, const glm::vec3& worldUp = glm::vec3(0.0f, 1.0f, 0.0f));

private:
    ProjectionMode mProjectionMode; // Current projection mode
    float mFovY;                   // Vertical field of view (radians) for perspective
    float mOrthoSize;              // Half-height of orthographic view volume
    float mAspectRatio;            // Width / height of viewport
    float mNearPlane;              // Near clipping plane
    float mFarPlane;               // Far clipping plane

    // Transition state
    bool mIsTransitioning;         // Whether a transition is in progress
    float mTransitionFactor;       // 0.0 (perspective) to 1.0 (orthographic)
    float mTransitionDuration;     // Duration of transition in seconds
    float mTransitionTime;         // Current time in transition
    ProjectionMode mTargetMode;    // Target mode for transition
};