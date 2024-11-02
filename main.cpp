
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <iostream>
#include <filesystem>
#define FMT_HEADER_ONLY
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <chrono>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_resize2.h"
#define cimg_display 0
#define cimg_use_png 1
#include "CImg.h"

using namespace std::chrono;
using namespace cimg_library;
namespace fs = std::filesystem;


class GLobj {
  public:
    GLuint vao;
    GLuint vbo;
    GLuint textures;
    unsigned int vertices;
};

GLobj directory;
std::vector<std::filesystem::directory_entry> files;

void GLAPIENTRY MessageCallback( GLenum source,
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

void create_hull(std::vector<GLfloat> *vertices, std::vector<GLfloat> *bary, std::vector<GLuint> *indices, float x_offset, unsigned int index){
	float cube_width = 20.0f;
	float cube_height = 10.0f;
	float cube_depth = 10.0f;

	std::vector<GLfloat> v = {
		-cube_width,cube_depth + x_offset,  cube_height,//0
		cube_width,cube_depth + x_offset,   cube_height,//2
		cube_width,x_offset,  cube_height,//3
		-cube_width,cube_depth + x_offset,  cube_height,//0
		cube_width,x_offset,  cube_height,//3
		-cube_width,x_offset, cube_height,//1
		cube_width,cube_depth + x_offset,   cube_height,//2
		cube_width,cube_depth + x_offset,  0.0f,//6
		cube_width,x_offset, 0.0f,//7
		cube_width,cube_depth + x_offset,   cube_height,//2
		cube_width,x_offset, 0.0f,//7
		cube_width,x_offset,  cube_height,//3
		-cube_width,cube_depth + x_offset, 0.0f,//4 
		-cube_width,x_offset,0.0f,//5
		-cube_width,cube_depth + x_offset,  cube_height,//0
		-cube_width,cube_depth + x_offset,  cube_height,//0
		-cube_width,x_offset,0.0f,//5
		-cube_width,x_offset, cube_height,//1
	};

	std::vector<GLfloat> b = {
		0.0f, 0.0f,//0
		4.0f, 0.0f,//2
		4.0f, 1.0f,//3
		0.0f, 0.0f,//0
		4.0f, 1.0f,//3
		0.0f, 1.0f,//1
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
		indices->push_back(1);
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

GLuint buildTextureArray(std::vector<std::filesystem::directory_entry> files){
	int width = 640, height = 480;
	GLsizei count = files.size(); 

	GLuint texture3D;
	glGenTextures(1, &texture3D);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture3D);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB, width, height, count + 2, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	CImg<unsigned char> floor("../floor.png");
	floor.resize(width, height);
	floor.permute_axes("cxyz");
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, floor.data());
	CImg<unsigned char> ceil("../ceil.png");
	ceil.resize(width, height);
	ceil.permute_axes("cxyz");
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, ceil.data());


	CImg<unsigned char> folder_icon("../folder.png");
	CImg<unsigned char> file_icon("../file.png");
	
	//
	int i = 2;
	for (std::filesystem::directory_entry file : files)
	{
		CImg<unsigned char> image(width,height,1,3);
		
		cimg_forXY(image,x,y) {
			image(x,y,0,0)=255;
			image(x,y,0,1)=0;
			image(x,y,0,2)=255;
		}
		unsigned char cyan[]    = {0,   255, 255 };
		unsigned char black[]   = {0,   0,   0   };
		unsigned char yellow[]  = {255, 255, 0   };
		if (file.is_directory()) {
			image.draw_image(50, 50, folder_icon);
			image.draw_text(30,60, file.path().c_str(), black,cyan,1,64);
		} else {
			if (file.path().extension() == ".png" || file.path().extension() == ".jpg") {
				image = CImg<unsigned char>(file.path().c_str());
				image.resize(width, height);
				image.draw_text(30,60, file.path().c_str(), black,yellow,1,64);
			} else {
				image.draw_image(50, 50, file_icon);
				image.draw_text(30,60, file.path().c_str(), black,yellow,1,64);
			}
		}
		
		image.permute_axes("cxyz");
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, image.data());
		i++;
	}
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	return texture3D;
}

