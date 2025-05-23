#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

class Transform {
public:
    Transform(glm::vec3 localPosition, glm::vec3 localEulerRotation, Transform* parent = nullptr);
    ~Transform();

    void setLocalPosition(glm::vec3 localPosition);
    glm::vec3 getLocalPosition() const;

    void setWorldPosition(glm::vec3 globalPosition);
    glm::vec3 getWorldPosition() const;

    void setLocalRotation(glm::quat localRotation);
    glm::quat getLocalRotation() const;

    void setWorldRotation(glm::quat globalRotation);
    glm::quat getWorldRotation() const;

    void setLocalRotationEuler(glm::vec3 localRotationEuler);
    void setWorldRotationEuler(glm::vec3 localRotationEuler);

    void rotateAround(const glm::vec3& worldPoint, const glm::vec3& axis, float angleDegrees);
    void rotateAxis(const glm::vec3& axis, float angleDegrees);
    void lookAt(const glm::vec3& target, const glm::vec3& worldUp = glm::vec3(0.0f, 1.0f, 0.0f));

    void translate(const glm::vec3& offset);
    void translateLocal(const glm::vec3& localOffset);
    glm::vec3 transformDirection(const glm::vec3& direction);
    glm::vec3 inverseTransformDirection(const glm::vec3& worldDirection);
    glm::vec3 transformPoint(const glm::vec3& point);
    glm::vec3 inverseTransformPoint(const glm::vec3& worldPoint);

    void setParent(Transform* newParent);
    Transform* getParent() const;

    void addChild(Transform* child);
    void removeChild(Transform* child);
    int getChildCount() const;
    Transform* getChild(int index);

    glm::mat4 getLocalTransformMatrix() const;
    glm::mat4 getWorldTransformMatrix() const;

    virtual void updateWorldTransformMatrix();

    glm::vec3 getForward() const;
    glm::vec3 getRight() const;
    glm::vec3 getUp() const;

private:
    glm::vec3 mLocalPosition;
    glm::quat mLocalRotation;
    Transform* parent;
    std::vector<Transform*> mChildren;
    glm::mat4 mWorldTransformMatrix;
};