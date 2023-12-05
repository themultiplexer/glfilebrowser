
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <iostream>
#define FMT_HEADER_ONLY
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <chrono>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define cimg_display 0
#include "CImg.h"

using namespace std::chrono;
using namespace cimg_library;


void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}

void PrintProgramInfoLog(GLint const Program)
{
	int InfoLogLength = 0;
	int CharsWritten = 0;

	glGetProgramiv(Program, GL_INFO_LOG_LENGTH, & InfoLogLength);

	if (InfoLogLength > 0)
	{
		GLchar * InfoLog = new GLchar[InfoLogLength];
		glGetProgramInfoLog(Program, InfoLogLength, & CharsWritten, InfoLog);
		std::cout << "Shader Info Log:" << std::endl << InfoLog << std::endl;
		delete [] InfoLog;
	}
}

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

const unsigned int SCR_WIDTH = 3840;
const unsigned int SCR_HEIGHT = 2560;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float yaw         = -90.0f;
float pitch       =  0.0f;

glm::vec3 cameraPos   = glm::vec3(-0.01f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 0.0f, 1.0f);

glm::vec3 walkDirection = glm::vec3(0.0f, 1.0f, 0.0f);

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
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

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
	direction.y = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.z = sin(glm::radians(pitch));
    direction.x = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);

	glm::vec3 wd;
	wd.y = cos(glm::radians(yaw));
    wd.z = 0.0f;
    wd.x = sin(glm::radians(yaw));
	walkDirection = wd;
}  


void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    float cameraSpeed = 0.5f; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraSpeed *= 2.0;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * walkDirection;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * walkDirection;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(walkDirection, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(walkDirection, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && cameraPos.z <= 3.0)
        cameraPos.z += 6.0f;

	if (cameraPos.z > 3.0)
		cameraPos.z -= 0.1;
}

GLuint loadTexture(const char * imagepath){
    int w;
    int h;
    int comp;
    unsigned char* image = stbi_load(imagepath, &w, &h, &comp, STBI_rgb_alpha);

    if(image == nullptr) {
      throw(std::string("Failed to load texture"));
    }

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Return the ID of the texture we just created
    return textureID;
}

GLuint CreateShader(char const *vert, char const *frag) {
	GLint Compiled;
	GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VertexShader, 1, & vert, NULL);
	glCompileShader(VertexShader);
	glGetShaderiv(VertexShader, GL_COMPILE_STATUS, & Compiled);
	if (! Compiled)
	{
		std::cerr << "Failed to compile vertex shader!" << std::endl;
		PrintShaderInfoLog(VertexShader);
	}

	GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShader, 1, & frag, NULL);
	glCompileShader(FragmentShader);
	glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, & Compiled);
	if (!Compiled)
	{
		std::cerr << "Failed to compile fragment shader!" << std::endl;
		PrintShaderInfoLog(FragmentShader);
	}

	GLint Linked;
	GLuint ShaderProgram = glCreateProgram();
	glAttachShader(ShaderProgram, VertexShader);
	glAttachShader(ShaderProgram, FragmentShader);
	glLinkProgram(ShaderProgram);

	glGetProgramiv( ShaderProgram, GL_LINK_STATUS, &Linked); //requesting the status
        if (!Linked)
        {
		std::cerr << "Failed to link shader!" << std::endl;
		PrintProgramInfoLog(ShaderProgram);
	}
	return ShaderProgram;
}

void update_camera(GLuint shader) {
	glm::mat4 trans = glm::mat4(1.0f);
	trans = glm::rotate(trans, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	GLint uniTrans = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(trans));

	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, glm::vec3(0.0f, 0.0f, 1.0f));
	GLint uniView = glGetUniformLocation(shader, "view");
	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

	glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)3840 / (float)2560, 0.1f, 400.0f);
	GLint uniProj = glGetUniformLocation(shader, "proj");
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));
}

void create_box(std::vector<GLfloat> *vertices, std::vector<GLuint> *indices){
	float cube_width = 10.0f;
	float cube_height = 10.0f;
	float cube_depth = 10.0f;

	std::vector<GLfloat> v = {
		-cube_width,cube_depth,  cube_height,//0
		-cube_width,-cube_depth, cube_height,//1
		cube_width,cube_depth,   cube_height,//2
		cube_width,-cube_depth,  cube_height,//3
		-cube_width,cube_depth, 0.0f,//4 
		-cube_width,-cube_depth,0.0f,//5
		cube_width,cube_depth,  0.0f,//6
		cube_width,-cube_depth, 0.0f,//7 
	};
	 std::vector<GLuint> i = {
		0, 2, 3, 0, 3, 1,
		2, 6, 7, 2, 7, 3,
		7, 6, 4, 4, 5, 7,
		4, 5, 0, 0, 5, 1,
		0, 4, 6, 0, 6, 2,
		1, 7, 5, 1, 3, 7,
	};
	std::copy(std::begin(v), std::end(v), std::back_inserter(*vertices));
	std::copy(std::begin(i), std::end(i), std::back_inserter(*indices));
}

