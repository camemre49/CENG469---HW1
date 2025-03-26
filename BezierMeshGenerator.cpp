#include "BezierMeshGenerator.h"

#include <iostream>

void BezierMeshGenerator::generateObjectToFile() {
    for (int i = 0; i < 4; i++) {
        std::vector<glm::vec3> generatedCurve = BezierCurveGenerator::calculateBezierCurveVerticesByPoints(bezierPoints[0][i], 10);
        BezierCurveGenerator::printCurveVertices(generatedCurve);
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
    }
}
