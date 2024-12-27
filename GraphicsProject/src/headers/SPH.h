#ifndef SPH_H
#define SPH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <random>
#include <stdexcept>
#include <algorithm>

#include <glm/gtc/constants.hpp>
#include "Shader.h"

struct SPHParticle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 force;
    float density;
    float pressure;

    SPHParticle(const glm::vec3& pos) :
        position(pos), velocity(0.0f), force(0.0f),
        density(0.0f), pressure(0.0f) {}
};

class SPH {
public:
    SPH(const std::string& vertexPath, const std::string& fragmentPath, float containerSize = 10.0f);
    ~SPH();

    void update(float deltaTime);
    void render(const glm::mat4& view, const glm::mat4& projection);

private:
    std::vector<SPHParticle> particles;

    float h = 0.4f;            // Smoothing length
    float mass = 0.001f;         // Particle mass
    float rho0 = 1000.0f;      // Rest density of water
    float k = 8000.0f;         // Pressure constant
    float mu = 0.01f;          // Viscosity coefficient
    float g = -9.81f;          // Gravity
    float particleRadius = 0.4f;
    float containerSize = 50.0f;



    // GPU resources
    Shader shader;
    GLuint sphereVAO, sphereVBO, sphereEBO;
    GLuint instanceVBO;
    unsigned int sphereVertices;
    unsigned int sphereIndices;

    void generateSphereGeometry(float radius, int segments);
    void setupInstanceBuffer();
    void computeDensityPressure();
    void computeForces();
    glm::vec3 computePressureForce(const SPHParticle& pi, const SPHParticle& pj);
    glm::vec3 computeViscosityForce(const SPHParticle& pi, const SPHParticle& pj);
    void handleBoundaryCollisions(SPHParticle& particle);
    float W(const glm::vec3& r, float h);
    glm::vec3 gradW(const glm::vec3& r, float h);
};

#endif