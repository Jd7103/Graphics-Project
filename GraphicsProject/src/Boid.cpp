// Boid.cpp
#include "Boid.h"

BoidManager::BoidManager(const std::string& modelPath, Shader& shader) {
    // Load the model
    boidModel = std::make_shared<Model>(modelPath);

    // Setup instance buffer
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

    // Setup the instance buffer attributes for the model's meshes
    for (auto& mesh : boidModel->meshes) {
        glBindVertexArray(mesh.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

        // Instance transform matrix (4 vec4s)
        for (int i = 0; i < 4; i++) {
            glEnableVertexAttribArray(7 + i);
            glVertexAttribPointer(7 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
                (void*)(sizeof(glm::vec4) * i));
            glVertexAttribDivisor(7 + i, 1);
        }
    }
}

BoidManager::~BoidManager() {
    if (instanceVBO != 0) {
        glDeleteBuffers(1, &instanceVBO);
        instanceVBO = 0;
    }
}

void BoidManager::initialize(int numBoids, float spawnRadius) {
    boids.clear();
    modelMatrices.reserve(numBoids);

    for (int i = 0; i < numBoids; i++) {
        float theta = rand() / (float)RAND_MAX * 2.0f * 3.14159f;
        float phi = rand() / (float)RAND_MAX * 3.14159f;
        float r = rand() / (float)RAND_MAX * spawnRadius;

        glm::vec3 position(
            r * sin(phi) * cos(theta),
            r * sin(phi) * sin(theta) + 200.0f,
            r * cos(phi)
        );

        boids.emplace_back(position);
    }

    boundaryRadius = spawnRadius;

    // Resize the instance buffer
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, boids.size() * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
}

void BoidManager::update(float deltaTime) {
    modelMatrices.clear();
    modelMatrices.reserve(boids.size());

    for (auto& boid : boids) {
        // Calculate flocking forces
        glm::vec3 separation = calculateSeparation(boid) * separationWeight;
        glm::vec3 alignment = calculateAlignment(boid) * alignmentWeight;
        glm::vec3 cohesion = calculateCohesion(boid) * cohesionWeight;

        // Apply flocking forces
        boid.applyForce(separation);
        boid.applyForce(alignment);
        boid.applyForce(cohesion);

        // Only apply boundary avoidance when close to the boundary
        const float BOUNDARY_MARGIN = 50.0f;  // Start avoiding when 50 units from boundary
        const float BOUNDARY_FORCE = 2.5f;    // Increased force for stronger avoidance

        glm::vec2 xzPos = glm::vec2(boid.position.x, boid.position.z);
        float distanceFromCenter = glm::length(xzPos);

        // Only apply force if we're close to the boundary
        if (distanceFromCenter > (boundaryRadius - BOUNDARY_MARGIN) && distanceFromCenter > 0.01f) {
            // Calculate normalized direction to center
            glm::vec2 towardCenter = -xzPos / distanceFromCenter;

            // Force increases exponentially as we get closer to boundary
            float distanceFromBoundary = boundaryRadius - distanceFromCenter;
            float forceMagnitude = BOUNDARY_FORCE * (1.0f - (distanceFromBoundary / BOUNDARY_MARGIN));
            forceMagnitude = forceMagnitude * forceMagnitude; // Square it for exponential increase

            glm::vec3 avoidanceForce(
                towardCenter.x * forceMagnitude,
                0.0f,
                towardCenter.y * forceMagnitude
            );
            boid.applyForce(avoidanceForce);
        }

        // Height constraints with avoidance forces
        const float MIN_HEIGHT = 200.0f;
        const float MAX_HEIGHT = 800.0f;
        const float HEIGHT_MARGIN = 20.0f;
        const float HEIGHT_FORCE = 1.5f;

        if (boid.position.y < (MIN_HEIGHT + HEIGHT_MARGIN)) {
            float forceMagnitude = HEIGHT_FORCE *
                (1.0f - (boid.position.y - MIN_HEIGHT) / HEIGHT_MARGIN);
            boid.applyForce(glm::vec3(0.0f, forceMagnitude, 0.0f));
        }

        if (boid.position.y > (MAX_HEIGHT - HEIGHT_MARGIN)) {
            float forceMagnitude = HEIGHT_FORCE *
                ((boid.position.y - (MAX_HEIGHT - HEIGHT_MARGIN)) / HEIGHT_MARGIN);
            boid.applyForce(glm::vec3(0.0f, -forceMagnitude, 0.0f));
        }

        // Update position and velocity
        boid.update(deltaTime);

        // Store transform matrix for rendering
        modelMatrices.push_back(boid.getModelMatrix());
    }

    // Update instance buffer
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, modelMatrices.size() * sizeof(glm::mat4), modelMatrices.data());
}

void BoidManager::render(Shader& shader) {
    if (modelMatrices.empty()) return;

    shader.use();
    boidModel->render(shader, true, modelMatrices.size());
}

glm::vec3 BoidManager::calculateSeparation(Boid& boid) {
    glm::vec3 steering = glm::vec3(0.0f);
    int count = 0;

    for (auto& other : boids) {
        float distance = glm::length(boid.position - other.position);
        if (&other != &boid && distance < separationRadius) {
            glm::vec3 diff = boid.position - other.position;
            diff = glm::normalize(diff);
            diff /= distance;
            steering += diff;
            count++;
        }
    }

    if (count > 0) {
        steering /= (float)count;
        steering = glm::normalize(steering) * boid.maxSpeed;
        steering -= boid.velocity;
        if (glm::length(steering) > boid.maxForce) {
            steering = glm::normalize(steering) * boid.maxForce;
        }
    }
    return steering;
}

glm::vec3 BoidManager::calculateAlignment(Boid& boid) {
    glm::vec3 steering = glm::vec3(0.0f);
    int count = 0;

    for (auto& other : boids) {
        float distance = glm::length(boid.position - other.position);
        if (&other != &boid && distance < alignmentRadius) {
            steering += other.velocity;
            count++;
        }
    }

    if (count > 0) {
        steering /= (float)count;
        steering = glm::normalize(steering) * boid.maxSpeed;
        steering -= boid.velocity;
        if (glm::length(steering) > boid.maxForce) {
            steering = glm::normalize(steering) * boid.maxForce;
        }
    }
    return steering;
}

glm::vec3 BoidManager::calculateCohesion(Boid& boid) {
    glm::vec3 steering = glm::vec3(0.0f);
    glm::vec3 centerOfMass = glm::vec3(0.0f);
    float totalWeight = 0.0f;

    for (auto& other : boids) {
        float distance = glm::length(boid.position - other.position);
        if (&other != &boid && distance < cohesionRadius) {
            // Apply inverse square falloff to make distant boids less attractive
            float weight = 1.0f / (distance * distance + 1.0f);
            centerOfMass += other.position * weight;
            totalWeight += weight;
        }
    }

    if (totalWeight > 0.0f) {
        centerOfMass /= totalWeight;
        glm::vec3 desired = centerOfMass - boid.position;
        float distance = glm::length(desired);

        // Only steer if not too close
        if (distance > 0.0f) {
            desired = glm::normalize(desired) * boid.maxSpeed;
            steering = desired - boid.velocity;
            if (glm::length(steering) > boid.maxForce) {
                steering = glm::normalize(steering) * boid.maxForce;
            }
        }
    }
    return steering;
}