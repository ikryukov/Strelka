#include "shaders.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

Shader::Shader(const GLchar* vertex_shader_src_path, const GLchar* fragment_shader_src_path)
{
	std::ifstream vs_file, frs_file;    // Files
	std::stringstream vs_buf, frs_buf;  // Buffers
	std::string vs_src_t, frs_src_t;    // Strings

	// Bind exceptions for files
	vs_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	frs_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		// Open files
		vs_file.open(vertex_shader_src_path);
		frs_file.open(fragment_shader_src_path);

		// Fill buffers
		vs_buf << vs_file.rdbuf();
		frs_buf << frs_file.rdbuf();

		// Convert stream into string
		vs_src_t = vs_buf.str();
		frs_src_t = frs_buf.str();

		// Close files
		vs_file.close();
		frs_file.close();
	}
	catch (std::ifstream::failure error)
	{
		std::cout << "ERROR::SHADER::READ_FAILURE" << std::endl;
	}

	// Convert string to C-strings
	const GLchar* vs_src = vs_src_t.c_str();
	const GLchar* frs_src = frs_src_t.c_str();

	GLint success;
	GLchar infoLog[512];

	// Create vertex shader
	GLuint vetrex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vetrex_shader, 1, &vs_src, nullptr);
	glCompileShader(vetrex_shader);

	// Check errors
	glGetShaderiv(vetrex_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vetrex_shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED::[ " << vertex_shader_src_path << " ]\n" << infoLog << std::endl;
	};

	// Create fragment shader
	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &frs_src, nullptr);
	glCompileShader(fragment_shader);

	// Check errors
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment_shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED::[ " << fragment_shader_src_path << " ]\n" << infoLog << std::endl;
	};

	// Attach shaders
	this->program = glCreateProgram();
	glAttachShader(this->program, vetrex_shader);
	glAttachShader(this->program, fragment_shader);
	glLinkProgram(this->program);

	// Delete unnecessary data
	glDeleteShader(vetrex_shader);
	glDeleteShader(fragment_shader);
}

void Shader::useProgram() { glUseProgram(this->program); }
GLuint Shader::getProgram() { return this->program; }
