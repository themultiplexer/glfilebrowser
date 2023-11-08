
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <string>

void PrintShaderInfoLog(GLint const Shader)
{
	int InfoLogLength = 0;
	int CharsWritten = 0;

	glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, & InfoLogLength);

	if (InfoLogLength > 0)
	{
		GLchar * InfoLog = new GLchar[InfoLogLength];
		glGetShaderInfoLog(Shader, InfoLogLength, & CharsWritten, InfoLog);
		std::cout << "Shader Info Log:" << std::endl << InfoLog << std::endl;
		delete [] InfoLog;
	}
}

int main()
{
	GLFWwindow* window;

	if (! glfwInit())
		return -1;

	glfwWindowHint(GLFW_SAMPLES, 8 );  // defined samples for  GLFW Window
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	window = glfwCreateWindow(3840, 2560, "Hello World", NULL, NULL);
	if (! window)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
		glfwTerminate();
		return -1;
	}

	char const * VertexShaderSource = R"GLSL(
		#version 330
		layout (location = 0) in vec2 position;
		layout (location = 1) in vec3 bary;
		layout (location = 2) in vec2 aOffset;

		out vec3 v_bc;

		void main()
		{
			vec2 pos = position * (gl_InstanceID / 100.0);
			gl_Position = vec4(pos + aOffset, 0.0, 1.0);
			v_bc = bary;
		}
	)GLSL";

	char const * FragmentShaderSource = R"GLSL(
		#version 330
		in vec3 v_bc;
		void main()
		{
			float lineWidth = 0.01;
			float f_closest_edge = min(v_bc.x, min(v_bc.y, v_bc.z) ); // see to which edge this pixel is the closest
			float f_width = fwidth(f_closest_edge); // calculate derivative (divide lineWidth by this to have the line width constant in screen-space)
			float f_alpha = smoothstep(lineWidth, lineWidth + f_width, f_closest_edge); // calculate alpha
			gl_FragColor = vec4(vec3(1.0 - f_alpha), 1.0);
		}
	)GLSL";

	GLfloat const Vertices [] = {
		-0.05f, -0.05f, 0.0f, 0.0f, 1.0f,
		0.05f, -0.05f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.05f, 1.0f, 0.0f, 0.0f
	};

	GLuint VAO;
	glGenVertexArrays(1, & VAO);
	glBindVertexArray(VAO);

	GLuint VBO;
	glGenBuffers(1, & VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glm::vec2 translations[100];
	int index = 0;
	float offset = 0.1f;
	for(int y = -10; y < 10; y += 2)
	{
		for(int x = -10; x < 10; x += 2)
		{
			glm::vec2 translation;
			translation.x = (float)x/10.0f + offset;
			translation.y = (float)y/10.0f + offset;
			translations[index++] = translation;
		}
	}  

	GLuint instanceVBO;
	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 100, &translations[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLint Compiled;
	GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VertexShader, 1, & VertexShaderSource, NULL);
	glCompileShader(VertexShader);
	glGetShaderiv(VertexShader, GL_COMPILE_STATUS, & Compiled);
	if (! Compiled)
	{
		std::cerr << "Failed to compile vertex shader!" << std::endl;
		PrintShaderInfoLog(VertexShader);
	}

	GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShader, 1, & FragmentShaderSource, NULL);
	glCompileShader(FragmentShader);
	glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, & Compiled);
	if (! Compiled)
	{
		std::cerr << "Failed to compile fragment shader!" << std::endl;
		PrintShaderInfoLog(FragmentShader);
	}

	GLuint ShaderProgram = glCreateProgram();
	glAttachShader(ShaderProgram, VertexShader);
	glAttachShader(ShaderProgram, FragmentShader);
	glLinkProgram(ShaderProgram);
	glUseProgram(ShaderProgram);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);	
	glVertexAttribDivisor(2, 1);  

	while (! glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 3, 100);  

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteProgram(ShaderProgram);
	glDeleteShader(FragmentShader);
	glDeleteShader(VertexShader);

	glDeleteBuffers(1, & VBO);
	glDeleteVertexArrays(1, & VAO);

	glfwTerminate();
	return 0;
}