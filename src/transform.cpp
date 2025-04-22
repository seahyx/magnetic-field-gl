#include "transform.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <algorithm>

Transform::Transform(glm::vec3 localPosition, glm::vec3 localEulerRotation, Transform* parent)
    : mLocalPosition(localPosition)
    , parent(parent)
{
    // Convert Euler angles to quaternion
    setLocalRotationEuler(localEulerRotation);

    // Initialize the global transform matrix
    updateWorldTransformMatrix();

    // Add to parent's children if parent exists
    if (parent) {
        parent->addChild(this);
    }
}

void Transform::setLocalPosition(glm::vec3 localPosition)
{
    mLocalPosition = localPosition;
    updateWorldTransformMatrix();
}

glm::vec3 Transform::getLocalPosition() const
{
    return mLocalPosition;
}

void Transform::setWorldPosition(glm::vec3 globalPosition)
{
    if (parent) {
        // Convert global position to local position based on parent's transform
        glm::mat4 parentInverseGlobal = glm::inverse(parent->getWorldTransformMatrix());
        glm::vec4 localPos = parentInverseGlobal * glm::vec4(globalPosition, 1.0f);
        mLocalPosition = glm::vec3(localPos);
    }
    else {
        // No parent, so global position is the same as local position
        mLocalPosition = globalPosition;
    }

    updateWorldTransformMatrix();
}

glm::vec3 Transform::getWorldPosition() const
{
    glm::vec4 globalPos = getWorldTransformMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    return glm::vec3(globalPos);
}

void Transform::setLocalRotation(glm::quat localRotation)
{
    mLocalRotation = localRotation;
    updateWorldTransformMatrix();
}

glm::quat Transform::getLocalRotation() const
{
    return mLocalRotation;
}

void Transform::setWorldRotation(glm::quat globalRotation)
{
    if (parent) {
        // Calculate local rotation by combining inverse of parent rotation with global rotation
        glm::quat parentGlobalRotation = parent->getWorldRotation();
        mLocalRotation = glm::inverse(parentGlobalRotation) * globalRotation;
    }
    else {
        // No parent, so global rotation is the same as local rotation
        mLocalRotation = globalRotation;
    }

    updateWorldTransformMatrix();
}

glm::quat Transform::getWorldRotation() const
{
    if (parent) {
        // Combine parent's global rotation with local rotation
        return parent->getWorldRotation() * mLocalRotation;
    }
    else {
        // No parent, so global rotation is the same as local rotation
        return mLocalRotation;
    }
}

void Transform::setLocalRotationEuler(glm::vec3 localRotationEuler)
{
    // Convert Euler angles (in degrees) to quaternion
    mLocalRotation = glm::quat(glm::radians(localRotationEuler));
    updateWorldTransformMatrix();
}

void Transform::setWorldRotationEuler(glm::vec3 globalRotationEuler)
{
    // Convert Euler angles to quaternion and set as global rotation
    glm::quat globalRotation = glm::quat(glm::radians(globalRotationEuler));
    setWorldRotation(globalRotation);
}

// Rotate around a world point
void Transform::rotateAround(const glm::vec3& worldPoint, const glm::vec3& axis, float angleDegrees)
{
    // Get the current world position
    glm::vec3 worldPos = getWorldPosition();

    // Calculate vector from pivot to object
    glm::vec3 dirFromPivot = worldPos - worldPoint;

    // Create rotation quaternion
    glm::quat rotation = glm::angleAxis(glm::radians(angleDegrees), glm::normalize(axis));

    // Rotate the direction vector
    glm::vec3 rotatedDir = rotation * dirFromPivot;

    // Calculate new world position
    glm::vec3 newWorldPos = worldPoint + rotatedDir;

    // Apply the new position and rotation
    setWorldPosition(newWorldPos);
    setWorldRotation(rotation * getWorldRotation());
}

// Rotate around local axis
void Transform::rotateAxis(const glm::vec3& axis, float angleDegrees)
{
    // Create a rotation quaternion for the given axis and angle
    glm::quat rotation = glm::angleAxis(glm::radians(angleDegrees), glm::normalize(axis));

    // Combine with existing local rotation
    mLocalRotation = mLocalRotation * rotation;

    // Update transform matrices
    updateWorldTransformMatrix();
}

