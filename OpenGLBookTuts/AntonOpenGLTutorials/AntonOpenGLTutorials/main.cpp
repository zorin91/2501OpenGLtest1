#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <istream>
#include <string>
#include <sstream>
#include <time.h>
#include <stdarg.h>
#include "maths_funcs.h"
#define GL_LOG_FILE "gl.log"
#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define STARTMENU 0
#define GAMEPLAY  1
#define SHOP      2

typedef struct
{
	float x;
	float y;
	float z;
	int behaviour;
	int hp;
}enemy_t;

void initEnemy(enemy_t& enemy)
{
	enemy.x = rand() % 10;
	enemy.y = rand() % 10;
	enemy.z = rand() % 10;
	enemy.behaviour = rand() % 3;
	enemy.hp = 100;
}

int gamestate = GAMEPLAY;
float clearColors[3][3] = { {1.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 1.0} };

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
	{
		gamestate++;
		gamestate %= 3;
		glClearColor(clearColors[gamestate][0], clearColors[gamestate][1], clearColors[gamestate][2], 1.0f);
	}
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

mat4 createCameraViewMatrix()
{
	return mat4();
}

bool load_cube_map_side(
	GLuint texture, GLenum side_target, const char* file_name) {
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

	int x, y, n;
	int force_channels = 4;
	unsigned char*  image_data = stbi_load(
		file_name, &x, &y, &n, force_channels);
	if (!image_data) {
		fprintf(stderr, "ERROR: could not load %s\n", file_name);
		return false;
	}
	// non-power-of-2 dimensions check
	if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
		fprintf(stderr,
			"WARNING: image %s is not power-of-2 dimensions\n",
			file_name);
	}

	// copy image data into 'target' side of cube map
	glTexImage2D(
		side_target,
		0,
		GL_RGBA,
		x,
		y,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		image_data);
	free(image_data);
	return true;
}

void create_cube_map(
	const char* front,
	const char* back,
	const char* top,
	const char* bottom,
	const char* left,
	const char* right,
	GLuint* skybox) {
	// generate a cube-map texture to hold all the sides
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, skybox);

	// load each image and copy into a side of the cube-map texture
	load_cube_map_side(*skybox, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, front);
	load_cube_map_side(*skybox, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, back);
	load_cube_map_side(*skybox, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, top);
	load_cube_map_side(*skybox, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, bottom);
	load_cube_map_side(*skybox, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, left);
	load_cube_map_side(*skybox, GL_TEXTURE_CUBE_MAP_POSITIVE_X, right);
	// format cube map texture
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

//code taken from https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
//my own notes added
bool solveQuadratic(const float &a, const float &b, const float &c, float &x0, float &x1)
{
	//calculate discriminant to know how many intersections we have.
	//if discr > 0 then 2 intersections (typical case)
	//if discr == 0 then 1 intersection (case when the intersection is just on the edge of the sphere)
	//if discr < 0 then no intersections( complete miss by the ray)
	float discr = b * b - 4 * a * c;
	if (discr < 0)
	{
		return false;
	}
	else if (discr == 0)
	{
		x0 = x1 = -0.5 * b / a;
	}
	else 
	{
		float q = (b > 0) ?
			-0.5 * (b + sqrt(discr)) :
			-0.5 * (b - sqrt(discr));
		x0 = q / a;
		x1 = c / q;
	}
	if (x0 > x1)
	{
		std::swap(x0, x1);
	}

	return true;
}

#define RELPATH "../../external resources/skybox/"

int main()
{
	if(!glfwInit())
	{
		fprintf(stderr, "Error: could not start GLFW3\n");
		return 1;
	}
	//make it fullscreen
	GLFWmonitor* mon = glfwGetPrimaryMonitor();
	const GLFWvidmode* vmode = glfwGetVideoMode(mon);
	
	//switch first NuLL to mon to go to fullscreen and vice versa
	GLFWwindow* window = glfwCreateWindow(vmode->width/2 , vmode->height/2 , "Hello Triangle", NULL, NULL);
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
	//printf("Renderer: %s\n", renderer);
	//printf("OpenGL version supported: %s\n", version);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//enable vsync
	glfwSwapInterval(1);


	//set the cursor mode to GLFW_CURSOR_DISABLED. (GLFW will then take care of all the details of cursor re-centering and offset calculation and providing the application with a virtual cursor position)
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//keep in mind which winding order you are making new shapes in for culling.(note this setting is global, you can turn it off in between draw calls to make some items two sided)
	//glEnable(GL_CULL_FACE); //cull face
	//glCullFace(GL_BACK); //cull face back
	//glFrontFace(GL_CW); //GL_CW for clock-wise ... GL_CCW for counter clock-wise


	//note on order of operations for translations and rotations when using column major matrices(what we use)
	// V' = T * R * V -> rotate, then translate
	// V' = R * T * V -> rotate around a point(orbit?)
	// V' = V * T * R -> INVALID!!
	//in short, order matters!


	float enemy_ship_z = -4.0f;
	float enemy_ship_x = 0.0f;
	float enemy_ship_y = 0.0f;

	float asteroid_z = -4.0f;
	float asteroid_x = 0.0f;
	float asteroid_y = 0.0f;


	//matrix for translation demo             //when using a 1d array as a 4x4 matrix this is the order of elements
	float matrix[] = 
	{           
		1.0f, 0.0f, 0.0f, 0.0f,									//0, 4,  8, 12,
		0.0f, 1.0f, 0.0f, 0.0f,									//1, 5,  9, 13,
		0.0f, 0.0f, 1.0f, 0.0f,									//2, 6, 10, 14,
		enemy_ship_x, enemy_ship_y, enemy_ship_z, 1.0f									//3, 7, 11, 15,
	};

	float matrix2[] =
	{
		1.0f, 0.0f, 0.0f, 0.0f,									//0, 4,  8, 12,
		0.0f, 1.0f, 0.0f, 0.0f,									//1, 5,  9, 13,
		0.0f, 0.0f, 1.0f, 0.0f,									//2, 6, 10, 14,
		enemy_ship_x, enemy_ship_y, enemy_ship_z, 1.0f			//3, 7, 11, 15,
	};

	float matrix3[] =
	{
		6.0f, 0.0f, 0.0f, 0.0f,									//0, 4,  8, 12,
		0.0f, 6.0f, 0.0f, 0.0f,									//1, 5,  9, 13,
		0.0f, 0.0f, 6.0f, 0.0f,									//2, 6, 10, 14,
		asteroid_x, asteroid_y, asteroid_z, 1.0f				//3, 7, 11, 15,
	};

	float points_skybox[] = {
		-100.0f,  100.0f, -100.0f,
		-100.0f, -100.0f, -100.0f,
		100.0f, -100.0f, -100.0f,
		100.0f, -100.0f, -100.0f,
		100.0f,  100.0f, -100.0f,
		-100.0f,  100.0f, -100.0f,

		-100.0f, -100.0f,  100.0f,
		-100.0f, -100.0f, -100.0f,
		-100.0f,  100.0f, -100.0f,
		-100.0f,  100.0f, -100.0f,
		-100.0f,  100.0f,  100.0f,
		-100.0f, -100.0f,  100.0f,

		100.0f, -100.0f, -100.0f,
		100.0f, -100.0f,  100.0f,
		100.0f,  100.0f,  100.0f,
		100.0f,  100.0f,  100.0f,
		100.0f,  100.0f, -100.0f,
		100.0f, -100.0f, -100.0f,

		-100.0f, -100.0f,  100.0f,
		-100.0f,  100.0f,  100.0f,
		100.0f,  100.0f,  100.0f,
		100.0f,  100.0f,  100.0f,
		100.0f, -100.0f,  100.0f,
		-100.0f, -100.0f,  100.0f,

		-100.0f,  100.0f, -100.0f,
		100.0f,  100.0f, -100.0f,
		100.0f,  100.0f,  100.0f,
		100.0f,  100.0f,  100.0f,
		-100.0f,  100.0f,  100.0f,
		-100.0f,  100.0f, -100.0f,

		-100.0f, -100.0f, -100.0f,
		-100.0f, -100.0f,  100.0f,
		100.0f, -100.0f, -100.0f,
		100.0f, -100.0f, -100.0f,
		-100.0f, -100.0f,  100.0f,
		100.0f, -100.0f,  100.0f
	};
	GLuint vbo_sky;
	glGenBuffers(1, &vbo_sky);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_sky);
	glBufferData(GL_ARRAY_BUFFER, 3 * 36 * sizeof(float), &points_skybox, GL_STATIC_DRAW);

	GLuint vao_sky;
	glGenVertexArrays(1, &vao_sky);
	glBindVertexArray(vao_sky);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_sky);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);



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
		-0.01f,  -1.0f, 0.0f,
	   0.01f,  -1.0f, 0.0f,
		0.0f,  0.12f, -1.0f
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



	//Third VAO
	//Define vertex colours example
	GLfloat interpExamplePoints[] = 
	{
		0.0f,0.5f,0.0f,
		0.5f,-0.5f,0.5f,
		-0.5f,-0.5f,0.5f,

		0.0f,0.5f,0.0f,
		-0.5f,-0.5f,-0.5f,
		0.5f,-0.5f,-0.5f,

		0.0f,0.5f,0.0f,
		0.5f,-0.5f,-0.5f,
		0.5f,-0.5f,0.5f,

		0.0f,0.5f,0.0f,
		-0.5f,-0.5f,0.5f,
		-0.5f,-0.5f,-0.5f

	};
	GLfloat interpExampleColours[] = 
	{
		1.0f,0.0f,0.0f,
		0.0f,1.0f,0.0f,
		0.0f,0.0f,1.0f,

		1.0f,0.0f,0.0f,
		1.0f,0.0f,0.0f,
		1.0f,0.0f,0.0f,
		

		1.0f,0.0f,0.0f,
		0.0f,0.0f,1.0f,
		0.0f,1.0f,0.0f,

		1.0f,0.0f,0.0f,
		0.0f,0.0f,1.0f,
		0.0f,1.0f,0.0f
	};

	//create a vertex buffer object(vbo) to pass on our positions array to the GPU (Define vertex colours example)
	GLuint points_vbo = 0;
	glGenBuffers(1, &points_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(interpExamplePoints), interpExamplePoints, GL_STATIC_DRAW);

	//create a vertex buffer object(vbo) to pass on our positions array to the GPU (Define vertex colours example)
	GLuint colours_vbo = 0;
	glGenBuffers(1, &colours_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, colours_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(interpExampleColours), interpExampleColours, GL_STATIC_DRAW);

	//create a vertex attribute object(VAO) so that we dont have to bind each vertex buffer object every time we draw a mesh.
	//VAO remembers all the vertex buffers that you want and the memory layout of each one. 
	//Set up the VAO once per mesh and bind it before every mesh you want to draw.
	GLuint vao4 = 0;
	glGenVertexArrays(1, &vao4);
	glBindVertexArray(vao4);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, colours_vbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);


