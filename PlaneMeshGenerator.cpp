#include "PlaneMeshGenerator.h"

#include <fstream>
#include <iomanip>
#include <iostream>

std::vector<std::vector<glm::vec3>> PlaneMeshGenerator::calculateBezierSurfaceVertices(std::vector<std::vector<glm::vec3>> & surfaceBezierPoints, int resolution) {
    std::vector<std::vector<glm::vec3>> curveVertices = {};
    for (int i = 0; i < 4; i++) {
        curveVertices.push_back(BezierCurveGenerator::calculateBezierCurveVerticesByPoints(surfaceBezierPoints[i], resolution));
    }
    std::vector<std::vector<glm::vec3>> surfacePoints = {};
    for (int i = 0; i < resolution; i++) {
        std::vector<glm::vec3> curvePoints = {curveVertices[0][i], curveVertices[1][i], curveVertices[2][i], curveVertices[3][i]};
        surfacePoints.push_back(BezierCurveGenerator::calculateBezierCurveVerticesByPoints(curvePoints, resolution));
    }

    return surfacePoints;
}

void PlaneMeshGenerator::printSurfacePoints(std::vector<std::vector<glm::vec3>> &surfaceBezierVertices) {
    for (auto & surfaceBezierVertice : surfaceBezierVertices) {
        BezierCurveGenerator::printCurveVertices(surfaceBezierVertice);
    }
}

void PlaneMeshGenerator::calculateAllBezierSurfaceVertices(int resolution) {
    surfaces.clear();
    for (auto& bezierPointGroup : bezierPoints) {
    surfaces.push_back(generateBezierSurface(bezierPointGroup, resolution));
    }
    exportAllSurfacesToOBJ(surfaces, filename);
}

BezierSurface PlaneMeshGenerator::generateBezierSurface(std::vector<std::vector<glm::vec3>>& controlPoints, int resolution) {
    BezierSurface surface;

    auto surfaceGrid = calculateBezierSurfaceVertices(controlPoints, resolution);
    for (const auto& row : surfaceGrid) {
        surface.vertices.insert(surface.vertices.end(), row.begin(), row.end());
    }

    // surface.vertices => curve1.vertices, curve2.vertices, curve3.vertices, ...
    // Create vertices by using simple triangle way
    for (int row = 0; row < resolution - 1; row++) {
        for (int col = 0; col < resolution - 1; col++) {
            // Current quad indices
            int topLeft = row * resolution + col;
            int topRight = row * resolution + col + 1;
            int bottomLeft = (row + 1) * resolution + col;
            int bottomRight = (row + 1) * resolution + col + 1;

            // Triangle 1 (top-left to bottom-right)
            surface.indices.push_back(topLeft);
            surface.indices.push_back(topRight);
            surface.indices.push_back(bottomRight);

            // Triangle 2
            surface.indices.push_back(topLeft);
            surface.indices.push_back(bottomRight);
            surface.indices.push_back(bottomLeft);
        }
    }

    /** Print function for indices
    for (size_t i = 0; i < surface.indices.size(); i += 3) {
        std::cout << "Triangle " << i/3 + 1 << ": "
                  << surface.indices[i] << ", "
                  << surface.indices[i+1] << ", "
                  << surface.indices[i+2] << std::endl;
    }
    */

    surface.normals.resize(surface.vertices.size(), glm::vec3(0.0f));
    std::vector<int> normalCounts(surface.vertices.size(), 0); // Track how many faces contribute

    // First pass: Accumulate face normals and count contributions
    for (size_t i = 0; i < surface.indices.size(); i += 3) {
        unsigned int i0 = surface.indices[i];
        unsigned int i1 = surface.indices[i+1];
        unsigned int i2 = surface.indices[i+2];

        glm::vec3 v0 = surface.vertices[i0];
        glm::vec3 v1 = surface.vertices[i1];
        glm::vec3 v2 = surface.vertices[i2];

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 faceNormal = glm::cross(edge1, edge2);

        // Accumulate normals and increment counts
        surface.normals[i0] += faceNormal;
        surface.normals[i1] += faceNormal;
        surface.normals[i2] += faceNormal;

        normalCounts[i0]++;
        normalCounts[i1]++;
        normalCounts[i2]++;
    }

    // Second pass: Average and normalize
    for (size_t i = 0; i < surface.normals.size(); i++) {
        if (normalCounts[i] > 0) {
            // True average: divide by count before normalizing
            surface.normals[i] = glm::normalize(surface.normals[i] / static_cast<float>(normalCounts[i]));
        } else {
            surface.normals[i] = glm::vec3(0.0f, 1.0f, 0.0f); // Fallback
        }
    }
    return surface;
}

void PlaneMeshGenerator::exportAllSurfacesToOBJ(const std::vector<BezierSurface>& surfaces,
                                              const std::string& filename) {
    std::ofstream objFile(filename);
    if (!objFile.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    }

    objFile << std::fixed << std::setprecision(6);
    objFile << "# Combined export of " << surfaces.size() << " Bezier surfaces\n\n";

    // 1. Write ALL vertices first
    objFile << "# Vertices (" << surfaces.size() << " surfaces)\n";
    for (const auto& surface : surfaces) {
        for (const auto& vertex : surface.vertices) {
            objFile << "v " << vertex.x << " " << vertex.y << " " << vertex.z << "\n";
        }
    }

    // 2. Write ALL vertex normals
    objFile << "\n# Vertex normals\n";
    for (const auto& surface : surfaces) {
        for (const auto& normal : surface.normals) {
            objFile << "vn " << normal.x << " " << normal.y << " " << normal.z << "\n";
        }
    }

    // 3. Write ALL faces with proper offsets
    objFile << "\n# Faces\n";
    unsigned int vertexOffset = 0;
    unsigned int normalOffset = 0;

    for (const auto& surface : surfaces) {
        for (size_t i = 0; i < surface.indices.size(); i += 3) {
            unsigned int idx0 = surface.indices[i] + 1 + vertexOffset;
            unsigned int idx1 = surface.indices[i+1] + 1 + vertexOffset;
            unsigned int idx2 = surface.indices[i+2] + 1 + vertexOffset;

            objFile << "f "
                    << idx0 << "//" << idx0 << " "
                    << idx1 << "//" << idx1 << " "
                    << idx2 << "//" << idx2 << "\n";
        }

        vertexOffset += surface.vertices.size();
        normalOffset += surface.normals.size();
    }

    objFile.close();
    std::cout << "Successfully exported " << surfaces.size()
              << " surfaces to " << filename << std::endl;
}