GLuint buildTextureArrayNew(std::vector<std::filesystem::directory_entry> files){
	int width = 640, height = 480;
	GLsizei count = files.size(); 

	GLuint texture3D;
	glGenTextures(1, &texture3D);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture3D);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, width, height, count + 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	int w, h, c;
	unsigned char* floor = (unsigned char*) malloc(width*height*4);
	unsigned char* floor_full = stbi_load("../floor.png", &w, &h, &c, STBI_rgb_alpha);
	stbir_resize_uint8_srgb(floor_full, w, h, 0, floor, width, height, 0, (stbir_pixel_layout)4);
	
	unsigned char* ceil = (unsigned char*) malloc(width*height*4);
	unsigned char* ceil_full = stbi_load("../ceil.png", &w, &h, &c, STBI_rgb_alpha);
	stbir_resize_uint8_srgb(ceil_full, w, h, 0, ceil, width, height, 0, (stbir_pixel_layout)4);

	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, floor);
	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 1, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, ceil);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

	unsigned char* folder_icon_full = stbi_load("../folder.png", &w, &h, &c, STBI_rgb_alpha);
	unsigned char* file_icon_full = stbi_load("../file.png", &w, &h, &c, STBI_rgb_alpha);

	unsigned char* folder_icon = (unsigned char*) malloc(width*height*4);
	unsigned char* file_icon = (unsigned char*) malloc(width*height*4);
    stbir_resize_uint8_srgb(folder_icon_full, w, h, 0, folder_icon, width, height, 0, (stbir_pixel_layout)4);
    stbir_resize_uint8_srgb(file_icon_full, w, h, 0, file_icon, width, height, 0, (stbir_pixel_layout)4);

	unsigned char *blank = (unsigned char*) malloc(width*height*4);
	unsigned char *image = blank;
	
	int i = 3;
	for (std::filesystem::directory_entry file : files)
	{
		if (file.is_directory()) {
			image = folder_icon;
		} else {
			if (file.path().extension() == ".png" || file.path().extension() == ".jpg") {
				image = blank;
				unsigned char* diskimage = stbi_load(file.path().c_str(), &w, &h, &c, STBI_rgb_alpha);
				stbir_resize_uint8_srgb(diskimage, w, h, 0, image, width, height, 0, (stbir_pixel_layout)4);
			} else {
				image = file_icon;
			}
		}
		
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image);
		i++;
	}
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	return texture3D;
}

GLobj buildDirectory(std::string path) {
	files.clear();

    for (const auto & entry : fs::directory_iterator(path)){
		files.push_back(entry);
	}

	fs::directory_entry dd = fs::directory_entry(path.append(fs::path("/..")));
	files.push_back(dd);
	printf("%s\n", dd.path().c_str());

	GLobj data;

	std::vector<GLfloat> vertices;
	std::vector<GLfloat> bary;
	std::vector<GLuint> indices;

	float floor_width = 20.0f;
	float floor_length = files.size() * 11.0 / 2.0;

	GLfloat floorvertices[] = {
		-floor_width, floor_length, 0.0f,
		floor_width, floor_length, 0.0f,
		-floor_width, 0.0f, 0.0f,
		floor_width, floor_length, 0.0f,
		floor_width, 0.0f, 0.0f,
		-floor_width, 0.0f, 0.0f,
	};
	float tar = files.size();
	GLfloat texCoords[] = {
		0.0f, 0.0f,
		5.0f, 0.0f,
		0.0f, tar,
		5.0f, 0.0f,
		5.0f, tar,
		0.0f, tar,
	};
	GLuint texIndex[] = {0,0,0,0,0,0};

	std::copy(std::begin(floorvertices), std::end(floorvertices), std::back_inserter(vertices));
	std::copy(std::begin(texCoords), std::end(texCoords), std::back_inserter(bary));
	std::copy(std::begin(texIndex), std::end(texIndex), std::back_inserter(indices));

	for (int i = 0; i < ceil(files.size() / 2.0f); i++) {
		create_hull(&vertices, &bary, &indices, (float) i * 11.0f, i * 2 + 2);
	}

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
	
	glGenVertexArrays(1, & data.vao);
	glBindVertexArray(data.vao);
	glGenBuffers(1, & data.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, data.vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glBindVertexArray(data.vao);
	glBindBuffer(GL_ARRAY_BUFFER, data.vbo);
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

    std::vector<CImg<unsigned char>> images;
    data.textures = buildTextureArrayNew(files);
	data.vertices = vertices.size();
	return data;
}

const unsigned int SCR_WIDTH = 3840;
const unsigned int SCR_HEIGHT = 2560;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float yaw  = 0.0f;
float pitch  =  0.0f;

glm::vec3 cameraPos = glm::vec3(-0.01f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 walkDirection = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 hitPoint = glm::vec3(0.0f, 0.0f, 0.0f);

int active = 0;

bool jumped = false;

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

bool rayTest_MT( const glm::vec3 origin, const glm::vec3 direction, glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, bool withBackFace, float& len, float& U, float& V, float& W, glm::vec3 &hit )
{
	glm::vec3 e1(v2.x - v1.x,v2.y - v1.y,v2.z - v1.z);
	glm::vec3 e2(v3.x - v1.x,v3.y - v1.y,v3.z - v1.z);
	glm::vec3 pvec = glm::cross(direction, e2);
	float det = glm::dot(e1, pvec);
	
	if( withBackFace )
	{
		if( std::fabs(det) < std::numeric_limits<float>::epsilon() )
			return false;
	}
	else
	{
		if( det < std::numeric_limits<float>::epsilon() && det > -std::numeric_limits<float>::epsilon() )
			return false;
	}

	glm::vec3 tvec(origin.x - v1.x, origin.y - v1.y, origin.z - v1.z);

	float inv_det = 1.f / det;
	U = glm::dot(tvec, pvec) * inv_det;

	if( U < 0.f || U > 1.f )
		return false;

	glm::vec3 qvec = glm::cross(tvec, e1);
	V = glm::dot(direction, qvec) * inv_det;

	if( V < 0.f || U + V > 1.f )
		return false;

	len = glm::dot(e2, qvec) * inv_det;
	
	if( len < std::numeric_limits<float>::epsilon() ) return false;
	
	hit = origin + direction * len;

	W = 1.f - U - V;
	return true;
}

bool debug = false;

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    float cameraSpeed = 0.5f; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraSpeed *= 2.5;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * walkDirection;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * walkDirection;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(walkDirection, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(walkDirection, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && cameraPos.z <= 3) {
		jumped = true;
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
		debug = !debug;
	}

	if (jumped && cameraPos.z <= 6) {
		cameraPos.z += 0.5;
	} else {
		jumped = false;
		if (cameraPos.z > 3.0)
			cameraPos.z -= 0.2;
	}

	// Open folders by walking in
	if (abs(cameraPos.x) > 20.5 && cameraPos.y > 0.0f) {
		int y = static_cast<int>(floor(cameraPos.y / 11.0f));
		int fileidx = y * 2 + (cameraPos.x < 0.0f ? 1 : 0);

		printf("Intersected at %d\n", fileidx);
		printf("Intersected at %s\n", files[fileidx].path().c_str());

		if (files[fileidx].is_directory()) {
			cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
			cameraFront = glm::vec3(0.0f, 1.0f, 0.0f);
			walkDirection = glm::vec3(0.0f, 1.0f, 0.0f);
			firstMouse = true;
			yaw = 0.0f;
			pitch = 0.0f;
			directory = buildDirectory(files[fileidx].path());
		}
	}

	float len, U,V,W;
	rayTest_MT(cameraPos, cameraFront, glm::vec3(-20.0, 150.0, 0.0), glm::vec3(20.0, 150.0, 0.0), glm::vec3(-20.0, 0.0, 0.0), false, len, U, V, W, hitPoint);
	rayTest_MT(cameraPos, cameraFront, glm::vec3(20.0, 150.0, 0.0), glm::vec3(20.0, 0.0, 0.0), glm::vec3(-20.0, 0.0, 0.0), false, len, U, V, W, hitPoint);
	rayTest_MT(cameraPos, cameraFront, glm::vec3(-20.0, 0.0, 0.0), glm::vec3(-20.0, 150.0, 0.0), glm::vec3(-20.0, 0.0, 10.0), false, len, U, V, W, hitPoint);
	rayTest_MT(cameraPos, cameraFront, glm::vec3(-20.0, 150.0, 0.0), glm::vec3(-20.0, 150.0, 10.0), glm::vec3(-20.0, 0.0, 10.0), false, len, U, V, W, hitPoint);
	rayTest_MT(cameraPos, cameraFront, glm::vec3(20.0, 0.0, 0.0), glm::vec3(20.0, 150.0, 0.0), glm::vec3(20.0, 0.0, 10.0), false, len, U, V, W, hitPoint);
	rayTest_MT(cameraPos, cameraFront, glm::vec3(20.0, 150.0, 0.0), glm::vec3(20.0, 150.0, 10.0), glm::vec3(20.0, 0.0, 10.0), false, len, U, V, W, hitPoint);

	int y = static_cast<int>(floor(hitPoint.y / 11.0f));
	active = y * 2 + (hitPoint.x < 0.0f ? 1 : 0);


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

	glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)3840 / (float)2560, 0.1f, 1000.0f);
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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
		std::cout << hitPoint.x << " " << hitPoint.y << " " << hitPoint.z << std::endl;

		printf("Clicked at %d\n", active);
	}
}

