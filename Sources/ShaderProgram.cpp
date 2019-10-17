#include "ShaderProgram.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <exception>
#include <ios>

using namespace std;

// Create a GPU program i.e., a graphics pipeline
ShaderProgram::ShaderProgram () : m_id (glCreateProgram ()) {}


ShaderProgram::~ShaderProgram () {
	glDeleteProgram (m_id); 
}

std::string ShaderProgram::file2String (const std::string & filename) 
{
	std::ifstream input (filename.c_str ());

	if (!input)
		throw std::ios_base::failure ("[Shader Program][file2String] Error: cannot open " + filename);

	std::stringstream buffer;
	buffer << input.rdbuf ();
	return buffer.str ();
}

GLuint ShaderProgram::loadShader (GLenum type, const std::string & shaderFilename) 
{
	GLint compile_status = GL_TRUE;
	GLuint shader = glCreateShader (type); // Create the shader, e.g., a vertex shader to be applied to every single vertex of a mesh
	std::string shaderSourceString = file2String (shaderFilename); // Loads the shader source from a file to a C++ string
	const GLchar * shaderSource = (const GLchar *)shaderSourceString.c_str (); // Interface the C++ string through a C pointer
	glShaderSource (shader, 1, &shaderSource, NULL); // Load the vertex shader source code
	/*glCompileShader (shader);  // THe GPU driver compile the shader
	glAttachShader (m_id, shader); // Set the vertex shader as the one to be used with the program/pipeline
	glDeleteShader (shader);*/
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
	if (compile_status != GL_TRUE) {
		GLint log_size;
		char* shader_log;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
		shader_log = (char*)std::malloc(log_size + 1); /* +1 pour le caractere de fin de chaine '\0' */
		if (shader_log != 0) {
			glGetShaderInfoLog(shader, log_size, &log_size, shader_log);
			std::cerr << "SHADER " << shaderFilename << ": " << shader_log << std::endl;
			std::free(shader_log);
		}
	}
	return shader;
}

std::shared_ptr<ShaderProgram> ShaderProgram::genBasicShaderProgram (const std::string & vertexShaderFilename, 
																	 const std::string& geometryShaderFilename,
															 	 	 const std::string & fragmentShaderFilename) 
{
	std::shared_ptr<ShaderProgram> shaderProgramPtr = std::make_shared<ShaderProgram> ();
	GLuint vertex_shader = shaderProgramPtr->loadShader (GL_VERTEX_SHADER, vertexShaderFilename);
	GLuint geometry_shader = shaderProgramPtr->loadShader(GL_GEOMETRY_SHADER, geometryShaderFilename);
	GLuint fragment_shader = shaderProgramPtr->loadShader (GL_FRAGMENT_SHADER, fragmentShaderFilename);

	GLuint program_id;
	GLint link_status = GL_TRUE;
	program_id = glCreateProgram();
	glAttachShader(program_id, vertex_shader);
	glAttachShader(program_id, geometry_shader);
	glAttachShader(program_id, fragment_shader);
	glLinkProgram(program_id);
	glGetProgramiv(program_id, GL_LINK_STATUS, &link_status);
	if (link_status != GL_TRUE) {
		GLint log_size;
		char* program_log;
		glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_size);
		program_log = (char*)std::malloc(log_size + 1); /* +1 pour le caractere de fin de chaine '\0' */
		if (program_log != 0) {
			glGetProgramInfoLog(program_id, log_size, &log_size, program_log);
			std::cerr << "Program " << program_log << std::endl;
			std::free(program_log);
		}
		glDeleteProgram(program_id);
		glDeleteShader(vertex_shader);
		glDeleteShader(geometry_shader);
		glDeleteShader(fragment_shader);
		program_id = 0;
	}
	glUseProgram(program_id);

	return shaderProgramPtr;
}