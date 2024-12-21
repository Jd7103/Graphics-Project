#include "Main.h"

const unsigned int width = 1920;
const unsigned int height = 1080;

int main()
{
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

	GLFWwindow* window = glfwCreateWindow(width, height, "Graphics Project", NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	gladLoadGL();
	glViewport(0, 0, width, height);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	Camera camera(width, height, glm::vec3(0.0f, 0.0f, 2.0f));

	double prevTime = 0.0;
	double currentTime = 0.0;
	double timeDiff;
	unsigned int counter = 0;

	glm::vec3 lightDir = glm::vec3(-0.5f, -0.7f, -0.5f);

	Skybox skybox = Skybox();
	skybox.init();

	Model model = Model(string(PROJECT_ROOT) + "/assets/models/city/small_buildingC.glb");

	string vert = string(PROJECT_ROOT) + "/src/shaders/default.vert";
	string frag = string(PROJECT_ROOT) + "/src/shaders/default.frag";
	GLuint shader = LoadShadersFromFile(vert.c_str(), frag.c_str());

	Terrain terrain = Terrain();
	terrain.initPlanar(shader);

	while (!glfwWindowShouldClose(window))
	{
		currentTime = glfwGetTime();
		timeDiff = currentTime - prevTime;
		counter++;

		if (timeDiff >= 1.0 / 30.0)
		{
			std::string FPS = std::to_string((1.0 / timeDiff) * counter);
			std::string ms = std::to_string((timeDiff / counter) * 1000);
			std::string newTitle = "Graphics Project : " + FPS + "FPS / " + ms + "ms";
			glfwSetWindowTitle(window, newTitle.c_str());

			prevTime = currentTime;
			counter = 0;
		}

		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		camera.Inputs(window);
		glm::mat4 view = glm::lookAt(camera.position, camera.position + camera.orientation, camera.up);
		glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)width / height, 0.5f, 2500.0f);

		//Model
		glm::vec3 translate = glm::vec3(250.0f, -50.0f, 0.0f);
		glm::vec3 scale = glm::vec3(100.0f, 100.0f, 100.0f);
		glm::vec3 rotateAxis = glm::vec3(0.0f, 1.0f, 0.0f);
		float rotateAngle = 35;
		model.render(shader, camera, height, width, view, projection, lightDir, scale, translate, rotateAngle, rotateAxis);
		//Model

		terrain.renderPlanar(shader, camera, width, height, view, projection, lightDir);

		skybox.render(camera, width, height, view, projection);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}