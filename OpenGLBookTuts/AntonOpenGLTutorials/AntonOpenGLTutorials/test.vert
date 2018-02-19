	#version 410
	in vec3 vp;
	void main()
	{
		gl_Position = vec4(vp[0], vp[1], vp[2], 1.0);
	};