int main() {
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
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	

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
		uniform vec3 point;
		uniform uint active;
		uniform sampler2DArray textures;

		float sdCircle( vec2 p, float r )
		{
			return length(p) - r;
		}

		void main()
		{
			if(active == texID){
				gl_FragColor = vec4(texture(textures, vec3(1.0 - texCoords.xy, texID)).rgb, 1.0) + vec4(sdCircle(1.0 - (point - pos).xy, 0.5), 0.0, 0.0, 1.0);
			} else {
				gl_FragColor = vec4(texture(textures, vec3(1.0 - texCoords.xy, texID)).rgb, 1.0);
			}
		}
	)GLSL";

	GLuint ShaderProgram = CreateShader(VertexShaderSource, FragmentShaderSource);

	glUseProgram(ShaderProgram);
	update_camera(ShaderProgram);
	glUseProgram(0);

	directory = buildDirectory("../");

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
		glUniform1ui(glGetUniformLocation(ShaderProgram, "active"), active + 2);
		if (debug) {
			glUniform3fv(glGetUniformLocation(ShaderProgram, "point"), 1, glm::value_ptr(hitPoint));
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D_ARRAY, directory.textures);   
		glBindVertexArray(directory.vao);
		glDrawArrays(GL_TRIANGLES, 0, directory.vertices * sizeof(GLfloat));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		glUseProgram(0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteProgram(ShaderProgram);

	glDeleteBuffers(1, & directory.vbo);
	glDeleteVertexArrays(1, & directory.vao);

	glfwTerminate();
	return 0;
}