void create_hull(std::vector<GLfloat> *vertices, std::vector<GLfloat> *bary, std::vector<GLuint> *indices, float x_offset, unsigned int index){
	float cube_width = 20.0f;
	float cube_height = 5.0f;
	float cube_depth = 5.0f;

	std::vector<GLfloat> v = {
		-cube_width,cube_depth + x_offset,  cube_height * 2.0f,//0
		cube_width,cube_depth + x_offset,   cube_height * 2.0f,//2
		cube_width,-cube_depth + x_offset,  cube_height * 2.0f,//3
		-cube_width,cube_depth + x_offset,  cube_height * 2.0f,//0
		cube_width,-cube_depth + x_offset,  cube_height * 2.0f,//3
		-cube_width,-cube_depth + x_offset, cube_height * 2.0f,//1
		cube_width,cube_depth + x_offset,   cube_height * 2.0f,//2
		cube_width,cube_depth + x_offset,  0.0f,//6
		cube_width,-cube_depth + x_offset, 0.0f,//7
		cube_width,cube_depth + x_offset,   cube_height * 2.0f,//2
		cube_width,-cube_depth + x_offset, 0.0f,//7
		cube_width,-cube_depth + x_offset,  cube_height * 2.0f,//3
		-cube_width,cube_depth + x_offset, 0.0f,//4 
		-cube_width,-cube_depth + x_offset,0.0f,//5
		-cube_width,cube_depth + x_offset,  cube_height * 2.0f,//0
		-cube_width,cube_depth + x_offset,  cube_height * 2.0f,//0
		-cube_width,-cube_depth + x_offset,0.0f,//5
		-cube_width,-cube_depth + x_offset, cube_height * 2.0f,//1
	};

	std::vector<GLfloat> b = {
		0.0f, 1.0f,//0
		1.0f, 1.0f,//2
		0.0f, 1.0f,//3
		0.0f, 1.0f,//0
		0.0f, 1.0f,//3
		1.0f, 1.0f,//1
		1.0f, 1.0f,//2
		1.0f, 0.0f,//6
		0.0f, 0.0f,//7
		1.0f, 1.0f,//2
		0.0f, 0.0f,//7
		0.0f, 1.0f,//3
		0.0f, 0.0f,//4
		1.0f, 0.0f,//5
		0.0f, 1.0f,//0
		0.0f, 1.0f,//0
		1.0f, 0.0f,//5
		1.0f, 1.0f,//1
	};

	for ( int i = 0; i < 6; i++) {
		indices->push_back(0);
	}
	for ( int i = 0; i < 6; i++) {
		indices->push_back(index);
	}
	for ( int i = 0; i < 6; i++) {
		indices->push_back(index + 1);
	}

	std::copy(std::begin(v), std::end(v), std::back_inserter(*vertices));
	std::copy(std::begin(b), std::end(b), std::back_inserter(*bary));
}

GLuint buildTextureArray()
	{

		int width = 640, height = 480;
		GLsizei count = 10; 

		GLuint texture3D;
		glGenTextures(1, &texture3D);
		glBindTexture(GL_TEXTURE_2D_ARRAY, texture3D);

		//glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
		//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, width, height, count, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		int i = 0;
		for (int i = 0; i < count; i++)
		{
			// Create 640x480 image
			CImg<unsigned char> image(width,height,1,3);
			cimg_forXY(image,x,y) {
				image(x,y,0,0)=255;
				image(x,y,0,1)=0;
				image(x,y,0,2)=255;
			}
			unsigned char cyan[]    = {0,   255, 255 };
			unsigned char black[]   = {0,   0,   0   };
			unsigned char yellow[]  = {255, 255, 0   };
			image.draw_text(30,60, fmt::format("Black 64pt on cyan {}!", i).c_str(),black,cyan,1,64);
			image.draw_text(80,200,"Yellow 32pt on black semi-transparent",yellow,black,0.5,32);
			image.save_png(fmt::format("{}.png", i).c_str());
			image.permute_axes("cxyz");
			//printf("%s\n", &image.data());
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, image.data());
		}
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		return texture3D;
	}

