#include "glshaderprogram.h"
#include <stdlib.h>
#include <stdio.h>
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
    GLuint id = glCreateShader(type);
    GLint length;
    char *str = readFile(source.c_str(), length);
    glShaderSource(id, 1, (const char **)&str , &length);
    glCompileShader(id);
    printLog(id);
    free(str);
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
	if (!(str = (char *)malloc(len * sizeof(char)))) {
		cerr << "Can't malloc space for " << path << endl;
		return NULL;
	}
	r = fread(str, sizeof(char), len, fd);
	str[r - 1] = '\0'; /* Shader sources have to term with null */
	fclose(fd);

	return str;
}
