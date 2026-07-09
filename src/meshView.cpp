// takes in points and the connectivity matrix as vectors, displays the mesh

#include "meshView.h"

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 1000
#define SENSITIVITY 3.5

// camera settings
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
float cameraRadius = 30.0f;

glm::vec3 cameraPos;
//glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool firstMouse = true;
bool isPressed = false;
float yaw = 45.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 30.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;


// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int displayMesh(std::vector<double> points,  std::vector <GLuint> indices)
{
	if (points.empty() || indices.empty()) {
		std::cout << "Warning: Mesh data is empty. Nothing to render!" << std::endl;
		return -1;
	}
	
	// Initialise glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Mesh View", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	// frame buffer callback function
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mousebutton_callback);
	glfwSetCursorPosCallback(window, mouseposn_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	// setting viewport
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	// generating VAO, VBO and EBO

	/* Do this if you want to draw additional circles

	VAO VAO1;
	VAO1.Bind();

	// Binding 1st circle data
	VBO VBO1(positionsCircle1, sizeof(positionsCircle1));
	VAO1.LinkVBO(VBO1, 0);

	VAO1.Unbind();
	VBO1.Unbind();

	*/

	// Binding the random point data
	VAO VAO;
	VAO.Bind();
	VBO VBO(points.data(), points.size() * sizeof(double));
	VAO.LinkVBO(VBO, 0);
	EBO EBO(indices.data(), indices.size() * sizeof(GLuint));

	//VAO.Unbind();
	//VBO.Unbind();
	//EBO.Unbind();
	
	// Creating all the shader stuff and compiles the shader program
	Shader shaderProgram("default.vert", "default.frag");

	glEnable(GL_DEPTH_TEST); // corrects 3d depth visualization

	// Main Render Loop
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window); // to take inputs while application is running

		// Render commands here
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // bg colour

		shaderProgram.Activate();

		// Do this after activating (glUseProgram) shader program 
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view;
		glm::mat4 projection = glm::mat4(1.0f);
		//projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, -50.0f, 100.0f);

		const float radius = 10.0f;
		cameraPos.x = cameraTarget.x + cameraRadius * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		cameraPos.y = cameraTarget.y + cameraRadius * sin(glm::radians(pitch));
		cameraPos.z = cameraTarget.z + cameraRadius * cos(glm::radians(pitch)) * sin(glm::radians(yaw));
		view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		//view = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.0f));
		projection = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f); // -z is inside

		unsigned int modelLoc = glGetUniformLocation(shaderProgram.ID, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		unsigned int viewLoc = glGetUniformLocation(shaderProgram.ID, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		unsigned int projectionLoc = glGetUniformLocation(shaderProgram.ID, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

		//VAO1.Bind();
		//glDrawArrays(GL_LINE_STRIP, 0, 51); // use this to draw a circle || use for arrays only not for elems

		VAO.Bind();
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

		// Draw the wireframe (lines)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glLineWidth(1.5f);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

		// Draw the vertices
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		glPointSize(8.0f);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

		// swaps front and back frames
		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	// VAO1.Delete(); deleting every VAO  created
	VAO.Delete();

	// VBO1.Delete(); deleting every VBO created
	VBO.Delete();

	// deleting every EBO created
	EBO.Delete();

	// deleting all shader programs created 
	shaderProgram.Delete();

	// end opengl run sequence
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	
	float panSpeed = 10.0f * deltaTime;

	// get right and up vectors relative to the current camera view
	glm::vec3 viewDir = glm::normalize(cameraTarget - cameraPos);
	glm::vec3 right = glm::normalize(glm::cross(viewDir, cameraUp));
	glm::vec3 up = glm::normalize(glm::cross(right, viewDir));

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraTarget += up * panSpeed;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraTarget -= up * panSpeed;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraTarget -= right * panSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraTarget += right * panSpeed;

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mousebutton_callback(GLFWwindow* window, int button, int action, int mods) 
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		isPressed = true;
	else
		isPressed = false;

}

// glfw: whenever the mouse moves, this callback is called
void mouseposn_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = -xpos + lastX; // reversed to get the dragging effect correct
	float yoffset = ypos - lastY;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.5f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	if (isPressed)
	{
		yaw += xoffset;
		pitch += yoffset;
	}
	
	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// Zoom by changing the radius distance
	cameraRadius -= (float)yoffset * 2.0f;

	// Prevent zooming through the center of the object
	if (cameraRadius < 1.0f)
		cameraRadius = 1.0f;
}


// *************** shaderClass *******************
std::string get_file_contents(const char* filename)
{
	std::ifstream in(filename, std::ios::binary);
	if (in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return contents;
	}
	throw(errno);
}


Shader::Shader(const char* vertexFile, const char* fragmentFile)
{
	std::string vertexCode = get_file_contents(vertexFile);
	std::string fragmentCode = get_file_contents(fragmentFile);

	const char* vertexShaderSource = vertexCode.c_str();
	const char* fragmentShaderSource = fragmentCode.c_str();

	// Creating vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	// Error logging for vertex shader compilation
	int successVertex;
	char infoLogVertex[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &successVertex);

	if (successVertex == GL_FALSE) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLogVertex);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLogVertex << std::endl;
	}

	// Creating Fragment Shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	// Error logging for fragment shader compilation
	int successFragment;
	char infoLogFragment[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &successFragment);

	if (successFragment == GL_FALSE) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLogFragment);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLogFragment << std::endl;
	}

	// Creating Program
	ID = glCreateProgram();
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	glLinkProgram(ID);

	// Error logging for program
	int successProgram;
	char infoLogProgram[512];
	glGetProgramiv(ID, GL_LINK_STATUS, &successProgram);
	if (!successProgram) {
		glGetProgramInfoLog(ID, 512, NULL, infoLogProgram);
		std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLogProgram << std::endl;
	}

	// use the program and delete obsolete shaders
	// glUseProgram(shaderProgram); do this in render loop
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void Shader::Activate() {
	glUseProgram(ID);
}

void Shader::Delete() {
	glDeleteProgram(ID);
}


// *************** VBO *******************
VBO::VBO(double* vertices, GLsizeiptr size) {

	glGenBuffers(1, &ID);
	glBindBuffer(GL_ARRAY_BUFFER, ID);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

void VBO::Bind() {
	glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void VBO::Unbind() {
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::Delete() {
	glDeleteBuffers(1, &ID);
}

// *************** VAO *******************
VAO::VAO() {

	glGenVertexArrays(1, &ID);
}

void VAO::LinkVBO(VBO& VBO, GLuint layout) {

	VBO.Bind();
	glVertexAttribPointer(layout, 3, GL_DOUBLE, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(layout);
	VBO.Unbind();
}

void VAO::Bind() {

	glBindVertexArray(ID);
}

void VAO::Unbind() {

	glBindVertexArray(0);
}

void VAO::Delete() {

	glDeleteVertexArrays(1, &ID);
}


// *************** EBO *******************
EBO::EBO(GLuint* indices, GLsizeiptr size) {

	glGenBuffers(1, &ID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
}

void EBO::Bind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
}

void EBO::Unbind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void EBO::Delete() {
	glDeleteBuffers(1, &ID);
}
