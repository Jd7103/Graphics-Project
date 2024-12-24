#include "Main.h"

//Camera Params
const unsigned int SCREEN_WIDTH = 1920;
const unsigned int SCREEN_HEIGHT = 1080;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), 0.5f, 2500.0f, SCREEN_HEIGHT, SCREEN_WIDTH);
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true;

//FPS Tracker Vars
float deltaTime = 0.0f;
float lastFrame = 0.0f;
int frames = 0;
int fTime = 0;

//Lighting Params
glm::vec3 lightDir = glm::normalize(glm::vec3(-0.5f, -0.75f, -0.25f));

//Shadow Mapping Params
GLuint depthFBO, depthMap;
unsigned int depthMapResolution = 2048;
int frameBufferWidth, frameBufferHeight;

int main() {

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Graphics Project", NULL, NULL);
    if (window == NULL) {

        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;

    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {

        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    
    }

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    std::string vert = std::string(PROJECT_ROOT) + "/src/shaders/skybox.vert";
    std::string frag = std::string(PROJECT_ROOT) + "/src/shaders/skybox.frag";

    Shader skyboxShader(vert.c_str(), frag.c_str());
    Skybox skybox(skyboxShader);

    vert = std::string(PROJECT_ROOT) + "/src/shaders/default.vert";
    frag = std::string(PROJECT_ROOT) + "/src/shaders/default.frag";

    Shader shader(vert.c_str(), frag.c_str());
    Terrain originPlane(shader, glm::vec3(1.0f));

    //std::string modelPath = std::string(PROJECT_ROOT) + "/assets/models/futureCity/Building2/scene.gltf";
    std::string modelPath = std::string(PROJECT_ROOT) + "/assets/models/city/skyscraperB.glb";
    Model model(modelPath.c_str());

    //Shadow Mapping
    initDepthFBO();

    vert = std::string(PROJECT_ROOT) + "/src/shaders/depth.vert";
    frag = std::string(PROJECT_ROOT) + "/src/shaders/depth.frag";
    Shader depthShader(vert.c_str(), frag.c_str());
    //Shadow Mapping

    shader.use();
    shader.setVec3("lightDir", lightDir);
    shader.setInt("depthMap", 1);

    while (!glfwWindowShouldClose(window)) {

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;

        frames++;
        fTime += deltaTime;
        if (fTime >= 2.0f) {
            float fps = frames / fTime;
            frames = 0;
            fTime = 0;
            std::string newTitle = "Graphics Project : " + std::to_string(fps) + "FPS";
            glfwSetWindowTitle(window, newTitle.c_str());

            lastFrame = currentFrame;
        }

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 modelMatrix = glm::mat4(1.0f);

        glm::vec3 translate = glm::vec3(0.0f, -50.0f, -200.0f);
        glm::vec3 scale = glm::vec3(100.0f, 100.0f, 100.0f);
        glm::vec3 rotateAxis = glm::vec3(0.0f, 1.0f, 0.0f);
        float rotateAngle = 35;

        glm::mat4 modelMatrixTransformed = glm::translate(modelMatrix, translate);
        modelMatrixTransformed = glm::scale(modelMatrixTransformed, scale);
        modelMatrixTransformed = glm::rotate(modelMatrixTransformed, glm::radians(rotateAngle), rotateAxis);

        //Shadow Pass
        depthShader.use();

        float near_plane = 700.0f, far_plane = 2000.0f;
        glm::vec3 lightPosition = camera.Position - lightDir * 1000.0f;
        glm::mat4 lightProjection = glm::ortho(-1000.0f, 1000.0f, -1000.0f, 1000.0f, near_plane, far_plane);
        glm::mat4 lightView = glm::lookAt(lightPosition, camera.Position, glm::vec3(0.0, 1.0, 0.0));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
        glViewport(0, 0, depthMapResolution, depthMapResolution);
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);

        depthShader.setMat4("model", modelMatrix);
        originPlane.render(depthShader, glm::vec3(1.0f));

        depthShader.setMat4("model", modelMatrixTransformed);
        model.render(depthShader);

        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, frameBufferWidth, frameBufferHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //Shadow Pass

        //saveDepthTexture(depthFBO, "depth.png");

        //Regular Pass
        shader.use();

        shader.setMat4("model", modelMatrix);
        shader.setMat4("view", camera.viewMatrix());
        shader.setMat4("projection", camera.projectionMatrix());
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        shader.setVec3("viewPosition", camera.Position);
        shader.setVec3("lightPosition", lightPosition);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);

        shader.setInt("useTexture", 1);

        originPlane.render(shader, glm::vec3(1.0f));

        //models
        shader.setInt("useTexture", 0);

        shader.setMat4("model", modelMatrixTransformed);
        model.render(shader);

        //models

        skybox.render(skyboxShader, camera);
        //Regular

        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    originPlane.deleteBuffers();
    skybox.deleteBuffers();
    
    glfwTerminate();
    return 0;

}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(FORWARD);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(BACKWARD);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(LEFT);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(RIGHT);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.processKeyboard(UP);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.processKeyboard(DOWN);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.processKeyboard(FAST);
    else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE)
        camera.processKeyboard(SLOW);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {

    glViewport(0, 0, width, height);
    frameBufferWidth = width;
    frameBufferHeight = height;

}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.processMouseMovement(xoffset, yoffset);

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {

    camera.processMouseScroll(static_cast<float>(yoffset));

}

void initDepthFBO() {
    
    glGenFramebuffers(1, &depthFBO);
    
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, depthMapResolution, depthMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

static void saveDepthTexture(GLuint fbo, std::string filename) {
    int width =  depthMapResolution;
    int height = depthMapResolution;

    int channels = 3;

    std::vector<float> depth(width * height);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glReadBuffer(GL_DEPTH_COMPONENT);
    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depth.data());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::vector<unsigned char> img(width * height * 3);
    for (int i = 0; i < width * height; ++i) img[3 * i] = img[3 * i + 1] = img[3 * i + 2] = depth[i] * 255;

    stbi_write_png(filename.c_str(), width, height, channels, img.data(), width * channels);
}
