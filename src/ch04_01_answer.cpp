#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "rendering/Shader.h"
#include "rendering/Texture.h"

GLFWwindow* window;
const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;

unsigned int VBO, VAO, EBO;

Shader* shader  = nullptr;
Texture* texture1 = nullptr;
Texture* texture2 = nullptr;

void window_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
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
	glClearColor(0.6784f, 0.8f, 1.0f, 1.0f);
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	return true;
}

void render()
{
	// 프레임 버퍼를 단색으로 초기화
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // state-setting function
	glClear(GL_COLOR_BUFFER_BIT); // state-using function
	
	// 삼각형 그리기
	texture1->bind(0);
	texture2->bind(1);
	shader->apply();
	
	glBindVertexArray(VAO); // 버텍스 어레이 선택
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void update()
{
	float vertices[] = {
		// positions          // colors           // texture coords
		 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
		 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
		-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	texture1 = new Texture();
	texture1->load("res/models/brick.jpg");
	texture1->bind(0);

	texture2 = new Texture();
	texture2->load("res/models/awesomeface.png");
	texture2->bind(1);

	shader = new Shader("ch04_01.vert", "ch04_01.frag");
	shader->apply();
	shader->setUniform1i("texture1", 0);
	shader->setUniform1i("texture2", 1);

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		/* Render here */
		render();

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

int main(void)
{
	if (!init())
		return -1;

	update();

	glfwTerminate();

	return 0;
}