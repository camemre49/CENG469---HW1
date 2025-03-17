#ifndef BEZIERCURVEGENERATOR_H
#define BEZIERCURVEGENERATOR_H

#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>


class BezierCurveGenerator {
private:
    // A float to denote invalid position to determine if control point is defined or not
    const float INVALID_POSITION = -100.0f;

    // Bounds of the curve control points
    struct CurveBounds {
        float minX, maxX;
        float minY, maxY;
        float minZ, maxZ;
    };
    const CurveBounds curveBounds;

    // Bezier matrix
    const glm::mat4x4 bezierMatrix = glm::mat4x4(
        -1, 3, -3, 1,
        3, -6, 3, 0,
        -3, 3, 0, 0,
        1, 0, 0, 0);

    /**
     * Generates the next curve by using the previous curve
     */
    void generateNextCurveCPs();

// Constructors
public:
    BezierCurveGenerator(float minX, float maxX, float minY, float maxY, float minZ, float maxZ)
        : curveBounds{minX, maxX, minY, maxY, minZ, maxZ} {}

    BezierCurveGenerator(float commonMin, float commonMax)
        : curveBounds{commonMin, commonMax, commonMin, commonMax, commonMin, commonMax} {}

public:
    static constexpr int sampleCount = 100;

    glm::mat4x3 currentCurveCPsMatrix = {
        INVALID_POSITION, INVALID_POSITION, INVALID_POSITION,
        INVALID_POSITION,INVALID_POSITION, INVALID_POSITION,
        INVALID_POSITION, INVALID_POSITION,INVALID_POSITION,
        INVALID_POSITION, INVALID_POSITION, INVALID_POSITION};
    std::vector<glm::vec3> currentCurveCoordinates;
    int currentTIndex = 0;
    glm::vec3 currentCurveTangent;
    glm::vec3 currentUpVector = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 currentLeftVector = glm::vec3(INVALID_POSITION, INVALID_POSITION, INVALID_POSITION);

    /**
     * generate random Bezier curve points without any initial and set the private CP member
     */
    void generateRandomCurveCPs();

    /**
     * Generate 100 vertices for curve generation
     */
    void calculateBezierCurveVertices();

    /**
     * Increase the T index by preserving the range
     *
     * @return true if buffering the new curve data is needed.
     */
    bool increaseTIndex();

    /**
     * Calculates the basis matrix for the currently defined Bezier curve control points.
     */
    void calculateCurveBasis();
};

#endif //BEZIERCURVEGENERATOR_H
