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
#include "maths_funcs.h"
#define GL_LOG_FILE "gl.log"
#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
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
	//make it fullscreen
	GLFWmonitor* mon = glfwGetPrimaryMonitor();
	const GLFWvidmode* vmode = glfwGetVideoMode(mon);
	
	GLFWwindow* window = glfwCreateWindow(vmode->width, vmode->height, "Hello Triangle", mon, NULL);
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

	//enable vsync
	glfwSwapInterval(1);


	//set the cursor mode to GLFW_CURSOR_DISABLED. (GLFW will then take care of all the details of cursor re-centering and offset calculation and providing the application with a virtual cursor position)
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//keep in mind which winding order you are making new shapes in for culling.(note this setting is global, you can turn it off in between draw calls to make some items two sided)
	glEnable(GL_CULL_FACE); //cull face
	glCullFace(GL_BACK); //cull face back
	glFrontFace(GL_CW); //GL_CW for clock-wise ... GL_CCW for counter clock-wise


	//note on order of operations for translations and rotations when using column major matrices(what we use)
	// V' = T * R * V -> rotate, then translate
	// V' = R * T * V -> rotate around a point(orbit?)
	// V' = V * T * R -> INVALID!!
	//in short, order matters!


	//matrix for translation demo             //when using a 1d array as a 4x4 matrix this is the order of elements
	float matrix[] = 
	{           
		1.0f, 0.0f, 0.0f, 0.0f,									//0, 4,  8, 12,
		0.0f, 1.0f, 0.0f, 0.0f,									//1, 5,  9, 13,
		0.0f, 0.0f, 1.0f, 0.0f,									//2, 6, 10, 14,
		0.0f, 0.0f, 0.0f, 1.0f									//3, 7, 11, 15,
	};





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
		0.0f,1.0f,0.0f,
		0.0f,0.0f,1.0f,
		

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

	//sets clear color to grey
	glClearColor(0.5, 0.5, 0.5, 1.0);

	//camera variable setup
	float cam_speed = 5.0f;
	float cam_yaw_speed = 50.0f;
	float cam_pitch_speed = 50.0f;
	float cam_pos[] = { 0.0f, 0.0f, 2.0f }; //make sure that the z component is not zero otherwise it will be within the near clipping plane(assuming we draw something at the origin here)
	float cam_yaw = 0.0f;
	float cam_pitch = 0.0f;


	//test setup of our view matrix camera vectors
	vec3 view_up = vec3(0.0f, 1.0f, 0.0f);
	vec3 view_forward = vec3(0.0f, 0.0f, -1.0f);
	vec3 view_right = vec3(1.0f, 0.0f, 0.0f);


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

	versor x_axis = quat_from_axis_deg(45.0f, view_up.v[0], view_up.v[1], view_up.v[2]);
	versor y_axis = quat_from_axis_deg(45.0f, view_right.v[0], view_right.v[1], view_right.v[2]);
	versor z_axis = quat_from_axis_deg(0.0f, view_forward.v[0], view_forward.v[1], view_forward.v[2]);

	versor cam_orient_versor = quat_from_axis_deg(0.0f, view_forward.v[0], view_forward.v[1], view_forward.v[2]);
	mat4 testMat = quat_to_mat4(cam_orient_versor);

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


	//hard coded view matrix for testing
	mat4 test_view_mat = 
	{
		1.0f, 0.0f, 0.0f,  0.0f,
		0.0f, 1.0f, 0.0f,  0.0f,
		0.0f, 0.0f,  1.0f,  0.0f,
		0.0f, 0.0f,  -2.0f,  1.0f
	};

	

	mat4 init_view_matrix = mat4(
		view_right.v[0], view_up.v[0], -view_forward.v[0], 0.0f,
		view_right.v[1], view_up.v[1], -view_forward.v[1], 0.0f,
		view_right.v[2], view_up.v[2], -view_forward.v[2], 0.0f,
		           0.0f,         0.0f,               0.0f, 1.0f
	);



	//creating view matrix for camera
	mat4 T = translate(identity_mat4(), vec3(-cam_pos[0], -cam_pos[1], -cam_pos[2]));
	//testing using a versor to claculate these two, so we dont need them right now
	//mat4 R = rotate_y_deg(identity_mat4(), -cam_yaw);
	//mat4 Rx = rotate_x_deg(identity_mat4(), -cam_pitch);
	mat4 view_mat = testMat * T;


	//print(T);
	//printf("translate matrix");
	//print(T*identity_mat4());


	//creating perspective view
	float near = 0.1f; //near clipping plane distance
	float far = 100.0f; //far clipping plane distance
	float fov = 67.0f * ONE_DEG_IN_RAD; // 67 is a good default for fov, then convert degree to rad
	float aspect = (float)vmode->width / (float)vmode->height;
		
	float range = tan(fov * 0.5f) * near;
	float Sx = (2.0f * near) / (range * aspect + range * aspect);
	float Sy = near / range;
	float Sz = -(far + near) / (far - near);
	float Pz = -(2.0f * far * near) / (far - near);
	//create the perspective matrix
	float proj_mat[] = 
	{
		  Sx, 0.0f, 0.0f,  0.0f,
		0.0f,   Sy, 0.0f,  0.0f,
		0.0f, 0.0f,   Sz, -1.0f,
		0.0f, 0.0f,   Pz,  0.0f
	};




	//intialize the values of our projection and view matrices in the vertex shader
	int view_matrix_location = glGetUniformLocation(shader_program_VertexColourExample, "view");
	glUseProgram(shader_program_VertexColourExample);
	glUniformMatrix4fv(view_matrix_location, 1, GL_FALSE, view_mat.m);
	int projection_matrix_location = glGetUniformLocation(shader_program_VertexColourExample, "proj");
	glUseProgram(shader_program_VertexColourExample);
	glUniformMatrix4fv(projection_matrix_location, 1, GL_FALSE, proj_mat);
	

	float speed = 1.0f; //move at 1 unit per second
	float rotation_speed = 1.0f; //rotate at 1 degree per second
	float last_position = 0.0f;
	vec3 newForward = {0.0f,0.0f,-1.0f};

	//mouse x and y positions
	double mouseX, mouseY;
	double mouseXDisplacement = 0;
	double mouseYDisplacement = 0;


	//drawing loop
	while (!glfwWindowShouldClose(window))
	{
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
		if (glfwGetKey(window, GLFW_KEY_A))
		{
			cam_pos[0] -= cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if (glfwGetKey(window, GLFW_KEY_D))
		{
			cam_pos[0] += cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if (glfwGetKey(window, GLFW_KEY_PAGE_UP))
		{
			cam_pos[1] += cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN))
		{
			cam_pos[1] -= cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if (glfwGetKey(window, GLFW_KEY_W))
		{
			print(view_forward);
			//cam_pos[2] -= cam_speed * elapsed_seconds;
			cam_pos[0] -= view_forward.v[0] * cam_speed * elapsed_seconds; 
			cam_pos[1] -= view_forward.v[1] * cam_speed * elapsed_seconds;
			cam_pos[2] += view_forward.v[2] * cam_speed * elapsed_seconds;
			printf("New componenets. X:%f,Y:%f,Z:%f\n",cam_pos[0], cam_pos[1], cam_pos[2]);
			cam_moved = true;
		}
		if (glfwGetKey(window, GLFW_KEY_S))
		{
			cam_pos[2] += cam_speed * elapsed_seconds;
			cam_moved = true;
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

		if (mouseXDisplacement > 0.0)
		{
			cam_yaw += mouseXDisplacement;
			cam_moved = true;
		}

		if (mouseXDisplacement < 0.0)
		{
			cam_yaw += mouseXDisplacement;
			cam_moved = true;
		}

		if (mouseYDisplacement > 0.0)
		{
			cam_pitch += mouseYDisplacement;
			cam_moved = true;
		}

		if (mouseYDisplacement < 0.0)
		{
			cam_pitch += mouseYDisplacement;
			cam_moved = true;
		}
		//we update/recalculate our view matrix if one of the previous keys were pressed
		if (cam_moved)
		{

			//initializing the orientation versor for our camera
			//lets try the cross product of our two vectors in one versor
			//vec3 testVector = cross()

			//
			versor cam_orient_versorX = quat_from_axis_deg(cam_yaw, view_up.v[0], view_up.v[1], view_up.v[2]);


			versor cam_orient_versorY = quat_from_axis_deg(cam_pitch, view_right.v[0], view_right.v[1], view_right.v[2]);

			versor finalVersor = cam_orient_versorX * cam_orient_versorY;

			mat4 testMat = quat_to_mat4(finalVersor);

			mat4 init_view_matrix = mat4(
				view_right.v[0], view_up.v[0], -view_forward.v[0], 0.0f,
				view_right.v[1], view_up.v[1], -view_forward.v[1], 0.0f,
				view_right.v[2], view_up.v[2], -view_forward.v[2], 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f

				
			);

			

			mat4 T = translate(identity_mat4(), vec3(-cam_pos[0], -cam_pos[1], -cam_pos[2]));
			
			//mat4 R = rotate_y_deg(identity_mat4(), -cam_yaw);
			//mat4 Rx = rotate_x_deg(identity_mat4(), -cam_pitch);
			//mat4 view_mat = R  * T * init_view_matrix;
			mat4 view_mat = testMat * T;

			view_forward = { -view_mat.m[8],-view_mat.m[9],-view_mat.m[10] };
			view_forward = normalise(view_forward);

			//view_right = { view_mat.m[0],view_mat.m[1],0.0f };
			//view_right = normalise(view_right);

			//view_up = cross(view_forward, view_right);
			//view_up = normalise(view_up);

			//view_up = { view_mat.m[4],view_mat.m[5],view_mat.m[6] };
			//view_up = normalise(view_up);

			

			printf("Right vector:");
			print(view_right);
			printf("Up vector:");
			print(view_up);
			printf("forward vector:");
			print(view_forward);
			printf("\n\n");

			//view_right = { view_mat.m[0],view_mat.m[1],view_mat.m[2] };
			//view_right = normalise(view_right);
			//attempt to calulate new forward
			//vec3 newUp = {view_mat.m[4],view_mat.m[5],view_mat.m[6] };
			//newUp = normalise(newUp);
			//print(newUp);

			//pass in updated values to vertex shader
			glUniformMatrix4fv(view_matrix_location, 1, GL_FALSE, view_mat.m);
		}



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
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

		glUseProgram(shader_program_VertexColourExample);
		glBindVertexArray(vao4);
		glDrawArrays(GL_TRIANGLES, 0,12);


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