/** 
 * Copyright (C) 2022 Jooh
 **/

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define  GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "rendering/Shader.h"
#include "rendering/Texture.h"
#include "rendering/Model.h"
#include "rendering/Camera.h"
#include "rendering/Light.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"


GLFWwindow* window;
const int WINDOW_WIDTH  = 1920;
const int WINDOW_HEIGHT = 1080;
float lastX = WINDOW_WIDTH / 2.0;
float lastY = WINDOW_HEIGHT / 2.0;
bool firstMouse = true;
bool cursor_enabled = true;

Model   * mesh    = nullptr;
Shader  * shader  = nullptr;
Shader  * lightcube_shader  = nullptr;
Texture* diffuse_texture = nullptr;
Texture* specular_texture = nullptr;
Camera* camera = nullptr;

glm::mat4 model_matrix      = glm::mat4(1.0f);
glm::mat4 projection_matrix = glm::perspectiveFov(glm::radians(60.0f), float(WINDOW_WIDTH), float(WINDOW_HEIGHT), 0.1f, 100.0f);

unsigned int cubeVAO, lightCubeVAO;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
	{
		if (cursor_enabled)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		cursor_enabled = !cursor_enabled;
	}
}

void processInput(GLFWwindow* window, float deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (camera)
	{
		camera->processInput(window, deltaTime);
	}
}

void mouse_callback(GLFWwindow* window, double xpos_in, double ypos_in)
{
	if (cursor_enabled) return;

	float xpos = static_cast<float>(xpos_in);
	float ypos = static_cast<float>(ypos_in);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;

	if (camera)
	{
		camera->processMouseMovement(xoffset, yoffset);
	}
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    projection_matrix = glm::perspectiveFov(glm::radians(60.0f), float(width), float(height), 0.1f, 10.0f);

    if (shader != nullptr)
    {
        shader->setUniformMatrix4fv("viewProj", projection_matrix * camera->getViewMatrix());
    }
}

int init()
{
    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello Modern GL!", nullptr, nullptr);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSetWindowSizeCallback(window, window_size_callback);

    /* Initialize glad */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    /* Set the viewport */
    glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    glEnable(GL_DEPTH_TEST);
    
    // mouse focus
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // mouse callback
    glfwSetCursorPosCallback(window, mouse_callback);
	
	glfwSetKeyCallback(window, key_callback);

	// IMGUI
	// ------------
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();

    return true;
}

int loadContent()
{
    camera = new Camera(glm::vec3(0.0f, 0.0f, 3.f), glm::vec3(0.0f, 1.0f, 0.0f));

	// Diffuse and Specular Texture Bind
	diffuse_texture = new Texture();
	diffuse_texture->load("res/models/container_diffuse.png");
	diffuse_texture->bind(0);

	specular_texture = new Texture();
	specular_texture->load("res/models/container_specular.png");
	specular_texture->bind(1);

    /* Create and apply basic shader */
    shader = new Shader("ch07_06.vert", "ch07_06.frag");
    shader->apply();
	shader->setUniform1i("material.diffuse", 0);
	shader->setUniform1i("material.specular", 1);

    lightcube_shader = new Shader("lightcube.vert", "lightcube.frag");
	lightcube_shader->apply();

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
	};

	// first, configure the cube's VAO (and VBO)
	unsigned int VBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(cubeVAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);


	// second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
	float lightcube_vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};
	unsigned int lightCubeVBO;
	glGenBuffers(1, &lightCubeVBO);

	glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(lightcube_vertices), lightcube_vertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &lightCubeVAO);
	glBindVertexArray(lightCubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

    return true;
}

void renderCube(float time, const glm::vec3& objectPos, const Light& light, float shininess)
{
	glm::mat4 m = glm::mat4(1.f);
	m = glm::translate(m, objectPos);
    m = glm::rotate(m, time * glm::radians(-90.0f), glm::vec3(0, 1, 0));

	diffuse_texture->bind(0);
	specular_texture->bind(1);
	shader->setUniformMatrix4fv("modelMatrix", m);
	shader->setUniformMatrix4fv("viewMatrix", camera->getViewMatrix());
	shader->setUniformMatrix4fv("projectionMatrix", projection_matrix);

	// for light
	shader->setUniform3fv("cameraPos", camera->getCamPosition());

	shader->setUniform4fv("light.position", light.position);
	shader->setUniform3fv("light.ambient", light.ambient);
	shader->setUniform3fv("light.diffuse", light.diffuse);
	shader->setUniform3fv("light.specular", light.specular);
	shader->setUniform1f("light.constant", light.constant);
	shader->setUniform1f("light.linear", light.linear);
	shader->setUniform1f("light.quadratic", light.quadratic);

	// for material
	shader->setUniform1f("material.shininess", shininess);
	shader->apply();

	// render the cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void renderLightCube(const glm::vec3 &pos)
{
	model_matrix = glm::mat4(1.0f);
	model_matrix = glm::translate(model_matrix, pos);
	model_matrix = glm::scale(model_matrix, glm::vec3(0.2f)); // a smaller cube
	lightcube_shader->setUniformMatrix4fv("modelMatrix", model_matrix);
	lightcube_shader->setUniformMatrix4fv("viewMatrix", camera->getViewMatrix());
	lightcube_shader->setUniformMatrix4fv("projectionMatrix", projection_matrix);

	lightcube_shader->apply();

	glBindVertexArray(lightCubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void render(float time)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//Light light(glm::vec4(-0.2f, -1.0f, -0.3f, 0));
	Light light(glm::vec4(1.f, 1.f, 1.f, 0));

	// -------------
	// IMGUI

	static float shininess = 32.f;

	ImGui::SliderFloat("shininess", &shininess, 0, 32.f, "%.3f");
	static glm::vec3 light_position{1.0f, 1.0f, 1.0f};
	ImGui::SliderFloat3("light_position", (float*)&light_position, -5.0f, 5.0f);
	light.position = glm::vec4(light_position, light.position[3]);
	
	// --------------------

	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	for (const auto& cubePos : cubePositions)
	{
		renderCube(time, cubePos, light, shininess);
	}
	
	if (light.position[3] == 1) {
		renderLightCube(light.position);
	}
}

void update()
{
    float startTime = static_cast<float>(glfwGetTime());
    float elapsed  = 1.0f;
    float gameTime = 0.0f;
    float frameStart = startTime;
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

        float deltaTime = static_cast<float>(glfwGetTime()) - frameStart;
        frameStart = static_cast<float>(glfwGetTime());
        gameTime = frameStart - startTime;

		processInput(window, deltaTime);

        /* Render here */
        render(gameTime);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        /* Swap front and back buffers */
        glfwSwapBuffers(window);
		
        /* Poll for and process events */
        glfwPollEvents();

    }
}

int main(void)
{
    if (!init())
        return -1;

    if (!loadContent())
        return -1;

    update();

    glfwTerminate();

    delete mesh;
    delete shader;
    delete diffuse_texture;
    delete specular_texture;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

    return 0;
}