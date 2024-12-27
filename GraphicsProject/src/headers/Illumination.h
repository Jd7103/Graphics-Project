#ifndef ILLUMINATION_H
#define ILLUMINATION_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "Camera.h"
#include "Shader.h"

class Illumination {
public:

    GLuint depthMapArrayFBO, depthMapArray;
    static const int CASCADE_COUNT = 3;
    glm::vec3 lightDir;

    Illumination(unsigned int depthMapResolution, glm::vec3 lightDir);
    ~Illumination();

    void beginDepthPass();
    void endDepthPass();
    void updateCascades(Camera& camera);
    void bindDepthMaps(Shader& shader);

private:

    struct CascadeData {
        float splitDepth;
        glm::mat4 lightSpaceMatrix;
    };

    std::vector<CascadeData> cascades;
    std::vector<glm::vec3> frustumCorners(const glm::mat4& proj, const glm::mat4& view,  float near, float far);

    unsigned int depthMapResolution;

    void initDepthMaps();
};
#endif