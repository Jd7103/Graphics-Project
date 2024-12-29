#include "Main.h"

//Camera Params
const unsigned int SCREEN_WIDTH = 1920;
const unsigned int SCREEN_HEIGHT = 1080;

Camera camera(glm::vec3(0.0f, 0.0f, 0.0f), 0.5f, 4000.0f, SCREEN_HEIGHT, SCREEN_WIDTH);

//Mouse Control Vars
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
unsigned int depthMapResolution = 4096;
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

    //Skybox Shader
    std::string vert = std::string(PROJECT_ROOT) + "/src/shaders/skybox.vert";
    std::string frag = std::string(PROJECT_ROOT) + "/src/shaders/skybox.frag";

    Shader skyboxShader(vert.c_str(), frag.c_str());
    Skybox skybox(skyboxShader);
    //Skybox Shader


    //Default Instancing Shader
    vert = std::string(PROJECT_ROOT) + "/src/shaders/default.vert";
    frag = std::string(PROJECT_ROOT) + "/src/shaders/default.frag";

    Shader shader(vert.c_str(), frag.c_str());

    shader.use();
    shader.setVec3("lightDir", lightDir);
    shader.setInt("depthMap", 1);
    //Default Instancing Shader


    //Spawn Chunk Shader (No Instancing)
    vert = std::string(PROJECT_ROOT) + "/src/shaders/spawn.vert";
    frag = std::string(PROJECT_ROOT) + "/src/shaders/spawn.frag";
    Shader spawnShader(vert.c_str(), frag.c_str());

    spawnShader.use();
    spawnShader.setVec3("lightDir", lightDir);
    spawnShader.setInt("depthMap", 1);
    //Spawn Chunk Shader (No Instancing)


    //Road & Footpath Shader
    vert = std::string(PROJECT_ROOT) + "/src/shaders/roads.vert";
    frag = std::string(PROJECT_ROOT) + "/src/shaders/roads.frag";

    Shader roadShader = Shader(vert.c_str(), frag.c_str());

    roadShader.use();
    roadShader.setInt("depthMap", 1);
    roadShader.setVec3("lightDir", lightDir);
    //Road & Footpath Shader


    //Shadow Mapping
    initDepthFBO();

    vert = std::string(PROJECT_ROOT) + "/src/shaders/depth.vert";
    frag = std::string(PROJECT_ROOT) + "/src/shaders/depth.frag";
    Shader depthShader(vert.c_str(), frag.c_str());

    vert = std::string(PROJECT_ROOT) + "/src/shaders/spawnDepth.vert";
    frag = std::string(PROJECT_ROOT) + "/src/shaders/spawnDepth.frag";
    Shader spawnDepth(vert.c_str(), frag.c_str());
    //Shadow Mapping


    //Procedural Chunk Generation
    std::vector<std::string> buildingPaths = {

        std::string(PROJECT_ROOT) + "/assets/models/city/skyscraperA.glb",
        std::string(PROJECT_ROOT) + "/assets/models/city/skyscraperB.glb",
        std::string(PROJECT_ROOT) + "/assets/models/city/skyscraperC.glb",
        std::string(PROJECT_ROOT) + "/assets/models/city/skyscraperD.glb",
        std::string(PROJECT_ROOT) + "/assets/models/city/skyscraperE.glb",
        std::string(PROJECT_ROOT) + "/assets/models/city/skyscraperF.glb"

    };

    Generator generator(shader, spawnShader, buildingPaths);
    //Procedural Chunk Generation
    
    
    //Boids
    std::string modelPath = std::string(PROJECT_ROOT) + "/assets/models/gull.glb";

    BoidManager boidManager(modelPath, shader);
    boidManager.initialize(200, 500.0f);
    //Boids

    glm::mat4 modelMatrix = glm::mat4(1.0f);

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

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        //Updates
        processInput(window);
        generator.update(camera);
        boidManager.update(0.08f);


        glm::vec3 lightPosition = camera.Position - lightDir * 1000.0f;
        glm::mat4 lightProjection = glm::ortho(-2500.0f, 2500.0f, -2500.0f, 2500.0f, 700.0f, 4000.0f);
        glm::mat4 lightView = glm::lookAt(lightPosition, camera.Position, glm::vec3(0.0, 1.0, 0.0));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
        //Updates


        //Shadow Pass
        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        spawnDepth.use();
        spawnDepth.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        spawnDepth.setMat4("model", glm::mat4(1.0f));


        glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
        glViewport(0, 0, depthMapResolution, depthMapResolution);
        glClear(GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);

        generator.render(depthShader, spawnDepth, depthShader);
        boidManager.render(depthShader);
  
        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, frameBufferWidth, frameBufferHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        //Shadow Pass


        //Regular Pass

        //Default Instancing Shader
        shader.use();

        shader.setMat4("model", modelMatrix);
        shader.setMat4("view", camera.viewMatrix());
        shader.setMat4("projection", camera.projectionMatrix());
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        shader.setVec3("viewPosition", camera.Position);
        shader.setVec3("lightPosition", lightPosition);

        shader.setInt("useTexture", 0);
        //Default Instancing Shader

        //Roads & Footpaths
        roadShader.use();

        roadShader.setMat4("model", modelMatrix);
        roadShader.setMat4("view", camera.viewMatrix());
        roadShader.setMat4("projection", camera.projectionMatrix());
        roadShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        roadShader.setVec3("viewPosition", camera.Position);
        roadShader.setVec3("lightPosition", lightPosition);
        //Roads & Footpaths


        //Spawn Chunk Shader (No Instancing)
        spawnShader.use();
     
        spawnShader.setMat4("model", modelMatrix);
        spawnShader.setMat4("view", camera.viewMatrix());
        spawnShader.setMat4("projection", camera.projectionMatrix());
        spawnShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
       
        spawnShader.setVec3("viewPosition", camera.Position);
        spawnShader.setVec3("lightPosition", lightPosition);
        //Spawn Chunk Shader (No Instancing)


        //Render
        generator.render(shader, spawnShader, roadShader);
        boidManager.render(shader);
        skybox.render(skyboxShader, camera);
        //Render

        //Regular

        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    //Cleanup w/ Deconstructors
    skybox.~Skybox();
    boidManager.~BoidManager();
    //Cleanup w/ Deconstructors
    
    glfwTerminate();
    return 0;

}

void processInput(GLFWwindow* window) {

    //Terminate Key
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    //Movement Keys
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
    float borderColour[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColour);
    
    glBindFramebuffer(GL_FRAMEBUFFER, depthFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


}