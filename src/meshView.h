#ifndef MESH_VIEW_H
#define MESH_VIEW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// for shaderclass
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cerrno>


// mesh display
void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mousebutton_callback(GLFWwindow* window, int button, int action, int mods);
void mouseposn_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

int displayMesh(std::vector <double> points, std::vector <GLuint> indices);



// OpenGL data processing



// *************** shaderClass *******************
class Shader
{
public:
	GLuint ID;
	Shader(const char* vertexFile, const char* fragmentFile);

	void Activate();
	void Delete();
};



// *************** VBO *******************
class VBO {

public:
	GLuint ID;
	VBO(double* vertices, GLsizeiptr size);


	void Bind();
	void Unbind();
	void Delete();
};



// *************** VAO ******************* //todo
class VAO {

public:
	GLuint ID;
	VAO();

	void LinkVBO(VBO& VBO, GLuint layout);
	void Bind();
	void Unbind();
	void Delete();
};

// *************** EBO *******************
class EBO {

public:
	GLuint ID;
	EBO(GLuint* indices, GLsizeiptr size);


	void Bind();
	void Unbind();
	void Delete();
};


#endif // !MESH_VIEW_H
