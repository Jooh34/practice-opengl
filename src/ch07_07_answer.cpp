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
const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 1080;
float lastX = WINDOW_WIDTH / 2.0;
float lastY = WINDOW_HEIGHT / 2.0;
bool firstMouse = true;
bool cursor_enabled = true;
bool debug_shadow_mode = false;

Model* mesh = nullptr;
Shader* cube_shader = nullptr;
Shader* lightcube_shader = nullptr;
Shader* shadowpass_shader = nullptr;
Shader* debug_shadowpass_shader = nullptr;

Texture* shadowmap_texture = nullptr;

Texture* diffuse_texture = nullptr;
Texture* specular_texture = nullptr;
Texture* plane_texture = nullptr;
Camera* camera = nullptr;

glm::mat4 model_matrix = glm::mat4(1.0f);
glm::mat4 projection_matrix = glm::perspectiveFov(glm::radians(60.0f), float(WINDOW_WIDTH), float(WINDOW_HEIGHT), 0.1f, 100.0f);

unsigned int cubeVAO, lightCubeVAO;
unsigned int planeVAO, planeVBO;


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

	if (key == GLFW_KEY_V && action == GLFW_PRESS)
	{
		debug_shadow_mode = !debug_shadow_mode;
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

	if (cube_shader != nullptr)
	{
		cube_shader->setUniformMatrix4fv("viewProj", projection_matrix * camera->getViewMatrix());
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

void loadPlane()
{
	float planeVertices[] = {
		// positions          // normals             // texture Coords 
		 5.0f, -0.5f,  5.0f,  0.0f,  1.0f, 0.0f,	 2.0f, 0.0f,
		-5.0f, -0.5f,  5.0f,  0.0f,  1.0f, 0.0f,	 0.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f,  1.0f, 0.0f,	 0.0f, 2.0f,

		 5.0f, -0.5f,  5.0f,  0.0f,  1.0f, 0.0f,	 2.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f,  1.0f, 0.0f,	 0.0f, 2.0f,
		 5.0f, -0.5f, -5.0f,  0.0f,  1.0f, 0.0f,	 2.0f, 2.0f
	};

	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	plane_texture = new Texture();
	plane_texture->load("res/models/Stone_Tiles_003_COLOR.png");
}

void loadCube()
{
	// Diffuse and Specular Texture Bind
	diffuse_texture = new Texture();
	diffuse_texture->load("res/models/container_diffuse.png");

	specular_texture = new Texture();
	specular_texture->load("res/models/container_specular.png");

	/* Create and apply basic shader */
	cube_shader = new Shader("ch07_07_shadowmap.vert", "ch07_07_shadowmap_pcf.frag");
	cube_shader->apply();
	cube_shader->setUniform1i("material.diffuse", 0);
	cube_shader->setUniform1i("material.specular", 1);
	cube_shader->setUniform1i("shadowMap", 2);


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
	unsigned int cubeVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(cubeVAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
}

void loadLightCube()
{
	lightcube_shader = new Shader("lightcube.vert", "lightcube.frag");
	lightcube_shader->apply();


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

}

void loadShadowMap()
{
	// configure depth map FBO
	// -----------------------
	shadowmap_texture = new Texture();
	shadowmap_texture->loadDepthMap(2048, 2048);

	// load shader for shadow pass
	shadowpass_shader = new Shader("shadowpass.vert", "shadowpass.frag");
	debug_shadowpass_shader = new Shader("debug_shadowpass.vert", "debug_shadowpass.frag");

	debug_shadowpass_shader->setUniform1i("shadowMap", 0);
}

int loadContent()
{
	camera = new Camera(glm::vec3(0.0f, 0.0f, 3.f), glm::vec3(0.0f, 1.0f, 0.0f));

	loadCube();
	loadPlane();
	loadLightCube();
	loadShadowMap();

	return true;
}

void renderCube(float time, const glm::vec3& objectPos, const Light& light, float shininess, bool bShadowPass)
{
	glm::mat4 m = glm::mat4(1.f);
	m = glm::translate(m, objectPos);
	m = glm::rotate(m, time * glm::radians(-90.0f), glm::vec3(0, 1, 0));

	if (bShadowPass)
	{
		shadowpass_shader->setUniformMatrix4fv("modelMatrix", m);
		shadowpass_shader->setUniformMatrix4fv("world2lightNDC", light.GetWorld2LightNDC());

		shadowpass_shader->apply();
	}
	else // base pass
	{
		diffuse_texture->bind(0);
		specular_texture->bind(1);
		shadowmap_texture->bind(2);
		cube_shader->setUniformMatrix4fv("modelMatrix", m);
		cube_shader->setUniformMatrix4fv("viewMatrix", camera->getViewMatrix());
		cube_shader->setUniformMatrix4fv("projectionMatrix", projection_matrix);
		cube_shader->setUniformMatrix4fv("world2lightNDC", light.GetWorld2LightNDC());

		// for light
		cube_shader->setUniform3fv("cameraPos", camera->getCamPosition());

		cube_shader->setUniform4fv("light.position", light.position);
		cube_shader->setUniform3fv("light.ambient", light.ambient);
		cube_shader->setUniform3fv("light.diffuse", light.diffuse);
		cube_shader->setUniform3fv("light.specular", light.specular);
		cube_shader->setUniform1f("light.constant", light.constant);
		cube_shader->setUniform1f("light.linear", light.linear);
		cube_shader->setUniform1f("light.quadratic", light.quadratic);

		// for material
		cube_shader->setUniform1f("material.shininess", shininess);
		cube_shader->apply();
	}

	// render the cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
}

void renderLightCube(const glm::vec3& pos)
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

void renderPlane(const Light& light, float shininess, bool bShadowPass)
{
	glm::mat4 m = glm::mat4(1.f);
	
	if (bShadowPass)
	{
		shadowpass_shader->setUniformMatrix4fv("modelMatrix", m);
		shadowpass_shader->setUniformMatrix4fv("world2lightNDC", light.GetWorld2LightNDC());

		shadowpass_shader->apply();
	}
	else
	{
		plane_texture->bind(0);
		specular_texture->bind(1);
		shadowmap_texture->bind(2);
		cube_shader->setUniformMatrix4fv("modelMatrix", m);
		cube_shader->setUniformMatrix4fv("viewMatrix", camera->getViewMatrix());
		cube_shader->setUniformMatrix4fv("projectionMatrix", projection_matrix);
		cube_shader->setUniformMatrix4fv("world2lightNDC", light.GetWorld2LightNDC());

		// for light
		cube_shader->setUniform3fv("cameraPos", camera->getCamPosition());

		cube_shader->setUniform4fv("light.position", light.position);
		cube_shader->setUniform3fv("light.ambient", light.ambient);
		cube_shader->setUniform3fv("light.diffuse", light.diffuse);
		cube_shader->setUniform3fv("light.specular", light.specular);
		cube_shader->setUniform1f("light.constant", light.constant);
		cube_shader->setUniform1f("light.linear", light.linear);
		cube_shader->setUniform1f("light.quadratic", light.quadratic);

		// for material
		cube_shader->setUniform1f("material.shininess", shininess);
		cube_shader->apply();
	}

	// floor
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void render(float time)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//Light light(glm::vec4(-0.2f, -1.0f, -0.3f, 0));
	Light light(glm::vec4(0,0,0,1));

	// -------------
	// IMGUI

	static float shininess = 32.f;

	ImGui::SliderFloat("shininess", &shininess, 0, 32.f, "%.3f");
	static glm::vec3 light_position{-2.0f, 2.0f, 0.0f};
	static glm::vec3 light_ambient{1.0f, 1.0f, 1.0f};
	static glm::vec3 light_diffuse{1.0f, 1.0f, 1.0f};
	static glm::vec3 light_specular{1.0f, 1.0f, 1.0f};
	ImGui::SliderFloat3("light_position", (float*)&light_position, -5.0f, 5.0f);
	ImGui::SliderFloat3("light_ambient", (float*)&light_ambient, 0, 1);
	ImGui::SliderFloat3("light_diffuse", (float*)&light_diffuse, 0, 1);
	ImGui::SliderFloat3("light_specular", (float*)&light_specular, 0, 1);
	light.position = glm::vec4(light_position, light.position[3]);
	light.ambient = light_ambient;
	light.diffuse = light_diffuse;
	light.specular = light_specular;

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
	
	// ------ Shadow Pass -----
	shadowmap_texture->bindFrameBuffer();
	glClear(GL_DEPTH_BUFFER_BIT);

	renderPlane(light, shininess, true);
	for (const auto& cubePos : cubePositions)
	{
		renderCube(time, cubePos, light, shininess, true);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// -----------------------------
	
	if (debug_shadow_mode)
	{
		shadowmap_texture->bind(0);
		debug_shadowpass_shader->apply();

		renderQuad();
		return;
	}
	// reset viewport
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	renderPlane(light, shininess, false);

	for (const auto& cubePos : cubePositions)
	{
		renderCube(time, cubePos, light, shininess, false);
	}

	if (light.position[3] == 1) {
		renderLightCube(light.position);
	}
}

void update()
{
	float startTime = static_cast<float>(glfwGetTime());
	float elapsed = 1.0f;
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
	delete cube_shader;
	delete diffuse_texture;
	delete specular_texture;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	return 0;
}