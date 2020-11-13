#pragma once
#include "glad/glad.h"

class Shader
{
private:
	GLuint program;
public:
	Shader(const GLchar* vertex_shader_src_path, const GLchar* fragment_shader_src_path);
	void useProgram();
	GLuint getProgram();
};
