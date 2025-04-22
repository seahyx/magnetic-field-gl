#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

Camera::Camera(const glm::vec3& position, const glm::vec3& target, float nearPlane, float farPlane, Transform* parent)
    : Transform(position, glm::vec3(0.0f), parent)
    , mProjectionMode(ProjectionMode::Perspective)
    , mFovY(glm::radians(60.0f)) // Default 60-degree FOV
    , mOrthoSize(5.0f)          // Default orthographic half-height
    , mAspectRatio(1.0f)        // Default square aspect ratio
    , mNearPlane(nearPlane)
    , mFarPlane(farPlane)
    , mIsTransitioning(false)
    , mTransitionFactor(0.0f)
    , mTransitionDuration(0.0f)
    , mTransitionTime(0.0f)
    , mTargetMode(ProjectionMode::Perspective)
{
    // Orient camera to look at target
    lookAt(target);
}

void Camera::setPerspective(float fovY, float aspectRatio) {
    mProjectionMode = ProjectionMode::Perspective;
    mFovY = glm::radians(fovY); // Convert degrees to radians
    mAspectRatio = aspectRatio;
    mIsTransitioning = false; // Reset transition state
    mTransitionFactor = 0.0f;
}

void Camera::setOrthographic(float orthoSize, float aspectRatio) {
    mProjectionMode = ProjectionMode::Orthographic;
    mOrthoSize = orthoSize;
    mAspectRatio = aspectRatio;
    mIsTransitioning = false; // Reset transition state
    mTransitionFactor = 1.0f;
}

void Camera::setProjectionMode(ProjectionMode mode) {
    mProjectionMode = mode;
    mIsTransitioning = false;
    mTransitionFactor = (mode == ProjectionMode::Perspective) ? 0.0f : 1.0f;
}

void Camera::startTransition(ProjectionMode targetMode, float duration) {
    if (targetMode == mProjectionMode && !mIsTransitioning) {
        return; // No transition needed
    }

    mTargetMode = targetMode;
    mTransitionDuration = std::max(duration, 0.001f); // Avoid zero duration
    mTransitionTime = 0.0f;
    mIsTransitioning = true;

    // Set initial transition factor based on current mode
    mTransitionFactor = (mProjectionMode == ProjectionMode::Perspective) ? 0.0f : 1.0f;
}

void Camera::updateTransition(float deltaTime) {
    if (!mIsTransitioning) {
        return;
    }

    // Update transition time
    mTransitionTime += deltaTime;
    float t = mTransitionTime / mTransitionDuration;
    t = std::clamp(t, 0.0f, 1.0f); // Normalize to [0, 1]

    // Apply ease-out interpolation: f(t) = 1 - (1 - t)^2
    float easedT = 1.0f - (1.0f - t) * (1.0f - t);

    // Determine direction of transition
    float targetFactor = (mTargetMode == ProjectionMode::Perspective) ? 0.0f : 1.0f;
    float startFactor = (mProjectionMode == ProjectionMode::Perspective) ? 0.0f : 1.0f;

    // Interpolate transition factor using easedT
    mTransitionFactor = startFactor + (targetFactor - startFactor) * easedT;

    // Check if transition is complete
    if (t >= 1.0f) {
        mIsTransitioning = false;
        mProjectionMode = mTargetMode;
        mTransitionFactor = targetFactor;
    }
}

glm::mat4 Camera::getViewMatrix() const {
    // Camera looks along its negative Z-axis (forward direction)
    glm::vec3 position = getWorldPosition();
    glm::vec3 forward = getForward(); // From Transform: forward is -Z
    glm::vec3 target = position + forward; // Look in the direction of forward
    glm::vec3 up = getUp(); // Use camera's up vector

    return glm::lookAt(position, target, up);
}

glm::mat4 Camera::getProjectionMatrix() const {
    if (!mIsTransitioning) {
        // Non-transitioning case: use standard perspective or orthographic matrix
        if (mProjectionMode == ProjectionMode::Perspective) {
            return glm::perspective(mFovY, mAspectRatio, mNearPlane, mFarPlane);
        }
        else {
            float halfHeight = mOrthoSize;
            float halfWidth = halfHeight * mAspectRatio;
            return glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, mNearPlane, mFarPlane);
        }
    }

    // Transitioning: compute a blended projection matrix
    // Calculate frustum boundaries at the near plane
    float halfHeightPersp = mNearPlane * std::tan(mFovY * 0.5f);
    float halfWidthPersp = halfHeightPersp * mAspectRatio;
    float halfHeightOrtho = mOrthoSize;
    float halfWidthOrtho = halfHeightOrtho * mAspectRatio;

    // Interpolate frustum boundaries
    float t = mTransitionFactor; // 0.0 (perspective) to 1.0 (orthographic)
    float left = glm::mix(-halfWidthPersp, -halfWidthOrtho, t);
    float right = glm::mix(halfWidthPersp, halfWidthOrtho, t);
    float bottom = glm::mix(-halfHeightPersp, -halfHeightOrtho, t);
    float top = glm::mix(halfHeightPersp, halfHeightOrtho, t);

    // Compute perspective effect (scaling factor for vertices)
    float perspEffect = glm::mix(1.0f / mNearPlane, 0.0f, t); // Perspective: 1/z, Ortho: 0

    // Create a custom frustum matrix
    glm::mat4 proj(0.0f);
    proj[0][0] = 2.0f * mNearPlane / (right - left); // x scaling
    proj[2][0] = (right + left) / (right - left);     // x offset
    proj[1][1] = 2.0f * mNearPlane / (top - bottom);  // y scaling
    proj[2][1] = (top + bottom) / (top - bottom);     // y offset
    proj[2][2] = -(mFarPlane + mNearPlane) / (mFarPlane - mNearPlane); // z scaling
    proj[3][2] = -2.0f * mFarPlane * mNearPlane / (mFarPlane - mNearPlane); // z offset
    proj[2][3] = -1.0f; // Perspective term

    if (perspEffect < 0.001f) {
        // Pure orthographic: adjust for orthographic projection
        proj[2][2] = -2.0f / (mFarPlane - mNearPlane);
        proj[3][2] = -(mFarPlane + mNearPlane) / (mFarPlane - mNearPlane);
        proj[2][3] = 0.0f;
        proj[3][3] = 1.0f;
    }
    else {
        // Apply perspective effect
        proj[0][0] *= perspEffect;
        proj[1][1] *= perspEffect;
        proj[2][0] *= perspEffect;
        proj[2][1] *= perspEffect;
    }

    return proj;
}

void Camera::lookAt(const glm::vec3& target, const glm::vec3& worldUp) {
    // Use Transform's lookAt to orient the camera
    Transform::lookAt(target, worldUp);
}