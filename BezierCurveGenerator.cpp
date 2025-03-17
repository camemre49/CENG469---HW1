#include "BezierCurveGenerator.h"
#include <iostream>

float generateRandomFloat(float min, float max);
void BezierCurveGenerator::generateRandomCurveCPs() {
    // Seed the random number generator with the current time
    srand(static_cast<unsigned int>(time(nullptr)));

    // Generate random control points within a specific range
    for (int i = 0; i < currentCurveCPsMatrix.length(); ++i) {
        float x = generateRandomFloat(curveBounds.minX, curveBounds.maxX);
        float y = generateRandomFloat(curveBounds.minY, curveBounds.maxY);
        float z = generateRandomFloat(curveBounds.minZ, curveBounds.maxZ);
        currentCurveCPsMatrix[i] = glm::vec3(x, y, z);
    }
    // Restart T index
    currentTIndex = 0;

    // Print the generated control points
    std::cout << "************************************************************************************************************" << std::endl;
    std::cout << "Generated Control Points:\n";
    int pointNO = 1;
    for (int i = 0; i < currentCurveCPsMatrix.length(); ++i) {
        std::cout << "P" << pointNO << " = [" << currentCurveCPsMatrix[i].x << ", " << currentCurveCPsMatrix[i].y << ", " << currentCurveCPsMatrix[i].z << "]\n";
        pointNO++;
    }
    std::cout << "************************************************************************************************************\n" << std::endl;

    calculateBezierCurveVertices();
}

// Helper function to generate random float values within a given range
float generateRandomFloat(float min, float max) {
    return min + static_cast<float>(random()) / (RAND_MAX / (max - min));
}

void BezierCurveGenerator::calculateBezierCurveVertices() {
    float currentT = 0.0f;
    float stepSize = 1.0f / (sampleCount - 1);
    for (int i = 0; i < sampleCount; ++i) {
        // Compute the Bezier curve equation using Bernstein polynomials for cubic Bezier
        glm::vec3 P = glm::vec3(pow(1 - currentT, 3)) * currentCurveCPsMatrix[0] +
                      glm::vec3(3 * pow(1 - currentT, 2) * currentT) * currentCurveCPsMatrix[1] +
                      glm::vec3(3 * (1 - currentT) * pow(currentT, 2)) * currentCurveCPsMatrix[2] +
                      glm::vec3(pow(currentT, 3)) * currentCurveCPsMatrix[3];

        currentCurveCoordinates.emplace_back(P.x, P.y, P.z);
        currentT += stepSize;
    }
}


