#include "BezierCurveGenerator.h"
#include <iostream>
#include <random>

inline float generateRandomFloat(float min, float max);
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
    calculateCurveBasis();

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
inline float generateRandomFloat(float min, float max) {
    return min + static_cast<float>(random()) / (RAND_MAX / (max - min));
}

// Function to generate a random float within a given range
float randomFloat(float min, float max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(gen);
}

void BezierCurveGenerator::calculateBezierCurveVertices() {
    currentCurveCoordinates.clear();
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

bool BezierCurveGenerator::increaseTIndex() {
    currentTIndex++;
    calculateCurveBasis();

    if (currentTIndex >= sampleCount) {
        currentTIndex = 0;
        generateNextCurveCPs();
        return true;
    }

    return false;
}

float calculateScaleMax(glm::vec3 P0, glm::vec3 lastTangent);
void BezierCurveGenerator::generateNextCurveCPs() {
    std::vector<glm::vec3> nextCurveCPsMatrix;

    // Ensure C1 continuity: The first control point of the new curve is the last point of the previous curve
    glm::vec3 P0 = currentCurveCPsMatrix[3];  // Last control point of the previous curve
    nextCurveCPsMatrix.push_back(P0);

    // C1 Continuity: P1 should maintain the direction of the last segment (P3 - P2)
    glm::vec3 lastTangent = currentCurveCPsMatrix[3] - currentCurveCPsMatrix[2];
    float scaleFactor = randomFloat(0.5f, calculateScaleMax(P0, lastTangent));
    glm::vec3 P1 = P0 + lastTangent * scaleFactor;
    nextCurveCPsMatrix.push_back(P1);

    // Generate P2 and P3 randomly
    for (int i = 0; i < 2; ++i) {
        float x = generateRandomFloat(curveBounds.minX, curveBounds.maxX);
        float y = generateRandomFloat(curveBounds.minY, curveBounds.maxY);
        float z = generateRandomFloat(curveBounds.minZ, curveBounds.maxZ);
        nextCurveCPsMatrix.emplace_back(glm::vec3(x, y, z));
    }

    // Update the control points and recalculate the curve
    for (int i = 0; i < nextCurveCPsMatrix.size(); ++i) {
        currentCurveCPsMatrix[i] = nextCurveCPsMatrix[i];
    }
    calculateCurveBasis();
    calculateBezierCurveVertices();
}

// Helper function to compute scaling factor based on the max value and range [-1.2, 1.2]
float calculateScaleMax(glm::vec3 P0, glm::vec3 lastTangent) {
    float maxVal = std::max(P0.x + lastTangent.x, std::max(P0.y + lastTangent.y, P0.z + lastTangent.z));

    if (maxVal > 1.2f || maxVal < -1.2f) {
        return (maxVal > 0) ? 1.2f / maxVal : -1.2f / maxVal;
    } else {
        return (maxVal > 0) ? maxVal / 1.2f : maxVal / -1.2f;
    }
}

void BezierCurveGenerator::calculateCurveBasis() {
    if (currentTIndex == 0)
        currentCurveTangent = normalize(glm::vec3((currentCurveCPsMatrix[1] - currentCurveCPsMatrix[0])));
    else
        currentCurveTangent = normalize(glm::vec3((currentCurveCoordinates[currentTIndex] - currentCurveCoordinates[currentTIndex - 1])));


    // Update the full basis
    glm::vec3 newGaze = currentCurveTangent;
    glm::vec3 newLeft = normalize(glm::cross(currentUpVector, newGaze));
    glm::vec3 newUp = normalize(glm::cross(newGaze, newLeft));
    currentUpVector = newUp;
    currentLeftVector = newLeft;
}



