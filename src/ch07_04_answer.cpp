/** 
 * Copyright (C) 2018 Tomasz Ga�aj
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
Texture * texture = nullptr;
Camera* camera = nullptr;

glm::mat4 model_matrix      = glm::mat4(1.0f);
glm::mat4 projection_matrix = glm::perspectiveFov(glm::radians(60.0f), float(WINDOW_WIDTH), float(WINDOW_HEIGHT), 0.1f, 10.0f);

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

    /* Create and apply basic shader */
    shader = new Shader("ch07_04.vert", "ch07_04.frag");
    shader->apply();

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
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
	// first, configure the cube's VAO (and VBO)
	unsigned int VBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(cubeVAO);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	// second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
	glGenVertexArrays(1, &lightCubeVAO);
	glBindVertexArray(lightCubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

    return true;
}

void render(float time)
{
	// -------------
	// IMGUI

	static float ambient[3] = { 1.0f, 0.5f, 0.31f };
	static float diffuse[3] = { 1.0f, 0.5f, 0.31f };
	static float specular[3] = { 0.5f, 0.5f, 0.5f };
	static float shininess = 32.f;

	ImGui::ColorEdit3("ambient", ambient);
	ImGui::ColorEdit3("diffuse", diffuse);
	ImGui::ColorEdit3("specular", specular);
	ImGui::SliderFloat("shininess", &shininess, 0, 32.f, "%.3f");
	// --------------------

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::vec3 lightPos = glm::vec3(1.f, 0.5f, 1.f);
    model_matrix = glm::rotate(glm::mat4(1.0f), time * glm::radians(-90.0f), glm::vec3(0, 1, 0));

    shader->setUniformMatrix4fv("modelMatrix", model_matrix);
    shader->setUniformMatrix4fv("viewMatrix", camera->getViewMatrix());
    shader->setUniformMatrix4fv("projectionMatrix", projection_matrix);
    
    // for light
	shader->setUniform3fv("cameraPos", camera->getCamPosition());
    shader->setUniform3fv("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
	shader->setUniform3fv("lightPos", lightPos);
	
	// for material
	shader->setUniform3fv("material.ambient", glm::vec3(ambient[0], ambient[1], ambient[2]));
	shader->setUniform3fv("material.diffuse", glm::vec3(diffuse[0], diffuse[1], diffuse[2]));
	shader->setUniform3fv("material.specular", glm::vec3(specular[0], specular[1], specular[2]));
	shader->setUniform1f("material.shininess", shininess);
    shader->apply();

    // render the cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

	// also draw the lamp object
	model_matrix = glm::mat4(1.0f);
	model_matrix = glm::translate(model_matrix, lightPos);
	model_matrix = glm::scale(model_matrix, glm::vec3(0.2f)); // a smaller cube
	shader->setUniformMatrix4fv("modelMatrix", model_matrix);
	shader->setUniformMatrix4fv("viewMatrix", camera->getViewMatrix());
	shader->setUniformMatrix4fv("projectionMatrix", projection_matrix);

	// for lightCbue
	shader->setUniform3fv("cameraPos", camera->getCamPosition());
	shader->setUniform3fv("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
	shader->setUniform3fv("lightPos", lightPos);

	shader->setUniform3fv("material.ambient", glm::vec3(1,1,1));
	shader->setUniform3fv("material.diffuse", glm::vec3(1,1,1));
	shader->setUniform3fv("material.specular", glm::vec3(1,1,1));
	shader->setUniform1f("material.shininess", 1);

	shader->apply();

	glBindVertexArray(lightCubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
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
    delete texture;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

    return 0;
}