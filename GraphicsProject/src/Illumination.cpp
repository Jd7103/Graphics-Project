#include "Illumination.h"

Illumination::Illumination(unsigned int depthMapResolution, glm::vec3 lightDir)
    : depthMapResolution(depthMapResolution), lightDir(glm::normalize(glm::vec3(-0.5f, -0.75f, -0.25f))) {

    initDepthMaps();

}

Illumination::~Illumination() {

    glDeleteFramebuffers(1, &depthMapArrayFBO);
    glDeleteTextures(1, &depthMapArray);

}

void Illumination::initDepthMaps() {

    glGenFramebuffers(1, &depthMapArrayFBO);

    glGenTextures(1, &depthMapArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthMapArray);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F,
        depthMapResolution, depthMapResolution, CASCADE_COUNT,
        0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapArrayFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthMapArray, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Illumination::updateCascades(Camera& camera) {
    std::cout << "Before resize: " << cascades.size() << std::endl;
    cascades.resize(CASCADE_COUNT);
    std::cout << "After resize: " << cascades.size() << std::endl;

    float cascadeSplits[CASCADE_COUNT + 1] = {
        camera.zNear,
        camera.zNear * 50,
        camera.zNear * 200,
        camera.zFar
    };

    for (int i = 0; i < CASCADE_COUNT; i++) {
        try {
            std::cout << "Processing cascade " << i << std::endl;
            float nearSplit = cascadeSplits[i];
            float farSplit = cascadeSplits[i + 1];

            std::vector<glm::vec3> corners = frustumCorners(
                camera.projectionMatrix(),
                camera.viewMatrix(),
                nearSplit,
                farSplit
            );

            glm::vec3 center(0.0f);
            for (const auto& v : corners) {
                center += v;
            }
            center /= static_cast<float>(corners.size());

            float radius = 0.0f;
            for (const auto& corner : corners) {
                float distance = glm::length(corner - center);
                radius = glm::max(radius, distance);
            }
            radius = std::ceil(radius * 16.0f) / 16.0f;

            glm::vec3 lightPos = center - lightDir * radius;
            glm::mat4 lightView = glm::lookAt(
                lightPos,
                center,
                glm::vec3(0.0f, 1.0f, 0.0f)
            );
            glm::mat4 lightProj = glm::ortho(
                -radius, radius,
                -radius, radius,
                0.0f, 2.0f * radius
            );

            std::cout << "Accessing index " << i << std::endl;
            cascades.at(i).splitDepth = farSplit;
            cascades.at(i).lightSpaceMatrix = lightProj * lightView;
            std::cout << "Successfully set cascade " << i << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << "Exception at index " << i << ": " << e.what() << std::endl;
        }
    }
}

void Illumination::beginDepthPass() {
    glViewport(0, 0, depthMapResolution, depthMapResolution);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapArrayFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void Illumination::endDepthPass() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Illumination::bindDepthMaps(Shader& shader) {
    shader.setVec3("lightDir", lightDir);
    shader.setInt("depthMap", 1);  // Verify texture unit binding
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthMapArray);

    // Set lightSpaceMatrix for both regular and depth passes
    for (int i = 0; i < CASCADE_COUNT; i++) {
        std::string depthIndex = std::to_string(i);
        shader.setFloat("cascadePlaneDistances[" + depthIndex + "]", cascades[i].splitDepth);
        shader.setMat4("cascadeSpace[" + depthIndex + "]", cascades[i].lightSpaceMatrix);
        // For depth pass
        if (i == 0) { // Only need first cascade for depth pass
            shader.setMat4("lightSpaceMatrix", cascades[i].lightSpaceMatrix);
        }
    }
}

std::vector<glm::vec3> Illumination::frustumCorners(const glm::mat4& proj, const glm::mat4& view, float near, float far) {
    
    std::vector<glm::vec3> corners;
    const glm::mat4 inv = glm::inverse(proj * view);

    for (unsigned int x = 0; x < 2; ++x) {
        for (unsigned int y = 0; y < 2; ++y) {
            for (unsigned int z = 0; z < 2; ++z) {
                const glm::vec4 pt = inv * glm::vec4(
                    2.0f * x - 1.0f,
                    2.0f * y - 1.0f,
                    2.0f * z - 1.0f,
                    1.0f);
                corners.push_back(glm::vec3(pt / pt.w));
            }
        }
    }
    return corners;

}