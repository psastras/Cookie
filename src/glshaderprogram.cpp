#include "glshaderprogram.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <fstream>

using namespace std;
GLShaderProgram::GLShaderProgram() {
    programId_ =  glCreateProgram();
}

GLShaderProgram::~GLShaderProgram() {
    while(!shaders_.empty()) {
	glDeleteShader(shaders_.back());
	shaders_.pop_back();
    }
    glDeleteProgram(programId_);
}

void printLog(GLuint obj) {
    int infologLength = 0;
    char infoLog[1024];
	if (glIsShader(obj))
		glGetShaderInfoLog(obj, 1024, &infologLength, infoLog);
	else
		glGetProgramInfoLog(obj, 1024, &infologLength, infoLog);
    if (infologLength > 0)
		printf("%s\n", infoLog);
    fflush(stdout);
}

void GLShaderProgram::loadShaderFromSource(GLenum type, std::string source) {
    stringstream ss;
    if(type == GL_FRAGMENT_SHADER)
	ss << "#define _FRAGMENT_" << endl;
    else if(type == GL_VERTEX_SHADER)
	ss << "#define _VERTEX_" << endl;
    else if(type == GL_GEOMETRY_SHADER)
	ss << "#define _GEOMETRY_" << endl;
    ifstream file(source.c_str());
    string line;
    if (file.is_open()) {
       while (file.good()) {
	 getline(file, line);
	 ss << line << endl;
      }
      file.close();
    } else {
	cerr << "Failed to open file " << source << endl;
	return;
    }
    std::string str = ss.str();
    int length = str.length();
    const char *data = str.c_str();
    GLuint id = glCreateShader(type);
    glShaderSource(id, 1, (const char **)&data, &length);
    glCompileShader(id);
    printLog(id);
    glAttachShader(programId_, id);
    shaders_.push_back(id);
}


bool GLShaderProgram::link() {
    glLinkProgram(programId_);
    return true;
}

char * GLShaderProgram::readFile(const char *path, GLint &length) {

	FILE *fd;
	long len, r;
	char *str;
	if (!(fd = fopen(path, "r"))) {
		cerr << "Can't open file '%s' for reading " << path << endl;
		return NULL;
	}
	fseek(fd, 0, SEEK_END);
	len = ftell(fd);
	length = len;
	fseek(fd, 0, SEEK_SET);
	if (!(str = (char *)malloc((len) * sizeof(char)))) {
		cerr << "Can't malloc space for " << path << endl;
		return NULL;
	}
	r = fread(str, sizeof(char), len, fd);
	//str[r - 1] = '\0'; /* Shader sources have to term with null */
	fclose(fd);

	return str;
}
