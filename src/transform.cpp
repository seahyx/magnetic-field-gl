#include "transform.h"
#include <iostream>

Transform::Transform(glm::vec3 localPosition, glm::vec3 localEulerRotation, Transform* parent)
    : mLocalPosition(localPosition), parent(parent), mWorldTransformMatrix(1.0f) {
    setLocalRotationEuler(localEulerRotation);
    updateWorldTransformMatrix();
    if (parent) {
        parent->addChild(this);
    }
}

Transform::~Transform() {
    // Remove this transform from its parent's children
    if (parent) {
        parent->removeChild(this);
    }
    // Clear children and set their parent to nullptr
    for (Transform* child : mChildren) {
        if (child) {
            child->parent = nullptr;
        }
    }
    mChildren.clear();
}

void Transform::setLocalPosition(glm::vec3 localPosition) {
    mLocalPosition = localPosition;
    updateWorldTransformMatrix();
}

glm::vec3 Transform::getLocalPosition() const {
    return mLocalPosition;
}

void Transform::setWorldPosition(glm::vec3 globalPosition) {
    if (parent) {
        glm::mat4 parentInverseGlobal = glm::inverse(parent->getWorldTransformMatrix());
        glm::vec4 localPos = parentInverseGlobal * glm::vec4(globalPosition, 1.0f);
        mLocalPosition = glm::vec3(localPos);
    }
    else {
        mLocalPosition = globalPosition;
    }
    updateWorldTransformMatrix();
}

glm::vec3 Transform::getWorldPosition() const {
    glm::vec4 worldPos = getWorldTransformMatrix() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    return glm::vec3(worldPos);
}

void Transform::setLocalRotation(glm::quat localRotation) {
    mLocalRotation = localRotation;
    updateWorldTransformMatrix();
}

glm::quat Transform::getLocalRotation() const {
    return mLocalRotation;
}

void Transform::setWorldRotation(glm::quat globalRotation) {
    if (parent) {
        glm::quat parentGlobalRotation = parent->getWorldRotation();
        mLocalRotation = glm::inverse(parentGlobalRotation) * globalRotation;
    }
    else {
        mLocalRotation = globalRotation;
    }
    updateWorldTransformMatrix();
}

glm::quat Transform::getWorldRotation() const {
    if (parent) {
        return parent->getWorldRotation() * mLocalRotation;
    }
    return mLocalRotation;
}

void Transform::setLocalRotationEuler(glm::vec3 localRotationEuler) {
    mLocalRotation = glm::quat(glm::radians(localRotationEuler));
    updateWorldTransformMatrix();
}

void Transform::setWorldRotationEuler(glm::vec3 globalRotationEuler) {
    glm::quat globalRotation = glm::quat(glm::radians(globalRotationEuler));
    setWorldRotation(globalRotation);
}

void Transform::rotateAround(const glm::vec3& worldPoint, const glm::vec3& axis, float angleDegrees) {
    glm::vec3 worldPos = getWorldPosition();
    glm::vec3 dirFromPivot = worldPos - worldPoint;
    glm::quat rotation = glm::angleAxis(glm::radians(angleDegrees), glm::normalize(axis));
    glm::vec3 rotatedDir = rotation * dirFromPivot;
    glm::vec3 newWorldPos = worldPoint + rotatedDir;
    setWorldPosition(newWorldPos);
    setWorldRotation(rotation * getWorldRotation());
}

void Transform::rotateAxis(const glm::vec3& axis, float angleDegrees) {
    glm::quat rotation = glm::angleAxis(glm::radians(angleDegrees), glm::normalize(axis));
    mLocalRotation = mLocalRotation * rotation;
    updateWorldTransformMatrix();
}