// Look at a target position
void Transform::lookAt(const glm::vec3& target, const glm::vec3& worldUp)
{
    glm::vec3 position = getWorldPosition();

    // Calculate forward direction (from position to target)
    glm::vec3 forward = glm::normalize(target - position);

    // Calculate right vector
    glm::vec3 right = glm::normalize(glm::cross(worldUp, forward));

    // Recalculate up vector to ensure orthogonal basis
    glm::vec3 up = glm::cross(forward, right);

    // Create rotation matrix from the orthogonal basis
    glm::mat3 rotationMatrix(right, up, forward);

    // Convert to quaternion and set as world rotation
    glm::quat worldRotation = glm::quat_cast(rotationMatrix);
    setWorldRotation(worldRotation);
}

// Translate in world space
void Transform::translate(const glm::vec3& offset)
{
    setWorldPosition(getWorldPosition() + offset);
}

// Translate in local space
void Transform::translateLocal(const glm::vec3& localOffset)
{
    // Convert local offset to world offset using rotation
    glm::vec3 worldOffset = getWorldRotation() * localOffset;

    // Apply world offset
    translate(worldOffset);
}

// Transform direction from local to world space
glm::vec3 Transform::transformDirection(const glm::vec3& direction)
{
    return getWorldRotation() * direction;
}

// Transform direction from world to local space
glm::vec3 Transform::inverseTransformDirection(const glm::vec3& worldDirection)
{
    return glm::inverse(getWorldRotation()) * worldDirection;
}

// Transform point from local to world space
glm::vec3 Transform::transformPoint(const glm::vec3& point)
{
    glm::vec4 result = getWorldTransformMatrix() * glm::vec4(point, 1.0f);
    return glm::vec3(result);
}

// Transform point from world to local space
glm::vec3 Transform::inverseTransformPoint(const glm::vec3& worldPoint)
{
    glm::vec4 result = glm::inverse(getWorldTransformMatrix()) * glm::vec4(worldPoint, 1.0f);
    return glm::vec3(result);
}

void Transform::setParent(Transform* newParent)
{
    // If we already have a parent, remove ourselves from its children
    if (parent) {
        parent->removeChild(this);
    }

    // Store current global position and rotation
    glm::vec3 globalPos = getWorldPosition();
    glm::quat globalRot = getWorldRotation();

    // Update parent
    parent = newParent;

    // Add ourselves to new parent's children if it exists
    if (parent) {
        parent->addChild(this);
    }

    // Restore global position and rotation with new parent
    setWorldPosition(globalPos);
    setWorldRotation(globalRot);
}

Transform* Transform::getParent() const
{
    return parent;
}

// Add child to this transform
void Transform::addChild(Transform* child)
{
    // Check if child is already in the list
    if (std::find(mChildren.begin(), mChildren.end(), child) == mChildren.end()) {
        mChildren.push_back(child);
    }
}

// Remove child from this transform
void Transform::removeChild(Transform* child)
{
    auto it = std::find(mChildren.begin(), mChildren.end(), child);
    if (it != mChildren.end()) {
        mChildren.erase(it);
    }
}

// Get number of children
int Transform::getChildCount() const
{
    return static_cast<int>(mChildren.size());
}

// Get child by index
Transform* Transform::getChild(int index)
{
    if (index >= 0 && index < mChildren.size()) {
        return mChildren[index];
    }
    return nullptr;
}

glm::mat4 Transform::getLocalTransformMatrix() const
{
    // Build local transform matrix from position and rotation
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), mLocalPosition);
    glm::mat4 rotationMatrix = glm::toMat4(mLocalRotation);

    return translationMatrix * rotationMatrix;
}

glm::mat4 Transform::getWorldTransformMatrix() const
{
    return mWorldTransformMatrix;
}

void Transform::updateWorldTransformMatrix()
{
    if (parent) {
        // Global transform is parent's global transform multiplied by local transform
        mWorldTransformMatrix = parent->getWorldTransformMatrix() * getLocalTransformMatrix();
    }
    else {
        // No parent, so global transform is the same as local transform
        mWorldTransformMatrix = getLocalTransformMatrix();
    }

    // Update all children
    for (auto& child : mChildren) {
        child->updateWorldTransformMatrix();
    }
}

// Get direction vectors
// Forward, right, and up vectors in world space
glm::vec3 Transform::getForward() const
{
    // Forward is typically the negative Z axis in most 3D systems
    return getWorldRotation() * glm::vec3(0.0f, 0.0f, -1.0f);
}

glm::vec3 Transform::getRight() const
{
    // Right is the positive X axis
    return getWorldRotation() * glm::vec3(1.0f, 0.0f, 0.0f);
}

glm::vec3 Transform::getUp() const
{
    // Up is the positive Y axis
    return getWorldRotation() * glm::vec3(0.0f, 1.0f, 0.0f);
}