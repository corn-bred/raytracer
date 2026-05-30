#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Ray {
  public:
    Ray() {}

    Ray(const glm::vec3& origin, const glm::vec3& direction) : orig(origin), dir(direction) {}

    const glm::vec3 &Origin() const  { return orig; }
    const glm::vec3 &Direction() const { return dir; }

    glm::vec3 At(double t) const {
        return orig + dir * glm::vec3(t);
    }

  private:
    glm::vec3 orig = glm::vec3(0.0);
    glm::vec3 dir = glm::vec3(0.0);
};