#define _CRT_SECURE_NO_WARNINGS
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <time.h>
#include <stdarg.h>
#define GL_LOG_FILE "gl.log"

bool restart_gl_log()
{
	FILE* file = fopen(GL_LOG_FILE, "w");
	if (!file)
	{
		fprintf(stderr, "Error: could not open GL_LOG_FILE log file %s for writing\n", GL_LOG_FILE);
		return false;
	}
	time_t now = time(NULL);
	char* date = ctime(&now);
	fprintf(file, "GL_LOG_FILE log. local time %s\n", date);
	fclose(file);
	return true;
}

bool gl_log(const char* message, ...)
{
	va_list argptr;
	FILE* file = fopen(GL_LOG_FILE, "a");
	if (!file)
	{
		fprintf(stderr, "Error: could not open GL_LOG_FILE log file %s for appending\n", GL_LOG_FILE);
		return false;
	}
	va_start(argptr, message);
	vfprintf(file, message, argptr);
	va_end(argptr);
	fclose(file);
	return true;
}

static std::string ParseShader(std::string filepath)
{
	std::ifstream stream(filepath);
	if (!stream)
	{
		fprintf(stderr, "Unable to open shader file");
		exit(0);
	}
	std::string line;
	std::string result;
	while (getline(stream, line))
	{
		result += line;
		result += "\n";
	}
	return result;
}

