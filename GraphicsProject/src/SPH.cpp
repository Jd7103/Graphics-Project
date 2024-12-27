#include "SPH.h"
#include <glm/gtc/matrix_transform.hpp>

SPH::SPH(const std::string& vertexPath, const std::string& fragmentPath, float size)
    : containerSize(size), shader(vertexPath.c_str(), fragmentPath.c_str()) {

    float jitter = h * 0.2f;  // Increased initial spread
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-jitter, jitter);

    float spacing = h * 0.9f;
    int particlesPerSide = 6;

    for (int x = 0; x < particlesPerSide; x++) {
        for (int y = 0; y < particlesPerSide; y++) {
            for (int z = 0; z < particlesPerSide; z++) {
                glm::vec3 pos(
                    x * spacing + dist(gen),
                    y * spacing + containerSize * 0.5f + dist(gen),
                    z * spacing + dist(gen)
                );
                pos -= glm::vec3(
                    particlesPerSide * spacing * 0.5f,
                    0.0f,
                    particlesPerSide * spacing * 0.5f
                );
                particles.emplace_back(pos);
            }
        }
    }

    generateSphereGeometry(1.0f, 8);
    setupInstanceBuffer();
}

void SPH::generateSphereGeometry(float radius, int segments) {
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;

    // Generate sphere vertices
    for (int lat = 0; lat <= segments; lat++) {
        float theta = lat * glm::pi<float>() / segments;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int lon = 0; lon <= segments; lon++) {
            float phi = lon * 2 * glm::pi<float>() / segments;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            glm::vec3 vertex;
            vertex.x = radius * cosPhi * sinTheta;
            vertex.y = radius * cosTheta;
            vertex.z = radius * sinPhi * sinTheta;
            vertices.push_back(vertex);
        }
    }

    // Generate indices
    for (int lat = 0; lat < segments; lat++) {
        for (int lon = 0; lon < segments; lon++) {
            unsigned int first = lat * (segments + 1) + lon;
            unsigned int second = first + segments + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    sphereVertices = vertices.size();
    sphereIndices = indices.size();

    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);

    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
}

void SPH::setupInstanceBuffer() {
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, particles.size() * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);

    // Set up instance matrix attributes
    for (unsigned int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(1 + i);
        glVertexAttribPointer(1 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
            (void*)(sizeof(glm::vec4) * i));
        glVertexAttribDivisor(1 + i, 1);
    }
}

void SPH::render(const glm::mat4& view, const glm::mat4& projection) {
    std::vector<glm::mat4> modelMatrices;
    modelMatrices.reserve(particles.size());

    for (const auto& particle : particles) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), particle.position);
        model = glm::scale(model, glm::vec3(particleRadius));
        modelMatrices.push_back(model);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, particles.size() * sizeof(glm::mat4), modelMatrices.data());

    glBindVertexArray(sphereVAO);
    glDrawElementsInstanced(GL_TRIANGLES, sphereIndices, GL_UNSIGNED_INT, 0, particles.size());

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

float SPH::W(const glm::vec3& r, float h) {
    float len = glm::length(r);
    if (len >= h) return 0.0f;

    float q = len / h;
    float q2 = q * q;
    float q3 = q2 * q;

    float coeff = 15.0f / (2.0f * glm::pi<float>() * h * h * h);
    if (q < 0.5f) {
        return coeff * ((2.0f / 3.0f) - q2 + 0.5f * q3);
    }
    else {
        float tmp = 2.0f - q;
        return coeff * (1.0f / 6.0f) * tmp * tmp * tmp;
    }
}

glm::vec3 SPH::gradW(const glm::vec3& r, float h) {
    float len = glm::length(r);
    if (len >= h || len < 1e-5f) return glm::vec3(0.0f);

    float q = len / h;
    float coeff = 15.0f / (2.0f * glm::pi<float>() * h * h * h * h);
    float gradient;

    if (q < 0.5f) {
        gradient = coeff * (-2.0f + 1.5f * q);
    }
    else {
        float tmp = 2.0f - q;
        gradient = -coeff * 0.5f * tmp * tmp;
    }

    return gradient * r / len;
}

