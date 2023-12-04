
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define cimg_display 0
#include "CImg.h"

using namespace std::chrono;
using namespace cimg_library;

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

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Read the file, call glTexImage2D with the right parameters
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);


    // Nice trilinear filtering.
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //glBindTexture(GL_TEXTURE_2D, 0);

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
	if (! Compiled)
	{
		std::cerr << "Failed to compile fragment shader!" << std::endl;
		PrintShaderInfoLog(FragmentShader);
	}

	GLuint ShaderProgram = glCreateProgram();
	glAttachShader(ShaderProgram, VertexShader);
	glAttachShader(ShaderProgram, FragmentShader);
	glLinkProgram(ShaderProgram);
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

void create_hull(std::vector<GLfloat> *vertices, std::vector<GLfloat> *bary, float x_offset){
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
		0.0f, 0.0f, 1.0f,//0
		0.0f, 1.0f, 0.0f,//2
		1.0f, 0.0f, 0.0f,//3
		0.0f, 0.0f, 1.0f,//0
		1.0f, 0.0f, 0.0f,//3
		0.0f, 1.0f, 0.0f,//1
		0.0f, 1.0f, 0.0f,//2
		1.0f, 0.0f, 0.0f,//6
		0.0f, 0.0f, 1.0f,//7
		0.0f, 1.0f, 0.0f,//2
		0.0f, 0.0f, 1.0f,//7
		1.0f, 0.0f, 0.0f,//3
		0.0f, 1.0f, 0.0f,//4
		1.0f, 0.0f, 0.0f,//5
		0.0f, 0.0f, 1.0f,//0
		0.0f, 0.0f, 1.0f,//0
		1.0f, 0.0f, 0.0f,//5
		0.0f, 1.0f, 0.0f,//1
	};

	std::copy(std::begin(v), std::end(v), std::back_inserter(*vertices));
	std::copy(std::begin(b), std::end(b), std::back_inserter(*bary));
}

GLint buildTextureArray(std::vector<CImg<unsigned char>> arrayOfImages)
	{
		int width = arrayOfImages[0].width(), height = arrayOfImages[0].height();
		GLsizei count = (GLsizei)arrayOfImages.size(); 

		GLuint texture3D;
		glGenTextures(1, &texture3D);
		glBindTexture(GL_TEXTURE_2D_ARRAY, texture3D);

		glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_REPEAT);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, width, height, count, 0, GL_BGR, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

		int i = 0;
		for (CImg image : arrayOfImages)
		{
			//printf("%s\n", &image.data());
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, image.width(), image.height(), 1, GL_RGB, GL_UNSIGNED_BYTE, image.data());
			i++;
		}
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
		layout (location = 1) in vec3 barycentric;

		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 proj;

		uniform float time;

		out vec3 bary;
		out vec3 pos;

		void main()
		{
			gl_Position = proj * view * model * vec4(position, 1.0);
			bary = barycentric;
			pos = position;
		}
	)GLSL";

	char const * FragmentShaderSource = R"GLSL(
		#version 330
		in vec3 bary;
		in vec3 pos;
		uniform float time;
		uniform sampler2DArray textures;

    	
		void main()
		{
			float lineWidth = 0.001;
			float f_closest_edge = min(bary.x, min(bary.y, bary.z) ); // see to which edge this pixel is the closest
			float f_width = fwidth(f_closest_edge); // calculate derivative (divide lineWidth by this to have the line width constant in screen-space)
			float f_alpha = smoothstep(lineWidth, lineWidth + f_width, f_closest_edge); // calculate alpha
			gl_FragColor = vec4(vec3(1.0f) * (1.0 - f_alpha), 1.0) + vec4((pos / 10.0f + vec3(1.0))/2.0f, 1.0);

			gl_FragColor = texture(textures, texCoords.stp, 0).rgb;
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

	for (int i = 0; i < 5; i++) {
		create_hull(&vertices, &bary, (float) i * 20.0f);
	}
	
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
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);
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
	loadTexture("../test.png");
	glUseProgram(0);



   // Create 640x480 image
   CImg<unsigned char> image(640,480,1,3);

   // Fill with magenta
   cimg_forXY(image,x,y) {
      image(x,y,0,0)=255;
      image(x,y,0,1)=0;
      image(x,y,0,2)=255;
   }

   // Make some colours
   unsigned char cyan[]    = {0,   255, 255 };
   unsigned char black[]   = {0,   0,   0   };
   unsigned char yellow[]  = {255, 255, 0   };

   // Draw black text on cyan
   image.draw_text(30,60,"Black 64pt on cyan",black,cyan,1,64);

   // Draw yellow partially transparent text on black
   image.draw_text(80,200,"Yellow 32pt on black semi-transparent",yellow,black,0.5,32);

   // Save result image as NetPBM PNM - no libraries required
   image.save_pnm("result.pnm");

   std::vector<CImg<unsigned char>> images;

   images.push_back(image);

   buildTextureArray(images);


	glEnable(GL_DEPTH_TEST);

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


		glBindVertexArray(VAO);

		glDrawArrays(GL_TRIANGLES, 0, vertices.size() * sizeof(GLfloat));

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glUseProgram(0);


		glUseProgram(ShaderProgram2);
		glUniformMatrix4fv(glGetUniformLocation(ShaderProgram2, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glBindVertexArray(VAO2);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

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