int main()
{
	if(!glfwInit())
	{
		fprintf(stderr, "Error: could not start GLFW3\n");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(640, 480, "Hello Triangle", NULL, NULL);
	if (!window)
	{
		fprintf(stderr, "Error: could not open a window\n");
		glfwTerminate();
		return 1;
	}
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	glewInit();

	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported: %s\n", version);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//FIRST VAO
	//vec3 positions for our square vectors 
	GLfloat points[] =
	{
		0.5f,  0.5f, 0.0f,
	    0.5f, -0.5f, 0.0f,
	   -0.5f, -0.5f, 0.0f,

	   -0.5f, -0.5f, 0.0f,
	   -0.5f,  0.5f, 0.0f,
	    0.5f,  0.5f, 0.0f
	};

	

	//create a vertex buffer object(vbo) to pass on our positions array to the GPU 
	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

	//create a vertex attribute object(VAO) so that we dont have to bind each vertex buffer object every time we draw a mesh.
	//VAO remembers all the vertex buffers that you want and the memory layout of each one. 
	//Set up the VAO once per mesh and bind it before every mesh you want to draw.
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	//SECOND VAO
	//vec3 positions for our triangle vectors
	GLfloat points2[] =
	{
		0.5f, -0.5f, 0.0f,
		0.5f,  0.5f, 0.0f,
	    1.0f,  1.0f, 0.0f
	};

	//create a vertex buffer object(vbo) to pass on our positions array to the GPU 
	GLuint vbo2 = 0;
	glGenBuffers(1, &vbo2);
	glBindBuffer(GL_ARRAY_BUFFER, vbo2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points2), points2, GL_STATIC_DRAW);

	//create a vertex attribute object(VAO) so that we dont have to bind each vertex buffer object every time we draw a mesh.
	//VAO remembers all the vertex buffers that you want and the memory layout of each one. 
	//Set up the VAO once per mesh and bind it before every mesh you want to draw.
	GLuint vao2 = 0;
	glGenVertexArrays(1, &vao2);
	glBindVertexArray(vao2);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	//THIRD VAO
	//vec3 positions for our triangle vectors
	GLfloat points3[] =
	{
		0.5f,  0.5f, 0.0f,
	   -0.5f,  0.5f, 0.0f,
		1.0f,  1.0f, 0.0f
	};

	//create a vertex buffer object(vbo) to pass on our positions array to the GPU 
	GLuint vbo3 = 0;
	glGenBuffers(1, &vbo3);
	glBindBuffer(GL_ARRAY_BUFFER, vbo3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points3), points3, GL_STATIC_DRAW);

	//create a vertex attribute object(VAO) so that we dont have to bind each vertex buffer object every time we draw a mesh.
	//VAO remembers all the vertex buffers that you want and the memory layout of each one. 
	//Set up the VAO once per mesh and bind it before every mesh you want to draw.
	GLuint vao3 = 0;
	glGenVertexArrays(1, &vao3);
	glBindVertexArray(vao3);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo3);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	//FIRST SHADER PURPLE COLOUR
	//parse vertex shader from external file
	std::string vertex_source = ParseShader("test.vert");
	const GLchar * vertex_shader = (const GLchar *)vertex_source.c_str();

	//parse fragment shader from external file
	std::string fragment_source = ParseShader("test.frag");
	const GLchar * fragment_shader = (const GLchar *)fragment_source.c_str();

	//compile the vertex shader
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader, NULL);
	glCompileShader(vs);

	//compile the framgent shader
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader, NULL);
	glCompileShader(fs);

	//using both the compiled vertex shader and compiled framgent shader we create a single, executable GPU shader program
	GLuint shader_program_purple = glCreateProgram();
	glAttachShader(shader_program_purple, fs);
	glAttachShader(shader_program_purple, vs);
	glLinkProgram(shader_program_purple);

	//SECOND SHADER RED COLOUR
	//parse vertex shader from external file
	std::string vertex_source2 = ParseShader("test.vert");
	const GLchar * vertex_shader2 = (const GLchar *)vertex_source2.c_str();

	//parse fragment shader from external file
	std::string fragment_source2 = ParseShader("test2.frag");
	const GLchar * fragment_shader2 = (const GLchar *)fragment_source2.c_str();

	//compile the vertex shader
	GLuint vs2 = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs2, 1, &vertex_shader2, NULL);
	glCompileShader(vs2);

	//compile the framgent shader
	GLuint fs2 = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs2, 1, &fragment_shader2, NULL);
	glCompileShader(fs2);

	//using both the compiled vertex shader and compiled fragment shader we create a single, executable GPU shader program
	GLuint shader_program_red = glCreateProgram();
	glAttachShader(shader_program_red, fs2);
	glAttachShader(shader_program_red, vs2);
	glLinkProgram(shader_program_red);

	//THIRD SHADER BLUE COLOUR
	//parse vertex shader from external file
	std::string vertex_source3 = ParseShader("test.vert");
	const GLchar * vertex_shader3 = (const GLchar *)vertex_source3.c_str();

	//parse fragment shader from external file
	std::string fragment_source3 = ParseShader("test3.frag");
	const GLchar * fragment_shader3 = (const GLchar *)fragment_source3.c_str();

	//compile the framgent shader
	GLuint fs3 = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs3, 1, &fragment_shader3, NULL);
	glCompileShader(fs3);

	//using both the compiled vertex shader and compiled fragment shader we create a single, executable GPU shader program
	GLuint shader_program_blue = glCreateProgram();
	glAttachShader(shader_program_blue, vs2);
	glAttachShader(shader_program_blue, fs3);
	glLinkProgram(shader_program_blue);

	//sets clear color to grey
	glClearColor(0.5, 0.5, 0.5, 1.0);
	//drawing loop
	while (!glfwWindowShouldClose(window))
	{
		//wipe the drawing surface clear
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shader_program_purple);
		glBindVertexArray(vao);
		//draw points 0-3 from the currently bound VAO with current in-use shader
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glUseProgram(shader_program_red);
		glBindVertexArray(vao2);
		//draw points 0-3 from the currently bound VAO with current in-use shader
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glUseProgram(shader_program_blue);
		glBindVertexArray(vao3);
		//draw points 0-3 from the currently bound VAO with current in-use shader
		glDrawArrays(GL_TRIANGLES, 0, 3);

		//update other events like the input handling
		glfwPollEvents();
		//put the stuff we've been drawing onto the display
		glfwSwapBuffers(window);
			
	}

	glDeleteProgram(shader_program_purple);
	glDeleteProgram(shader_program_red);
	glDeleteProgram(shader_program_blue);

	glfwTerminate();
	return 0;
}