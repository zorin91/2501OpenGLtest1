	#version 410

	layout(location = 0) in vec3 vertex_position;
	layout(location = 1) in vec2 vt;
	out vec2 texture_coordinates;
	uniform mat4 matrix;
	uniform mat4 view, proj;
	void main()
	{
		texture_coordinates = vt;
		gl_Position = proj * view * matrix * vec4(vertex_position, 1.0);
	};