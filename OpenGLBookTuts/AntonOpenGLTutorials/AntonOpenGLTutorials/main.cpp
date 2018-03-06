#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
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
	
	GLFWwindow* window = glfwCreateWindow(vmode->width , vmode->height , "Hello Triangle", mon, NULL);
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
	//glEnable(GL_CULL_FACE); //cull face
	//glCullFace(GL_BACK); //cull face back
	//glFrontFace(GL_CW); //GL_CW for clock-wise ... GL_CCW for counter clock-wise


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
		0.0f, 0.0f, -4.0f, 1.0f									//3, 7, 11, 15,
	};

	float points_skybox[] = {
		-10.0f,  10.0f, -10.0f,
		-10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f,  10.0f, -10.0f,
		-10.0f,  10.0f, -10.0f,

		-10.0f, -10.0f,  10.0f,
		-10.0f, -10.0f, -10.0f,
		-10.0f,  10.0f, -10.0f,
		-10.0f,  10.0f, -10.0f,
		-10.0f,  10.0f,  10.0f,
		-10.0f, -10.0f,  10.0f,

		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f,  10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,

		-10.0f, -10.0f,  10.0f,
		-10.0f,  10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f, -10.0f,  10.0f,
		-10.0f, -10.0f,  10.0f,

		-10.0f,  10.0f, -10.0f,
		10.0f,  10.0f, -10.0f,
		10.0f,  10.0f,  10.0f,
		10.0f,  10.0f,  10.0f,
		-10.0f,  10.0f,  10.0f,
		-10.0f,  10.0f, -10.0f,

		-10.0f, -10.0f, -10.0f,
		-10.0f, -10.0f,  10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		-10.0f, -10.0f,  10.0f,
		10.0f, -10.0f,  10.0f
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

	//sets clear color to grey
	glClearColor(0.5, 0.5, 0.5, 1.0);

	

	//camera variable setup
	float cam_speed = 5.0f;
	float cam_yaw_speed = 50.0f;
	float cam_pitch_speed = 50.0f;
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

	GLuint skybox = 0;

	create_cube_map(RELPATH"bkg1_back6.png", RELPATH"bkg1_front5.png", RELPATH"bkg1_top3.png", RELPATH"bkg1_bottom4.png", RELPATH"bkg1_left2.png", RELPATH"bkg1_right1.png",&skybox);

	int tex_loc = glGetUniformLocation(skybox_program, "cube_texture");

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
		if (glfwGetKey(window, GLFW_KEY_A))
		{
			cam_pos -= view_right *  cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if (glfwGetKey(window, GLFW_KEY_D))
		{
			cam_pos += view_right *  cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if (glfwGetKey(window, GLFW_KEY_Q))
		{
			cam_pos += view_up * cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if (glfwGetKey(window, GLFW_KEY_E))
		{
			cam_pos -= view_up * cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if (glfwGetKey(window, GLFW_KEY_W))
		{
			//print(view_forward);
			//cam_pos.v[2] -= cam_speed * elapsed_seconds;
			cam_pos += view_forward * cam_speed * elapsed_seconds;
			//printf("New componenets. X:%f,Y:%f,Z:%f\n",cam_pos.v[0], cam_pos.v[1], cam_pos.v[2]);
			cam_moved = true;
		}
		if (glfwGetKey(window, GLFW_KEY_S))
		{
			//cam_pos.v[2] += cam_speed * elapsed_seconds;
			
			cam_pos -= view_forward * cam_speed * elapsed_seconds;
			//printf("New componenets. X:%f,Y:%f,Z:%f\n", cam_pos.v[0], cam_pos.v[1], cam_pos.v[2]);
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
			cam_yaw -= mouseXDisplacement;
			if (cam_yaw < 0.0f)
			{
				cam_yaw = 360.0f;
			}
			
			cam_moved = true;
		}

		if (mouseXDisplacement < 0.0)
		{
			cam_yaw -= mouseXDisplacement;
			if (cam_yaw > 360.0f)
			{
				cam_yaw = 0.0f;
			}
			cam_moved = true;
		}

		if (mouseYDisplacement > 0.0)
		{

			cam_pitch -= mouseYDisplacement;

			if (cam_pitch < -89.0f)
			{
				cam_pitch = -89.0f;
			}
			cam_moved = true;



		}

		if (mouseYDisplacement < 0.0)
		{

			cam_pitch -= mouseYDisplacement;
			if (cam_pitch > 89.0f)
			{
				cam_pitch = 89.0f;
			}
			cam_moved = true;

		}
		//we update/recalculate our view matrix if one of the previous keys were pressed
		if (cam_moved)
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
			printf("test forward vec\n");
			print(actual_offset_final);

			//rotate_vector_by_quaternion(actual_offset_result, quat_pitch, actual_offset_final);

			printf("result of second quaternion applied to offset: x:%f y:%f z:%f angle used:%f\n", actual_offset_final.v[0], actual_offset_final.v[1], actual_offset_final.v[2], cam_pitch);

			view_forward = actual_offset_final;

			view_forward = normalise(view_forward);

			view_right = cross(view_forward, vec3(0.0f,1.0f,0.0f));

			view_right = normalise(view_right);

			//vec3 test_up = cross(view_right, view_forward);
			//test_up = normalise(test_up);
			//printf("test up vector:");
			//print(test_up);

		//	printf("forward vector:");
		//	print(view_forward);
		//	printf("right vector:");
		//	print(view_right);

			view_up = cross(view_right,view_forward);
			view_up = normalise(view_up);


			//printf("forward vector:");
			//print(view_forward);
			//printf("right vector:");
			//print(view_right);
			printf("up vector:");
			print(view_up);
			printf("-----------------------------------------------------------------\n");

			//vec3 look_at_target = actual_offset_result + cam_pos;
			vec3 look_at_target = actual_offset_final + cam_pos;
			printf("final offset value:");
			print(actual_offset_final);

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