//Fourth VAO a square to draw our ship
	GLfloat textureExamplePoints[] =
	{
		//model pointing in the -z direction

		//top view
		//top 3 squares
		-0.5f,1.0f,1.5f,
		-0.5f,1.0f,0.5f,
		0.5f,1.0f,0.5f,

		0.5f,1.0f,0.5f,
		0.5f,1.0f,1.5f,
		-0.5f,1.0f,1.5f,

		-1.5f,1.0f,1.5f,
		-1.5f,1.0f,0.5f,
		-0.5f,1.0f,0.5f,

		-0.5f,1.0f,0.5f,
		-0.5f,1.0f,1.5f,
		-1.5f,1.0f,1.5f,

		0.5f,1.0f,1.5f,
		0.5f,1.0f,0.5f,
		1.5f,1.0f,0.5f,

		1.5f,1.0f,0.5f,
		1.5f,1.0f,1.5f,
		0.5f,1.0f,1.5f,
		//top 3 forward rectangles
		-0.5f,1.0f,0.5f,
		-0.5f,0.0f,-1.5f,
		0.5f,0.0f,-1.5f,

		0.5f,0.0f,-1.5f,
		0.5f,1.0f,0.5f,
		-0.5f,1.0f,0.5f,

		-1.5f,1.0f,0.5f,
		-1.5f,0.0f,-1.5f,
		-0.5f,0.0f,-1.5f,

		-0.5f,0.0f,-1.5f,
		-0.5f,1.0f,0.5f,
		-1.5f,1.0f,0.5f,

		0.5f,1.0f,0.5f,
		0.5f,0.0f,-1.5f,
		1.5f,0.0f,-1.5f,

		1.5f,0.0f,-1.5f,
		1.5f,1.0f,0.5f,
		0.5f,1.0f,0.5f,
		//top long side
		-1.5f, 0.0f,-1.5f,
		-1.5f, 1.0f,1.5f,
		-2.5f, 0.0f,1.5f,

		1.5f, 0.0f,-1.5f,
		2.5f, 0.0f,1.5f,
		1.5f, 1.0f,1.5f,
		//top side fillers
		-1.5f, 1.0f,0.5f,
		-1.5f, 1.0f,1.5f,
		-1.5f, 0.0f,-1.5f,

		1.5f, 1.0f,0.5f,
		1.5f, 0.0f,-1.5f,
		1.5f, 1.0f,1.5f,

		//back side
		-1.5f, 1.0f,1.5f,
		1.5f, 1.0f,1.5f,
		2.5f, -1.0f,1.5f,

		2.5f, -1.0f,1.5f,
		-2.5f, -1.0f,1.5f,
		-1.5f, 1.0f,1.5f,

		1.5f, 1.0f,1.5f,
		2.5f, 0.0f,1.5f,
		2.5f, -1.0f,1.5f,

		-1.5f, 1.0f,1.5f,
		-2.5f, -1.0f,1.5f,
		-2.5f, 0.0f,1.5f,
		//bottom side
		1.5f, -1.0f,-1.5f,
		2.5f, -1.0f,1.5f,
		-2.5f, -1.0f,1.5f,

		-2.5f, -1.0f,1.5f,
		-1.5f, -1.0f,-1.5f,
		1.5f, -1.0f,-1.5f,
		//front side

		-1.5f,0.0f,-1.5f,
		1.5f,0.0f,-1.5f,
		1.5f,-1.0f,-1.5f,

		1.5f,-1.0f,-1.5f,
		-1.5f, -1.0f, -1.5f,
		-1.5f, 0.0f, -1.5f,
		//left side 
		-2.5f, 0.0f, 1.5f,
		-1.5f, 0.0f, -1.5f,
		-1.5f, -1.0f, -1.5f,

		-1.5f, -1.0f, -1.5f,
		-2.5f, -1.0f, 1.5f,
		-2.5f, 0.0f, 1.5f,
		//right side 
		2.5f, 0.0f, 1.5f,
		1.5f, -1.0f, -1.5f,
		1.5f, 0.0f, -1.5f,


		1.5f, -1.0f, -1.5f,
		2.5f, 0.0f, 1.5f,
		2.5f, -1.0f, 1.5f,
		//thrusters
		//left thr, left side
		-1.5f, -0.5f, 1.75,
		-1.5f, 0.5f, 1.75,
		-1.25f, 0.25f, 1.5,



		-1.25f, 0.25f, 1.5,
		-1.25f, -0.25f, 1.5,
		-1.5f, -0.5f, 1.75,
		//left thr, right side
		-0.50f, -0.5f, 1.75,
		-0.50f, 0.5f, 1.75,
		-0.75f, 0.25f, 1.5,



		-0.75f, 0.25f, 1.5,
		-0.75f, -0.25f, 1.5,
		-0.5f, -0.5f, 1.75,
		//left thr, top side
		-1.25f, 0.25f, 1.5,
		-0.75f, 0.25f, 1.5,
		-0.5f, 0.5f, 1.75,



		-0.5f, 0.5f, 1.75,
		-1.5f, 0.5f, 1.75,
		-1.25f, 0.25f, 1.5,
		//left thr, bottom side
		-1.25f, -0.25f, 1.5,
		-0.75f, -0.25f, 1.5,
		-0.5f, -0.5f, 1.75,



		-0.5f, -0.5f, 1.75,
		-1.5f, -0.5f, 1.75,
		-1.25f, -0.25f, 1.5,
		//right thr, left side
		1.5f, -0.5f, 1.75,
		1.5f, 0.5f, 1.75,
		1.25f, 0.25f, 1.5,



		1.25f, 0.25f, 1.5,
		1.25f, -0.25f, 1.5,
		1.5f, -0.5f, 1.75,
		//right thr, right side
		0.50f, -0.5f, 1.75,
		0.50f, 0.5f, 1.75,
		0.75f, 0.25f, 1.5,



		0.75f, 0.25f, 1.5,
		0.75f, -0.25f, 1.5,
		0.5f, -0.5f, 1.75,
		//right thr, top side
		1.25f, 0.25f, 1.5,
		0.75f, 0.25f, 1.5,
		0.5f, 0.5f, 1.75,



		0.5f, 0.5f, 1.75,
		1.5f, 0.5f, 1.75,
		1.25f, 0.25f, 1.5,
		//right thr, bottom side
		1.25f, -0.25f, 1.5,
		0.75f, -0.25f, 1.5,
		0.5f, -0.5f, 1.75,



		0.5f, -0.5f, 1.75,
		1.5f, -0.5f, 1.75,
		1.25f, -0.25f, 1.5


	};

		//create a vertex buffer object(vbo) to pass on our positions array to the GPU (Define vertex colours example)
	GLuint tex_example_vbo = 0;
	glGenBuffers(1, &tex_example_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, tex_example_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textureExamplePoints), textureExamplePoints, GL_STATIC_DRAW);

	//create a vertex attribute object(VAO) so that we dont have to bind each vertex buffer object every time we draw a mesh.
	//VAO remembers all the vertex buffers that you want and the memory layout of each one. 
	//Set up the VAO once per mesh and bind it before every mesh you want to draw.
	GLuint vao5 = 0;
	glGenVertexArrays(1, &vao5);
	glBindVertexArray(vao5);
	glBindBuffer(GL_ARRAY_BUFFER, tex_example_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);


	int asteroid_vertice_count = 24;
	//sixth? VAO a square to draw our ship
	GLfloat asteroidPoints[] =
	{
		//left side
		0.0f,0.0f,1.0f,
		0.0f,1.0f,0.0f,
		-1.0f,0.0f,0.0f,

		-1.0f,0.0f,0.0f,
		0.0f,1.0f,0.0f,
		0.0f,0.0f,-1.0f,

		0.0f,0.0f,-1.0f,
		-1.0f,0.0f,0.0f,
		0.0f,-1.0f,0.0f,

		0.0f,-1.0f,0.0f,
		0.0f,0.0f,1.0f,
		-1.0f,0.0f,0.0f,

		//right side
		0.0f,0.0f,1.0f,
		0.0f,1.0f,0.0f,
		1.0f,0.0f,0.0f,

		1.0f,0.0f,0.0f,
		0.0f,1.0f,0.0f,
		0.0f,0.0f,-1.0f,

		0.0f,0.0f,-1.0f,
		1.0f,0.0f,0.0f,
		0.0f,-1.0f,0.0f,

		0.0f,-1.0f,0.0f,
		0.0f,0.0f,1.0f,
		1.0f,0.0f,0.0f,





	};

	//create a vertex buffer object(vbo) to pass on our positions array to the GPU (Define vertex colours example)
	GLuint asteroid_vbo = 0;
	glGenBuffers(1, &asteroid_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, asteroid_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(asteroidPoints), asteroidPoints, GL_STATIC_DRAW);

	//create a vertex attribute object(VAO) so that we dont have to bind each vertex buffer object every time we draw a mesh.
	//VAO remembers all the vertex buffers that you want and the memory layout of each one. 
	//Set up the VAO once per mesh and bind it before every mesh you want to draw.
	GLuint vao6 = 0;
	glGenVertexArrays(1, &vao6);
	glBindVertexArray(vao6);
	glBindBuffer(GL_ARRAY_BUFFER, asteroid_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);



	//skybox shader
	//parse vertex shader from external file
	std::string skybox_vertex_source = ParseShader("skybox_shader.vert");
	const GLchar * skybox_vertex_shader = (const GLchar *)skybox_vertex_source.c_str();

	//parse fragment shader from external file
	std::string skybox_fragment_source = ParseShader("skybox_shader.frag");
	const GLchar * skybox_fragment_shader = (const GLchar *)skybox_fragment_source.c_str();

	//compile the vertex shader
	GLuint sky_vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(sky_vs, 1, &skybox_vertex_shader, NULL);
	glCompileShader(sky_vs);

	//compile the framgent shader
	GLuint sky_fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(sky_fs, 1, &skybox_fragment_shader, NULL);
	glCompileShader(sky_fs);

	//using both the compiled vertex shader and compiled framgent shader we create a single, executable GPU shader program
	GLuint skybox_program = glCreateProgram();
	glAttachShader(skybox_program, sky_fs);
	glAttachShader(skybox_program, sky_vs);
	glLinkProgram(skybox_program);



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

	//Fourth SHADER vertex colour example
	//parse vertex shader from external file
	std::string vertex_source4 = ParseShader("vertexColoursExample.vert");
	const GLchar * vertex_shader4 = (const GLchar *)vertex_source4.c_str();

	//parse fragment shader from external file
	std::string fragment_source4 = ParseShader("vertexColoursExample.frag");
	const GLchar * fragment_shader4 = (const GLchar *)fragment_source4.c_str();

	//compile the vertex shader
	GLuint vs4 = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs4, 1, &vertex_shader4, NULL);
	glCompileShader(vs4);

	//compile the fragment shader
	GLuint fs4 = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs4, 1, &fragment_shader4, NULL);
	glCompileShader(fs4);

	//using both the compiled vertex shader and compiled fragment shader we create a single, executable GPU shader program
	GLuint shader_program_VertexColourExample = glCreateProgram();
	glAttachShader(shader_program_VertexColourExample, vs4);
	glAttachShader(shader_program_VertexColourExample, fs4);
	glLinkProgram(shader_program_VertexColourExample);

	//set matrix to uniform matrix in shader
	int matrix_location = glGetUniformLocation(shader_program_VertexColourExample,"matrix");
	glUseProgram(shader_program_VertexColourExample);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, matrix);


	//Fifth SHADER ship texture
	//parse vertex shader from external file
	std::string vertex_source5 = ParseShader("ship_shader.vert");
	const GLchar * vertex_shader5 = (const GLchar *)vertex_source5.c_str();

	//parse fragment shader from external file
	std::string fragment_source5 = ParseShader("ship_shader.frag");
	const GLchar * fragment_shader5 = (const GLchar *)fragment_source5.c_str();

	//compile the vertex shader
	GLuint vs5 = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs5, 1, &vertex_shader5, NULL);
	glCompileShader(vs5);

	//compile the fragment shader
	GLuint fs5 = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs5, 1, &fragment_shader5, NULL);
	glCompileShader(fs5);

	//using both the compiled vertex shader and compiled fragment shader we create a single, executable GPU shader program
	GLuint shader_program_ship = glCreateProgram();
	glAttachShader(shader_program_ship, vs5);
	glAttachShader(shader_program_ship, fs5);
	glLinkProgram(shader_program_ship);

	//set matrix to uniform matrix in shader
	glUseProgram(shader_program_ship);
	int matrix_location2 = glGetUniformLocation(shader_program_ship, "matrix");

	glUniformMatrix4fv(matrix_location2, 1, GL_FALSE, matrix2);


	//sixth SHADER asteroid
	//parse vertex shader from external file
	std::string vertex_source6 = ParseShader("asteroid.vert");
	const GLchar * vertex_shader6 = (const GLchar *)vertex_source6.c_str();

	//parse fragment shader from external file
	std::string fragment_source6 = ParseShader("test.frag");
	const GLchar * fragment_shader6 = (const GLchar *)fragment_source6.c_str();

	//compile the vertex shader
	GLuint vs6 = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs6, 1, &vertex_shader6, NULL);
	glCompileShader(vs6);

	//compile the fragment shader
	GLuint fs6 = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs6, 1, &fragment_shader6, NULL);
	glCompileShader(fs6);

	//using both the compiled vertex shader and compiled fragment shader we create a single, executable GPU shader program
	GLuint shader_program_asteroid = glCreateProgram();
	glAttachShader(shader_program_asteroid, vs6);
	glAttachShader(shader_program_asteroid, fs6);
	glLinkProgram(shader_program_asteroid);

	//set matrix to uniform matrix in shader
	glUseProgram(shader_program_asteroid);
	int matrix_location3 = glGetUniformLocation(shader_program_asteroid, "matrix");
	glUniformMatrix4fv(matrix_location3, 1, GL_FALSE, matrix3);




	//sets clear color to grey
	glClearColor(0.5, 0.5, 0.5, 1.0);

	//sphere ray tracing collision variables
	float sphere_radius = 1.0f;

	//camera variable setup
	float cam_speed_z = 0.0f;
	float cam_speed_x = 0.0f;
	float cam_speed_y = 1.0f;
	vec3 cam_pos = { 0.0f, 0.0f, 2.0f }; //make sure that the z component is not zero otherwise it will be within the near clipping plane(assuming we draw something at the origin here)
	float cam_yaw = 0.0f;
	float cam_pitch = 0.0f;

	//test setup of our view matrix camera vectors
	vec3 view_up = vec3(0.0f, 1.0f, 0.0f);
	vec3 view_forward = vec3(0.0f, 0.0f, -1.0f);
	vec3 view_right = vec3(1.0f, 0.0f, 0.0f);

	
	//vec3 updated_view_forward = normalise(target - vec3(cam_pos[0], cam_pos[1], cam_pos[2]));
	//vec3 updated_view_right = normalise(cross(view_up, updated_view_forward));
	//vec3 updated_view_up = cross(updated_view_forward, updated_view_right);
	//vec3 updated_cam_pos = {-dot(updated_view_right,vec3(cam_pos[0], cam_pos[1], cam_pos[2])),-dot(updated_view_up,vec3(cam_pos[0], cam_pos[1], cam_pos[2])) ,-dot(updated_view_forward,vec3(cam_pos[0], cam_pos[1], cam_pos[2])) };


	//mat4 init_course_book_view_matrix2 = look_at(updated_cam_pos, target, updated_view_up );

	//testing using a versor for calculation rotation/orientation
	//some important math functions to keep in mind for versors/quaternions (from maths_funcs)
	// quaternion functions
	//versor quat_from_axis_rad(float radians, float x, float y, float z);
	//versor quat_from_axis_deg(float degrees, float x, float y, float z);
	//mat4 quat_to_mat4(const versor &q);
	//versor normalise(versor &q);

	// versor[0] = angle ,  versors[1,2,3] are the forward pointing vector components
	//im quessing since we want to keep track of where were pointing we set the forward vector with and angle of zero and calculate the matrix based on that. 

	//initializing the orientation versor for our camera

	//versor x_axis = quat_from_axis_deg(45.0f, view_up.v[0], view_up.v[1], view_up.v[2]);
	//versor y_axis = quat_from_axis_deg(45.0f, view_right.v[0], view_right.v[1], view_right.v[2]);
	//versor z_axis = quat_from_axis_deg(0.0f, view_forward.v[0], view_forward.v[1], view_forward.v[2]);

	//versor cam_orient_versor = quat_from_axis_deg(0.0f, view_forward.v[0], view_forward.v[1], view_forward.v[2]);
	//mat4 testMat = quat_to_mat4(cam_orient_versor);



	//printf("Versor");
	//print(R);




	/*what our view matrix should look like
	     --              -- 
		 | Rx  Ry  Rz -Px |    R = right poiting vector
	V =  | Ux  Uy  Uz -Py |    U = Up poiting vector
	     |-Fx -Fy -Fz -Pz |    F = front facing vector
		 |  0   0   0   1 |    P = Position of camera
         --              --   
	*/


	//view mat calcs using antons math library
	//using mat4 look_at(const vec3 &cam_pos, vec3 targ_pos, const vec3 &up);

	vec3 target_offset = {0.0f,0.0f,-10.0f};
	mat4 camera_matrix;
	//init values
	vec3 look_at_target = target_offset;
	camera_matrix = look_at(vec3(cam_pos.v[0], cam_pos.v[1], cam_pos.v[2]), look_at_target, view_up);




	//creating view matrix for camera
	//mat4 T = translate(identity_mat4(), vec3(-cam_pos.v[0], -cam_pos.v[1], -cam_pos.v[2]));
	//testing using a versor to claculate these two, so we dont need them right now
	//mat4 R = rotate_y_deg(identity_mat4(), -cam_yaw);
	//mat4 Rx = rotate_x_deg(identity_mat4(), -cam_pitch);
	//mat4 view_mat = testMat * T ;
	//mat4 view_mat = finalRot * T;
	mat4 view_mat = camera_matrix ;
	//print(view_mat);

	//print(T);
	//printf("translate matrix");
	//print(T*identity_mat4());


	//creating perspective view
	float near = 0.1f; //near clipping plane distance
	float far = 300.0f; //far clipping plane distance
	float fov = 67.0f * ONE_DEG_IN_RAD; // 67 is a good default for fov, then convert degree to rad
	float aspect = (float)vmode->width / (float)vmode->height;
		
	float range = tan(fov * 0.5f) * near;
	float Sx = (2.0f * near) / (range * aspect + range * aspect);
	float Sy = near / range;
	float Sz = -(far + near) / (far - near);
	float Pz = -(2.0f * far * near) / (far - near);
	//create the perspective matrix
	float proj_mat[] = { Sx, 0.0f, 0.0f, 0.0f, 0.0f, Sy, 0.0f, 0.0f, 0.0f, 0.0f, Sz, -1.0f, 0.0f, 0.0f, Pz, 0.0f };
	mat4 proj_mat_ray = mat4( Sx, 0.0f, 0.0f,  0.0f, 0.0f,   Sy, 0.0f,  0.0f, 0.0f, 0.0f,   Sz, -1.0f, 0.0f, 0.0f,   Pz,  0.0f);


	//loading in a texture
	int img_x, img_y, img_n;
	int force_channels = 4;
	unsigned char* image_data = stbi_load(RELPATH"spaceship_texture_map.jpg",&img_x, &img_y, &img_n,force_channels);
	if (!image_data)
	{
		printf("Error:image not found!!!");
	}

	//swapping the image right side up
	int width_in_bytes = img_x * 4;
	unsigned char *top = NULL;
	unsigned char *bottom = NULL;
	unsigned char temp = 0;
	int half_height = img_y / 2;

	for (int row = 0; row < half_height;row++)
	{
		top = image_data + row * width_in_bytes;
		bottom = image_data + (img_y - row - 1) * width_in_bytes;
		for (int col = 0; col < width_in_bytes;col++)
		{
			temp = *top;
			*top = *bottom;
			*bottom = temp;
			top++;
			bottom++;
		}
	}

	//copying image data into OpenGL texture
	//***note we are aleardy using gl_texture0 for our skybox so we use gl_texture1 here i believe***
	//we need to manually swap our active textures in and out
	GLuint tex = 0;
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, tex);
	//specify a two-dimensional texture image
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,img_x,img_y,0,GL_RGBA,GL_UNSIGNED_BYTE,image_data);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


	//set our texture uniform in the ship shader
	int ship_tex_loc = glGetUniformLocation(shader_program_ship, "basic_texture");
	glUseProgram(shader_program_ship);
	glUniform1i(ship_tex_loc,1);




	//create texture coordinates

	GLfloat texcoords[] = {
		//top side
		//3 squares
		0.0f,1.0f,
		0.0f,0.0f,
		0.333f,0.0f,

		0.333f,0.0f,
		0.333f,1.0f,
		0.0f,1.0f,

		0.0f,1.0f,
		0.0f,0.0f,
		0.333f,0.0f,

		0.333f,0.0f,
		0.333f,1.0f,
		0.0f,1.0f,

		0.0f,1.0f,
		0.0f,0.0f,
		0.333f,0.0f,

		0.333f,0.0f,
		0.333f,1.0f,
		0.0f,1.0f,
		//3 rectangles (front window)
		0.0f,1.0f,
		0.0f,0.0f,
		0.333f,0.0f,

		0.333f,0.0f,
		0.333f,1.0f,
		0.0f,1.0f,

		0.0f,1.0f,
		0.0f,0.0f,
		0.333f,0.0f,

		0.333f,0.0f,
		0.333f,1.0f,
		0.0f,1.0f,

		0.7f,1.0f,
		0.7f,0.0f,
		1.0f,0.0f,

		1.0f,0.0f,
		1.0f,1.0f,
		0.7f,1.0f,
		//top long side
		0.0f,0.5f,
		0.0f,0.0f,
		0.333f,0.0f,

		0.333f,0.0f,
		0.333f,1.0f,
		0.0f,1.0f,
		//top side fillers
		0.0f,1.0f,
		0.0f,0.0f,
		0.333f,0.0f,

		0.333f,0.0f,
		0.333f,1.0f,
		0.0f,1.0f,
		//back side
		0.0f,1.0f,
		0.0f,0.0f,
		0.333f,0.0f,

		0.333f,0.0f,
		0.333f,1.0f,
		0.0f,1.0f,

		0.0f,1.0f,
		0.0f,0.0f,
		0.333f,0.0f,

		0.333f,0.0f,
		0.333f,1.0f,
		0.0f,1.0f,
		//bottom side
		0.0f,1.0f,
		0.0f,0.0f,
		0.333f,0.0f,

		0.333f,0.0f,
		0.333f,1.0f,
		0.0f,1.0f,
		//front
		0.0f,1.0f,
		0.0f,0.0f,
		0.333f,0.0f,

		0.333f,0.0f,
		0.333f,1.0f,
		0.0f,1.0f,
		//left side
		0.0f,1.0f,
		0.0f,0.0f,
		0.333f,0.0f,

		0.333f,0.0f,
		0.333f,1.0f,
		0.0f,1.0f,
		//right side
		0.0f, 1.0f,
		0.0f, 0.0f,
		0.333f, 0.0f,

		0.333f, 0.0f,
		0.333f, 1.0f,
		0.0f, 1.0f
		//thrusters
	};

	GLuint vt_vbo;
	glGenBuffers(1,&vt_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords), texcoords, GL_STATIC_DRAW);

	//update the ship vao
	glBindVertexArray(vao5);
	glBindBuffer(GL_ARRAY_BUFFER, vt_vbo);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);




	//intialize the values of our projection and view matrices in the ship vertex shader
	int ship_view_matrix_location = glGetUniformLocation(shader_program_ship, "view");
	glUseProgram(shader_program_ship);
	glUniformMatrix4fv(ship_view_matrix_location, 1, GL_FALSE, view_mat.m);
	int ship_projection_matrix_location = glGetUniformLocation(shader_program_ship, "proj");
	glUseProgram(shader_program_ship);
	glUniformMatrix4fv(ship_projection_matrix_location, 1, GL_FALSE, proj_mat);

	//intialize the values of our projection and view matrices in the asteroid vertex shader
	int asteroid_view_matrix_location = glGetUniformLocation(shader_program_asteroid, "view");
	glUseProgram(shader_program_asteroid);
	glUniformMatrix4fv(asteroid_view_matrix_location, 1, GL_FALSE, view_mat.m);
	int asteroid_projection_matrix_location = glGetUniformLocation(shader_program_asteroid, "proj");
	glUseProgram(shader_program_asteroid);
	glUniformMatrix4fv(asteroid_projection_matrix_location, 1, GL_FALSE, proj_mat);






	//intialize the values of our projection and view matrices in the entity vertex shader
	int view_matrix_location = glGetUniformLocation(shader_program_VertexColourExample, "view");
	glUseProgram(shader_program_VertexColourExample);
	glUniformMatrix4fv(view_matrix_location, 1, GL_FALSE, view_mat.m);
	int projection_matrix_location = glGetUniformLocation(shader_program_VertexColourExample, "proj");
	glUseProgram(shader_program_VertexColourExample);
	glUniformMatrix4fv(projection_matrix_location, 1, GL_FALSE, proj_mat);

	//intialize the values of our projection and view matrices in the skybox vertex shader
	int skybox_view_matrix_location = glGetUniformLocation(skybox_program, "view");
	glUseProgram(skybox_program);
	glUniformMatrix4fv(skybox_view_matrix_location, 1, GL_FALSE, view_mat.m);
	int skybox_projection_matrix_location = glGetUniformLocation(skybox_program, "proj");
	glUseProgram(skybox_program);
	glUniformMatrix4fv(skybox_projection_matrix_location, 1, GL_FALSE, proj_mat);
	

	float speed = 1.0f; //move at 1 unit per second
	float rotation_speed = 1.0f; //rotate at 1 degree per second
	float last_position = 0.0f;
	vec3 newForward = {0.0f,0.0f,-1.0f};

	//mouse x and y positions
	double mouseX, mouseY;
	double mouseXDisplacement = 0;
	double mouseYDisplacement = 0;

	bool is_firing = false;
	int player_money = 0;
	int wave_number = 1;
	double boost_activation_time = 0.0;
	double boost_duration_time = 0.0;
	bool has_turning_upgrade = false;
	bool is_speed_boost_active = false;

	GLuint skybox = 0;

	create_cube_map(RELPATH"bkg1_back6.png", RELPATH"bkg1_front5.png", RELPATH"bkg1_top3.png", RELPATH"bkg1_bottom4.png", RELPATH"bkg1_left2.png", RELPATH"bkg1_right1.png",&skybox);

	int tex_loc = glGetUniformLocation(skybox_program, "cube_texture");

	glfwSetKeyCallback(window, key_callback);

	int enemiesLeft = 10;
	enemy_t enemies[10];
	for (int i = 0; i < 10; i++)
	{
		initEnemy(enemies[i]);
	}

	//#define STARTMENU 0
	//#define GAMEPLAY  1
	//#define SHOP      2

	//drawing loop
	while (!glfwWindowShouldClose(window))
	{
		switch (gamestate)
		{
			case STARTMENU:
			{
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glfwPollEvents();
				glfwSwapBuffers(window);
				break;
			}
			case GAMEPLAY:
			{
				//resets mouse position to center of screen and checks for mouse movement
				if (mouseXDisplacement != 0.0 || mouseYDisplacement != 0.0)
				{
					glfwSetCursorPos(window, vmode->width / 2, vmode->height / 2);
				}


				glfwGetCursorPos(window, &mouseX, &mouseY);

				mouseXDisplacement = mouseX - (double)(vmode->width / 2);
				mouseYDisplacement = mouseY - (double)(vmode->height / 2);


				//create escape key
				if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
					glfwSetWindowShouldClose(window, 1);
				}


				//math test section lol
				//bug case looking at (-1,0,0) or (1,0,0)



				//timer for animation (this is probably how we should always do it)
				static double previous_seconds = glfwGetTime(); //why is it static here? does it refer to this (static storage duration. The storage for the object is allocated when the program begins and deallocated when the program ends. Only one instance of the object exists. All objects declared at namespace scope (including global namespace) have this storage duration, plus those declared with static or extern.)
				double current_seconds = glfwGetTime();
				double elapsed_seconds = current_seconds - previous_seconds;
				previous_seconds = current_seconds;

				//reverse direction when going too far left or right
				if (fabs(last_position) > 1.0f)
				{
					speed = -speed;
				}



				//code for moving camera, we use a bool here and change the view matrix after checking all input in case we press multiple buttons

				bool cam_moved = false;
				if (glfwGetKey(window, GLFW_KEY_F))
				{
					if (!is_speed_boost_active)
					{
						is_speed_boost_active = true;
						boost_activation_time = glfwGetTime();
						boost_duration_time = boost_activation_time + 5.0;

					}
				}

				if (is_speed_boost_active)
				{
					boost_activation_time = glfwGetTime();
					if (boost_activation_time > boost_duration_time)
					{
						is_speed_boost_active = false;
					}
				}

				if (glfwGetKey(window, GLFW_KEY_A))
				{
					if (is_speed_boost_active)
					{
						cam_speed_x -= 0.21;
						cam_moved = true;
					}
					else
					{ 
					cam_speed_x -= 0.07;
					cam_moved = true;
					}
				}
				if (glfwGetKey(window, GLFW_KEY_D))
				{
					if (is_speed_boost_active)
					{
						cam_speed_x += 0.21;
						cam_moved = true;
					}
					else
					{
						cam_speed_x += 0.07;
						cam_moved = true;
					}
				}
				if (glfwGetKey(window, GLFW_KEY_Q))
				{
					if (is_speed_boost_active)
					{
						cam_pos += view_up * cam_speed_y * elapsed_seconds * 3;
						cam_moved = true;
					}
					else
					{
						cam_pos += view_up * cam_speed_y * elapsed_seconds;
						cam_moved = true;
					}
				}
				if (glfwGetKey(window, GLFW_KEY_E))
				{
					if (is_speed_boost_active)
					{
						cam_pos -= view_up * cam_speed_y * elapsed_seconds * 3;
						cam_moved = true;
					}
					else
					{
						cam_pos -= view_up * cam_speed_y * elapsed_seconds;
						cam_moved = true;
					}
				}
				if (glfwGetKey(window, GLFW_KEY_W))
				{
					if (is_speed_boost_active)
					{
						cam_speed_z += 0.3;
						cam_moved = true;
					}
					else
					{ 
					cam_speed_z += 0.1;
					cam_moved = true;
					}
				}
				if (glfwGetKey(window, GLFW_KEY_S))
				{

					if (is_speed_boost_active)
					{
						cam_speed_z -= 0.3;
						cam_moved = true;
					}
					else
					{
						cam_speed_z -= 0.1;
						cam_moved = true;
					}
				}
				if (glfwGetKey(window, GLFW_KEY_X))
				{
					if (cam_speed_z > 1.0f)
					{
						cam_speed_z -= 0.2;
					}
					else if (cam_speed_z < -1.0f)
					{
						cam_speed_z += 0.2;
					}
					else
					{
						cam_speed_z = 0.0f;
					}

					if (cam_speed_x > 1.0f)
					{
						cam_speed_x -= 0.2;
					}
					else if (cam_speed_x < -1.0f)
					{
						cam_speed_x += 0.2;
					}
					else
					{
						cam_speed_x = 0.0f;
					}
				}

				/*
				if (glfwGetKey(window, GLFW_KEY_LEFT))
				{
				cam_yaw += cam_yaw_speed * elapsed_seconds;
				cam_moved = true;
				}
				if (glfwGetKey(window, GLFW_KEY_RIGHT))
				{
				cam_yaw -= cam_yaw_speed * elapsed_seconds;
				cam_moved = true;
				}
				*/


				//calculate the total speed in z and x axis to limit ship rotation speed 
				float total_speed = (abs(cam_speed_x)*1.5) + abs(cam_speed_z);

				if (total_speed < 0.1f)
				{
					total_speed = 5;
				}
				else
				{
					total_speed = 30 / total_speed;
				}

				//printf("%f\n", total_speed);

				if (mouseXDisplacement > 0.0)
				{
					if (mouseXDisplacement > total_speed)
					{
						mouseXDisplacement = total_speed;
					}
					cam_yaw -= mouseXDisplacement;
					if (cam_yaw < 0.0f)
					{
						cam_yaw = 360.0f;
					}

					cam_moved = true;
				}

				if (mouseXDisplacement < 0.0)
				{
					if (mouseXDisplacement < -total_speed)
					{
						mouseXDisplacement = -total_speed;
					}
					cam_yaw -= mouseXDisplacement;
					if (cam_yaw > 360.0f)
					{
						cam_yaw = 0.0f;
					}
					cam_moved = true;
				}

				if (mouseYDisplacement > 0.0)
				{
					if (mouseYDisplacement > total_speed)
					{
						mouseYDisplacement = total_speed;
					}
					cam_pitch -= mouseYDisplacement;

					if (cam_pitch < -89.0f)
					{
						cam_pitch = -89.0f;
					}
					cam_moved = true;



				}

				if (mouseYDisplacement < 0.0)
				{
					if (mouseYDisplacement < -total_speed)
					{
						mouseYDisplacement = -total_speed;
					}

					cam_pitch -= mouseYDisplacement;
					if (cam_pitch > 89.0f)
					{
						cam_pitch = 89.0f;
					}
					cam_moved = true;

				}

				//ray casting (dont need commented section since we are always facing forward direction)
				if (glfwGetKey(window, GLFW_KEY_SPACE))
				{
					is_firing = true;


					/*
					float ray_mouse_x = (2.0f * mouseX) / vmode->width - 1.0f;
					float ray_mouse_y = 1.0f - (2.0f * mouseY) / vmode->height;
					float ray_mouse_z = 1.0f;
					vec3 ray_nds = vec3(ray_mouse_x, ray_mouse_y, ray_mouse_z);
					vec4 ray_clip = vec4(ray_nds.v[0], ray_nds.v[1], -1.0f, 1.0f);
					vec4 ray_eye = inverse(proj_mat_ray) * ray_clip;
					ray_eye = vec4(ray_eye.v[0], ray_eye.v[1], -1.0, 0.0);
					vec4 temp = (inverse(view_mat) * ray_eye);
					vec3 ray_wor = vec3(temp.v[0], temp.v[1], temp.v[2]);
					ray_wor = normalise(ray_wor);
					printf("RAY: ");
					print(ray_wor);
					---------------------------------------------------*/

					//to solve ray trace collision we need the a,b,c parameters to pass into our solveQuadratic function
					//see https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection for an explanation


					float t0, t1;
					/*
					vec3 L = cam_pos - vec3(enemy_ship_x, enemy_ship_y, enemy_ship_z);
					float a = dot(view_forward, view_forward);
					float b = 2 * dot(view_forward, L);
					float c = dot(L, L) - pow(sphere_radius, 2);
					bool test_collision = solveQuadratic(a, b, c, t0, t1);
					if (test_collision)
					{
						printf("Hit\n");
					}
					else
					{
						printf("Miss\n");
					}
					*/

					for (int i = 0; i < enemiesLeft; i++)
					{
						vec3 L = cam_pos - vec3(enemies[i].x, enemies[i].y, enemies[i].z);
						float a = dot(view_forward, view_forward);
						float b = 2 * dot(view_forward, L);
						float c = dot(L, L) - pow(sphere_radius, 2);
						bool test_collision = solveQuadratic(a, b, c, t0, t1);
						if (test_collision)
						{
							//printf("Hit\n");
							enemies[i].hp -= 5;
							if (enemies[i].hp <= 0)
							{
								player_money += 5;
								printf("Enemy destroyed! %d enemies left!\n", enemiesLeft - 1);
								printf("You gained 5$! You have:%d$\n", player_money);
								for (int n = i; n < enemiesLeft-1; n++)
								{
									enemies[n] = enemies[n + 1];
								}
								enemiesLeft--;

								
							//enemies[i] = enemies[enemiesLeft - i];
							//enemiesLeft--;
							}
						}
					}

				}


				//we update our x and z position based on speed
				cam_pos += view_forward * cam_speed_z * elapsed_seconds;
				cam_pos += view_right * cam_speed_x * elapsed_seconds;

				//check to see if wave cleared
				
				
				//we update/recalculate our view matrix if one of the previous keys were pressed
				//if (cam_moved)
				if (1)
				{





					//system("CLS");

					//section to test course book camera
					//default offset {0,0,-2} -> 2 units forward
					//rotate_vector_by_quaternion(vec3& v, versor& q, vec3& vprime)

					vec3 actual_offset = target_offset;
					vec3 actual_offset_result;
					vec3 actual_offset_final;

					//actual_offset = normalise(actual_offset);

					//versor quat_yaw = quat_from_axis_deg(cam_yaw, view_up.v[0], view_up.v[1], view_up.v[2]);
					versor quat_yaw = quat_from_axis_deg(cam_yaw, 0.0, 1.0, 0.0);
					quat_yaw = normalise(quat_yaw);

					//rotate_vector_by_quaternion(actual_offset, quat_yaw,actual_offset_result);
					//printf("result of first quaternion applied to offset: x:%f y:%f z:%f angle used:%f\n", actual_offset_result.v[0], actual_offset_result.v[1], actual_offset_result.v[2], cam_yaw);

					//	printf("yaw angle:%f\n",cam_yaw);
					//printf("pitch angle:%f\n", cam_pitch);
					//printf("final offset direction: x:%f y:%f z:%f\n", actual_offset_result.v[0], actual_offset_result.v[1], actual_offset_result.v[2]);

					//	view_forward = actual_offset_result;
					//	view_forward = normalise(view_forward);
					//	view_right = cross(view_forward,vec3(0.0f,1.0f,0.0f));
					//	view_right = normalise(view_right);

					//printf("before forward vector:");
					//	print(view_forward);
					//	printf("before right vector:");
					//print(view_right);


					//versor quat_pitch = quat_from_axis_deg(cam_pitch, view_right.v[0], view_right.v[1], view_right.v[2]);
					versor quat_pitch = quat_from_axis_deg(cam_pitch, 1.0, 0.0, 0.0);
					quat_pitch = normalise(quat_pitch);




					versor result_quat = quat_yaw * quat_pitch;

					//print(result_quat);

					rotate_vector_by_quaternion(actual_offset, result_quat, actual_offset_final);
					actual_offset_final = normalise(actual_offset_final);
					//printf("test forward vec\n");
					//print(actual_offset_final);

					//rotate_vector_by_quaternion(actual_offset_result, quat_pitch, actual_offset_final);

					//printf("result of second quaternion applied to offset: x:%f y:%f z:%f angle used:%f\n", actual_offset_final.v[0], actual_offset_final.v[1], actual_offset_final.v[2], cam_pitch);

					view_forward = actual_offset_final;

					view_forward = normalise(view_forward);

					view_right = cross(view_forward, vec3(0.0f, 1.0f, 0.0f));

					view_right = normalise(view_right);

					//vec3 test_up = cross(view_right, view_forward);
					//test_up = normalise(test_up);
					//printf("test up vector:");
					//print(test_up);

					//	printf("forward vector:");
					//	print(view_forward);
					//	printf("right vector:");
					//	print(view_right);

					view_up = cross(view_right, view_forward);
					view_up = normalise(view_up);


					//printf("forward vector:");
					//print(view_forward);
					//printf("right vector:");
					//print(view_right);
				//	printf("up vector:");
				//	print(view_up);
				//	printf("-----------------------------------------------------------------\n");

					//vec3 look_at_target = actual_offset_result + cam_pos;
					vec3 look_at_target = actual_offset_final + cam_pos;
				//	printf("final offset value:");
				//	print(actual_offset_final);

					camera_matrix = look_at(vec3(cam_pos.v[0], cam_pos.v[1], cam_pos.v[2]), look_at_target, view_up);
					//camera_matrix = look_at(vec3(cam_pos.v[0], cam_pos.v[1], cam_pos.v[2]), look_at_target, vec3(0.0,1.0,0.0));

					//initializing the orientation versor for our camera
					//lets try the cross product of our two vectors in one versor
					//vec3 testVector = cross()

					//


					//mat4 R = rotate_y_deg(identity_mat4(), -cam_yaw);
					//mat4 Rx = rotate_x_deg(identity_mat4(), -cam_pitch);
					//mat4 view_mat = R  * T * init_view_matrix;
					//mat4 view_mat = testMat * T;
					//mat4 view_mat = finalRot * T;
					mat4 view_mat = camera_matrix;
					//print(view_mat);


					//pass in updated values to entity vertex shader
					glUseProgram(shader_program_VertexColourExample);
					glUniformMatrix4fv(view_matrix_location, 1, GL_FALSE, view_mat.m);

					//pass in updated values to ship vertex shader
					glUseProgram(shader_program_ship);
					glUniformMatrix4fv(ship_view_matrix_location, 1, GL_FALSE, view_mat.m);

					//pass in updated values to asteroid vertex shader
					glUseProgram(shader_program_asteroid);
					glUniformMatrix4fv(asteroid_view_matrix_location, 1, GL_FALSE, view_mat.m);

					//pass in updated values to skybox vertex shader
					glUseProgram(skybox_program);
					glUniformMatrix4fv(skybox_view_matrix_location, 1, GL_FALSE, view_mat.m);


				}

				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				
				glDepthMask(GL_FALSE);
				glUseProgram(skybox_program);
				glUniform1i(tex_loc, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
				glBindVertexArray(vao_sky);
				glDrawArrays(GL_TRIANGLES, 0, 36);
				glDepthMask(GL_TRUE);



				//updates the x component of the matrix in this case the 12th array position
				//matrix[12] = elapsed_seconds * speed + last_position;
				//last_position = matrix[12];
				//rotation around x axis corresponds to 5,6,9 and 10 like this
				//matrix[5] = cos(last_position* 180/3.14);
				//matrix[6] = sin(last_position * 180 / 3.14);
				//matrix[9] = -sin(last_position * 180 / 3.14);
				//matrix[10] = cos(last_position * 180 / 3.14);


				glUseProgram(shader_program_VertexColourExample);

				glUniformMatrix4fv(matrix_location, 1, GL_FALSE, matrix);

				//wipe the drawing surface clear	


				//glUseProgram(shader_program_purple);
				//glBindVertexArray(vao);
				////draw points 0-3 from the currently bound VAO with current in-use shader
				//glDrawArrays(GL_TRIANGLES, 0, 6);

				//glUseProgram(shader_program_red);
				//glBindVertexArray(vao2);
				////draw points 0-3 from the currently bound VAO with current in-use shader
				//glDrawArrays(GL_TRIANGLES, 0, 3);

				//glUseProgram(shader_program_blue);
				//glBindVertexArray(vao3);
				////draw points 0-3 from the currently bound VAO with current in-use shader
				//glDrawArrays(GL_TRIANGLES, 0, 3);

			//	glUseProgram(shader_program_VertexColourExample);
				//glBindVertexArray(vao4);
				//glDrawArrays(GL_TRIANGLES, 0, 12);

				//matrix2[12] = enemy_ship_x;
				//matrix2[13] = enemy_ship_y;
				//matrix2[14] = enemy_ship_z;

				glUseProgram(shader_program_ship);
				glUniformMatrix4fv(matrix_location2, 1, GL_FALSE, matrix2);
				glBindVertexArray(vao5);
				//glDrawArrays(GL_TRIANGLES, 0, 132);

				for (int i = 0; i < enemiesLeft; i++)
				{
					int movement = enemies[i].behaviour - 1;

					enemies[i].x += movement * (-cam_pos.v[0] / 500);
					enemies[i].y += movement * (-cam_pos.v[1] / 500);
					enemies[i].z += movement * (-cam_pos.v[2] / 500);

					matrix2[12] = enemies[i].x;
					matrix2[13] = enemies[i].y;
					matrix2[14] = enemies[i].z;

					glUniformMatrix4fv(matrix_location2, 1, GL_FALSE, matrix2);
					glBindVertexArray(vao5);
					glDrawArrays(GL_TRIANGLES, 0, 132);
				}


				//draw asteroids here
				glUseProgram(shader_program_asteroid);
				glUniformMatrix4fv(matrix_location3, 1, GL_FALSE, matrix3);
				glBindVertexArray(vao6);
				glDrawArrays(GL_TRIANGLES, 0, asteroid_vertice_count);



				if (is_firing)
				{
					glUseProgram(shader_program_red);
					glBindVertexArray(vao3);
					glDrawArrays(GL_TRIANGLES, 0, 3);
					is_firing = false;
				}

				if (enemiesLeft == 0)
				{
					wave_number++;
					cam_pos = { 0.0f, 0.0f, 2.0f };
					gamestate = SHOP;
				}

				//update other events like the input handling
				glfwPollEvents();
				//put the stuff we've been drawing onto the display
				glfwSwapBuffers(window);
				break;
			}
			case SHOP:
			{
				system("cls");
				printf("Wave cleared! You are now on wave %d.\n", wave_number);
				printf("Welcome to the shop!\n");
				printf("Press q at any time to start the next wave!\n");
				if(!has_turning_upgrade)
				{ 
				printf("\nPress 1 to buy speed power up! Cost: 30$\n");
				}
				else
				{
					printf("To use the turning upgrade press f while in combat to activate the ability. Be careful it only lasts a few seconds!\n");
				}
				//create escape key
				if (GLFW_PRESS == glfwGetKey(window, GLFW_KEY_ESCAPE)) {
					glfwSetWindowShouldClose(window, 1);
				}
				if (glfwGetKey(window, GLFW_KEY_1))
				{
					if (!has_turning_upgrade)
					{
						if (player_money < 30)
						{
							printf("You dont have enough for this upgrade!\n");
						}
						else
						{
							player_money -= 30;
							has_turning_upgrade = true;
						}
					}
					
				}
				if (glfwGetKey(window, GLFW_KEY_Q))
				{
					
					for (int i = 0; i < 10; i++)
					{
						initEnemy(enemies[i]);
					}
					enemiesLeft = 10;
					gamestate = GAMEPLAY;
					system("cls");
				}

				//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glfwPollEvents();
			//	glfwSwapBuffers(window);
				break;
			}
		}	
	}

	glDeleteProgram(shader_program_purple);
	glDeleteProgram(shader_program_red);
	glDeleteProgram(shader_program_blue);

	glfwTerminate();
	return 0;
}