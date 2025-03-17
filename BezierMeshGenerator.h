#ifndef BEZIERMESHGENERATOR_H
#define BEZIERMESHGENERATOR_H

#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

class BezierMeshGenerator {

public:
    glm::vec3 initialUpVector = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 initialLeftVector = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 initialGazeVector = glm::vec3(-1.0f, 0.0f, 0.0f);
};
#endif //BEZIERMESHGENERATOR_H
