#include "Shader.h"
#include <filesystem>

namespace fs = std::filesystem;

Shader::Shader(const char* vertexPath, const char* fragmentPath) {

	// Shader source code reading
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;

	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	// Try multiple path locations
	std::vector<std::string> vShaderPaths = {
		vertexPath,
		std::string("LearnOpenGL/") + vertexPath,
		std::string("../LearnOpenGL/") + vertexPath,
		std::string("../../LearnOpenGL/") + vertexPath,
	};

	std::vector<std::string> fShaderPaths = {
		fragmentPath,
		std::string("LearnOpenGL/") + fragmentPath,
		std::string("../LearnOpenGL/") + fragmentPath,
		std::string("../../LearnOpenGL/") + fragmentPath,
	};

	// Try to find and load the vertex shader
	bool vShaderFound = false;
	for (const auto& path : vShaderPaths) {
		if (fs::exists(path)) {
			try {
				vShaderFile.open(path);
				std::stringstream vShaderStream;
				vShaderStream << vShaderFile.rdbuf();
				vShaderFile.close();
				vertexCode = vShaderStream.str();
				vShaderFound = true;
				std::cout << "Loaded vertex shader: " << path << std::endl;
				break;
			}
			catch (std::ifstream::failure& e) {
				continue;
			}
		}
	}

	// Try to find and load the fragment shader
	bool fShaderFound = false;
	for (const auto& path : fShaderPaths) {
		if (fs::exists(path)) {
			try {
				fShaderFile.open(path);
				std::stringstream fShaderStream;
				fShaderStream << fShaderFile.rdbuf();
				fShaderFile.close();
				fragmentCode = fShaderStream.str();
				fShaderFound = true;
				std::cout << "Loaded fragment shader: " << path << std::endl;
				break;
			}
			catch (std::ifstream::failure& e) {
				continue;
			}
		}
	}

	if (!vShaderFound) {
		std::cout << "ERROR::SHADER::VERTEX_FILE_NOT_FOUND: " << vertexPath << std::endl;
	}
	if (!fShaderFound) {
		std::cout << "ERROR::SHADER::FRAGMENT_FILE_NOT_FOUND: " << fragmentPath << std::endl;
	}

	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();



	// Shader compilation
	unsigned int vertex, fragment;
	int success;
	char infoLog[512];


	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	ID = glCreateProgram();
	glAttachShader(ID,vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);

	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::use() {
	glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const {
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void Shader::setInt(const std::string& name, int value) const {
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setFloat(const std::string& name, float value) const {
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const {

	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));

}

void Shader::setVec3(const std::string& name, const glm::vec3& value)const {

	glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));

}