int main()
{
	GLFWwindow* window;
	milliseconds start = duration_cast< milliseconds >(
		system_clock::now().time_since_epoch()
	);

	if (!glfwInit())
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
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
	

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cerr << "Error: " << glewGetErrorString(err) << std::endl;
		glfwTerminate();
		return -1;
	}

	char const * VertexShaderSource = R"GLSL(
		#version 330
		layout (location = 0) in vec3 position;
		layout (location = 1) in vec2 tex;
		layout (location = 2) in uint tID;

		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 proj;

		uniform float time;

		out vec2 texCoords;
		out vec3 pos;
		flat out uint texID;

		void main()
		{
			gl_Position = proj * view * model * vec4(position, 1.0);
			pos = position;
			texCoords = tex;
			texID = tID;
		}
	)GLSL";

	char const * FragmentShaderSource = R"GLSL(
		#version 330
		in vec3 pos;
		in vec2 texCoords;
		flat in uint texID;
		uniform float time;
		uniform sampler2DArray textures;

		void main()
		{

			gl_FragColor = vec4(texture(textures, vec3(1.0 - texCoords.xy, texID)).rgb, 1.0);
		}
	)GLSL";

	char const * VertexShaderSource2 = R"GLSL(
		#version 330 core
		layout (location = 0) in vec3 aPos;
		layout (location = 1) in vec2 aTexCoord;

		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 proj;

		out vec2 TexCoord;

		void main()
		{
			gl_Position = proj * view * model * vec4(aPos, 1.0);
			TexCoord = aTexCoord;
		}
	)GLSL";

	char const * FragmentShaderSource2 = R"GLSL(
		#version 330
		in vec2 TexCoord;
		uniform sampler2D ourTexture;

		void main()
		{
			gl_FragColor = texture(ourTexture, TexCoord);
			//gl_FragColor = vec4(TexCoord, 0.0, 1.0);
		}
	)GLSL";

	std::vector<GLfloat> vertices;
	std::vector<GLfloat> bary;
	std::vector<GLuint> indices;

	for (int i = 0; i < 5; i++) {
		create_hull(&vertices, &bary, &indices, (float) i * 20.0f, i * 2);
	}

	fmt::print("{}\n", indices);

	float floor_width = 50.0f;
	float floor_height = 50.0f;

	GLfloat vertices2[] = {
		-floor_width, -floor_width, 0.0f, 0.0f, 0.0f,
		floor_width, -floor_width, 0.0f, 1.0f, 0.0f,
		-floor_width, floor_width, 0.0f, 0.0f, 1.0f,
		floor_width, -floor_width, 0.0f, 1.0f, 0.0f,
		floor_width, floor_width, 0.0f, 1.0f, 1.0f,
		-floor_width, floor_width, 0.0f, 0.0f, 1.0f,
	};

	GLuint barybuffer;
	glGenBuffers(1, &barybuffer);
	glBindBuffer(GL_ARRAY_BUFFER, barybuffer);
	glBufferData(GL_ARRAY_BUFFER, bary.size() * sizeof(GLfloat), &bary[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	
	GLuint indexbuffer;
	glGenBuffers(1, &indexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, indexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	GLuint VAO;
	glGenVertexArrays(1, & VAO);
	glBindVertexArray(VAO);
	GLuint VBO;
	glGenBuffers(1, & VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	
	GLuint VAO2;
	glGenVertexArrays(1, & VAO2);
	glBindVertexArray(VAO2);
	GLuint VBO2;
	glGenBuffers(1, & VBO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLuint ShaderProgram = CreateShader(VertexShaderSource, FragmentShaderSource);
	GLuint ShaderProgram2 = CreateShader(VertexShaderSource2, FragmentShaderSource2);

	//glUseProgram(ShaderProgram);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	glBindBuffer(GL_ARRAY_BUFFER, barybuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, indexbuffer);
	glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, GL_FALSE, 0);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glBindVertexArray(VAO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glUseProgram(ShaderProgram);
	update_camera(ShaderProgram);
	glUseProgram(0);

	glUseProgram(ShaderProgram2);
	update_camera(ShaderProgram2);
	GLuint floorTexture = loadTexture("../test.png");
	glUseProgram(0);


    std::vector<CImg<unsigned char>> images;
    GLuint textureArray = buildTextureArray();

	glEnable(GL_TEXTURE_2D_ARRAY);
	glEnable(GL_DEPTH_TEST);
	// During init, enable debug output
	glEnable              ( GL_DEBUG_OUTPUT );
	glDebugMessageCallback( MessageCallback, 0 );

	while (! glfwWindowShouldClose(window))
	{
		processInput(window);
		milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		float time = (double)((double)ms.count() - (double)start.count()) / 1000.0f;
		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, glm::vec3(0.0f, 0.0f, 1.0f));

		glUseProgram(ShaderProgram);
		glUniformMatrix4fv(glGetUniformLocation(ShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniform1f(glGetUniformLocation(ShaderProgram, "time"), (float)time);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);   
		glBindVertexArray(VAO);

		glDrawArrays(GL_TRIANGLES, 0, vertices.size() * sizeof(GLfloat));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		glUseProgram(0);


		glUseProgram(ShaderProgram2);
		glBindTexture(GL_TEXTURE_2D, floorTexture);
		glUniformMatrix4fv(glGetUniformLocation(ShaderProgram2, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glBindVertexArray(VAO2);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteProgram(ShaderProgram);

	glDeleteBuffers(1, & VBO);
	glDeleteVertexArrays(1, & VAO);

	glfwTerminate();
	return 0;
}