#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "rendering/Shader.h"

GLFWwindow* window;
const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;

unsigned int VAO;

Shader* shader  = nullptr;

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
	shader->apply();
	float timeValue = glfwGetTime();
	float greenValue = sin(timeValue) / 2.0f + 0.5f;
	shader->setUniform4fv("ourColor", glm::vec4(0.f, greenValue, 0.f, 1.f));
	
	glBindVertexArray(VAO); // 버텍스 어레이 선택
	glDrawArrays(GL_TRIANGLES, 0, 3); // 드로우 콜
}

void update()
{
	float vertices[] = {
		// first triangle
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 0.0f,  0.5f, 0.0f
	};

	unsigned int VBO;
	glGenBuffers(1, &VBO);

	// GL_ARRAY_BUFFER : 버텍스 특성을 담는 버퍼.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // (종류, 버퍼크기, 포인터 주소, 옵션)
	
	// Vertex Array Object 만들고, 바인딩
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Vertex attributes pointer 세팅하기
	// => 아까 세팅한 VBO를 어떻게 해석할지 정의.

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	shader = new Shader("ch03_01.vert", "ch03_01.frag");
	shader->apply();

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
}

int main(void)
{
	if (!init())
		return -1;

	update();

	glfwTerminate();

	return 0;
}