void SPH::computeDensityPressure() {
    // First pass - calculate base densities
    for (auto& pi : particles) {
        pi.density = 0.0f;
        for (const auto& pj : particles) {
            glm::vec3 r = pi.position - pj.position;
            pi.density += mass * W(r, h);
        }
        pi.density = glm::max(pi.density, rho0 * 0.1f);
    }

    // Second pass - normalized contribution
    for (auto& pi : particles) {
        float normalizedDensity = 0.0f;
        for (const auto& pj : particles) {
            glm::vec3 r = pi.position - pj.position;
            // Divide contribution by particle's own density
            normalizedDensity += mass * W(r, h) / pj.density;
        }
        pi.density = normalizedDensity * rho0;
        pi.pressure = k * (pi.density - rho0);
    }
}

glm::vec3 SPH::computePressureForce(const SPHParticle& pi, const SPHParticle& pj) {
    glm::vec3 r = pj.position - pi.position;
    float len = glm::length(r);
    if (len < 1e-5f || len >= h) return glm::vec3(0.0f);

    float minDensity = rho0 * 0.1f;
    float pi_density = glm::max(pi.density, minDensity);
    float pj_density = glm::max(pj.density, minDensity);

    // Anti-clustering force at very short range
    float repulsion = 0.0f;
    if (len < h * 0.1f) {
        repulsion = 5000.0f * (1.0f - len / (h * 0.1f));
    }

    float pressureForce = mass * (pi.pressure + pj.pressure) / (2.0f * pj_density); // +repulsion;
    return pressureForce * gradW(r, h);
}


glm::vec3 SPH::computeViscosityForce(const SPHParticle& pi, const SPHParticle& pj) {
    glm::vec3 r = pi.position - pj.position;
    float len = glm::length(r);
    if (len >= h) return glm::vec3(0.0f);

    glm::vec3 velDiff = pj.velocity - pi.velocity;
    // Reduce viscosity effect at close range
    float viscosityScale = glm::smoothstep(0.0f, h, len);
    return mass * viscosityScale * mu * velDiff * W(r, h) / pj.density;
}

void SPH::computeForces() {
#pragma omp parallel for
    for (size_t i = 0; i < particles.size(); i++) {
        glm::vec3 totalForce(0.0f);
        for (size_t j = 0; j < particles.size(); j++) {
            if (i != j) {
                totalForce += computePressureForce(particles[i], particles[j]);
                totalForce += computeViscosityForce(particles[i], particles[j]);
            }
        }
        particles[i].force = totalForce + glm::vec3(0.0f, g, 0.0f) * particles[i].density;
    }
}

void SPH::handleBoundaryCollisions(SPHParticle& particle) {
    float damping = 0.8f;    
    float eps = h * 0.01f;
    float boundaryForce = 1000.0f;  // Added boundary force

    auto handleBound = [damping, boundaryForce](float& pos, float& vel, float bound, float eps) {
        if (std::abs(pos) > bound) {
            // Boundary force
            float penetration = std::abs(pos) - bound;
            float force = boundaryForce * penetration;
            vel += -glm::sign(pos) * force * 0.016f;  // Using a fixed timestep for stability

            // Position correction
            pos = glm::sign(pos) * bound;
            vel *= -damping;
        }
    };

    handleBound(particle.position.x, particle.velocity.x, containerSize, eps);
    handleBound(particle.position.y, particle.velocity.y, containerSize, eps);
    handleBound(particle.position.z, particle.velocity.z, containerSize, eps);
}

void SPH::update(float deltaTime) {
    const float maxSpeed = 200.0f;
    deltaTime = glm::min(deltaTime, 0.016f);

    computeDensityPressure();
    computeForces();

#pragma omp parallel for
    for (size_t i = 0; i < particles.size(); i++) {
        // Add velocity damping
        particles[i].velocity *= 0.99f;

        glm::vec3 acceleration = particles[i].force / particles[i].density;

        // Clamp acceleration
        const float maxAccel = 300.0f;
        if (glm::length(acceleration) > maxAccel) {
            acceleration = glm::normalize(acceleration) * maxAccel;
        }

        particles[i].velocity += acceleration * deltaTime;

        // Clamp velocity
        float speed = glm::length(particles[i].velocity);
        if (speed > maxSpeed) {
            particles[i].velocity *= maxSpeed / speed;
        }

        particles[i].position += particles[i].velocity * deltaTime;
        handleBoundaryCollisions(particles[i]);
    }
}

SPH::~SPH() {
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);
    glDeleteBuffers(1, &instanceVBO);
}