void Transform::lookAt(const glm::vec3& target, const glm::vec3& worldUp) {
    glm::vec3 position = getWorldPosition();
    glm::vec3 forward = position - target;
    float forwardLength = glm::length(forward);
    if (forwardLength < 0.0001f) {
        forward = glm::vec3(0.0f, 0.0f, -1.0f);
    }
    else {
        forward = forward / forwardLength;
    }
    glm::vec3 up = glm::normalize(worldUp);
    float dot = glm::dot(forward, up);
    if (std::abs(dot) > 0.9999f) {
        up = (std::abs(forward.y) > 0.9999f) ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
    }
    glm::vec3 right = glm::normalize(glm::cross(up, forward));
    up = glm::cross(forward, right);
    glm::mat3 rotationMatrix(right, up, forward);
    glm::quat worldRotation = glm::quat_cast(rotationMatrix);
    setWorldRotation(worldRotation);
}

void Transform::translate(const glm::vec3& offset) {
    setWorldPosition(getWorldPosition() + offset);
}

void Transform::translateLocal(const glm::vec3& localOffset) {
    glm::vec3 worldOffset = getWorldRotation() * localOffset;
    translate(worldOffset);
}

glm::vec3 Transform::transformDirection(const glm::vec3& direction) {
    return getWorldRotation() * direction;
}

glm::vec3 Transform::inverseTransformDirection(const glm::vec3& worldDirection) {
    return glm::inverse(getWorldRotation()) * worldDirection;
}

glm::vec3 Transform::transformPoint(const glm::vec3& point) {
    glm::vec4 result = getWorldTransformMatrix() * glm::vec4(point, 1.0f);
    return glm::vec3(result);
}

glm::vec3 Transform::inverseTransformPoint(const glm::vec3& worldPoint) {
    glm::vec4 result = glm::inverse(getWorldTransformMatrix()) * glm::vec4(worldPoint, 1.0f);
    return glm::vec3(result);
}

void Transform::setParent(Transform* newParent) {
    if (parent) {
        parent->removeChild(this);
    }
    glm::vec3 globalPos = getWorldPosition();
    glm::quat globalRot = getWorldRotation();
    parent = newParent;
    if (parent) {
        parent->addChild(this);
    }
    setWorldPosition(globalPos);
    setWorldRotation(globalRot);
}

Transform* Transform::getParent() const {
    return parent;
}

void Transform::addChild(Transform* child) {
    if (std::find(mChildren.begin(), mChildren.end(), child) == mChildren.end()) {
        mChildren.push_back(child);
    }
}

void Transform::removeChild(Transform* child) {
    auto it = std::find(mChildren.begin(), mChildren.end(), child);
    if (it != mChildren.end()) {
        (*it)->parent = nullptr; // Clear child's parent pointer
        mChildren.erase(it);
    }
}

int Transform::getChildCount() const {
    return static_cast<int>(mChildren.size());
}

Transform* Transform::getChild(int index) {
    if (index >= 0 && index < static_cast<int>(mChildren.size())) {
        return mChildren[index];
    }
    return nullptr;
}

glm::mat4 Transform::getLocalTransformMatrix() const {
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), mLocalPosition);
    glm::mat4 rotationMatrix = glm::toMat4(mLocalRotation);
    return translationMatrix * rotationMatrix;
}

glm::mat4 Transform::getWorldTransformMatrix() const {
    return mWorldTransformMatrix;
}

void Transform::updateWorldTransformMatrix() {
    if (parent) {
        mWorldTransformMatrix = parent->getWorldTransformMatrix() * getLocalTransformMatrix();
    }
    else {
        mWorldTransformMatrix = getLocalTransformMatrix();
    }
    for (auto& child : mChildren) {
        child->updateWorldTransformMatrix();
    }
}

glm::vec3 Transform::getForward() const {
    return getWorldRotation() * glm::vec3(0.0f, 0.0f, -1.0f);
}

glm::vec3 Transform::getRight() const {
    return getWorldRotation() * glm::vec3(1.0f, 0.0f, 0.0f);
}

glm::vec3 Transform::getUp() const {
    return getWorldRotation() * glm::vec3(0.0f, 1.0f, 0.0f);
}