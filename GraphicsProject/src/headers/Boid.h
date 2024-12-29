#ifndef BOID_H
#define BOID_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>
#include "Shader.h"
#include "Model.h"

class Boid {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    float maxSpeed = 20.0f;
    float maxForce = 1.0f;

    Boid(glm::vec3 pos) : position(pos) {

        //Normalised Random Inital Velocity Direction by Max Speed
        velocity = glm::normalize(glm::vec3(
            (float)rand() / RAND_MAX * 2.0f - 1.0f,
            (float)rand() / RAND_MAX * 2.0f - 1.0f,
            (float)rand() / RAND_MAX * 2.0f - 1.0f
        )) * maxSpeed;

        acceleration = glm::vec3(0.0f);

    }

    //Velocities Updated With Respect to Framerate (This Caused Issues so I Clamped it to 30fps/0.08 in Main)
    void update(float deltaTime) {

        velocity += acceleration * deltaTime;
        if (glm::length(velocity) > maxSpeed) {
            velocity = glm::normalize(velocity) * maxSpeed;
        }
        position += velocity * deltaTime;
        acceleration = glm::vec3(0.0f);

    }

    //Forces Applied to Boids When Near their Containment Boundary, 
    //or Separation, Cohesion, or Alignment Forces Near Other Boids
    void applyForce(glm::vec3 force) {

        acceleration += force;

    }

    glm::mat4 getModelMatrix() const {

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);

        glm::vec3 forward = glm::normalize(velocity);
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 up = glm::normalize(glm::cross(right, forward));

        //Rotation Needed to Face Direction of Movement
        glm::mat4 rotation = glm::mat4(
            glm::vec4(right, 0.0f),
            glm::vec4(up, 0.0f),
            glm::vec4(forward, 0.0f),
            glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
        );

        model *= rotation;
        model = glm::scale(model, glm::vec3(2.0f));
        return model;

    }
};

class BoidManager {

public:

    BoidManager(const std::string& modelPath, Shader& shader);
    ~BoidManager();

    void initialize(int numBoids, float spawnRadius);
    void update(float deltaTime);
    void render(Shader& shader);

private:

    std::vector<Boid> boids;
    std::vector<glm::mat4> modelMatrices;
    std::shared_ptr<Model> boidModel;
    GLuint instanceVBO;

    //Parameters
    float separationRadius = 10.0f;
    float alignmentRadius = 50.0f;
    float cohesionRadius = 100.0f;
    float separationWeight = 1.5f;
    float alignmentWeight = 1.0f;
    float cohesionWeight = 1.0f;
    float boundaryRadius = 200;

    glm::vec3 calculateSeparation(Boid& boid);
    glm::vec3 calculateAlignment(Boid& boid);
    glm::vec3 calculateCohesion(Boid& boid);
};

#endif