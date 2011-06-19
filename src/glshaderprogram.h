#ifndef GLSHADERPROGRAM_H
#define GLSHADERPROGRAM_H

#include "common.h"
#include "glcommon.h"

#include <vector>

class GLShaderProgram
{
public:
    GLShaderProgram();
    ~GLShaderProgram();

    void loadShaderFromSource(GLenum type, std::string source);

    bool link();
    void bind() { glUseProgram(programId_); }
    void release() { glUseProgram(0); }

    inline GLuint getUniformLocation(const char *name) {
	glGetUniformLocation(programId_, name);
    }

    inline void setGeometryInputType(GLenum type) {
	glProgramParameteriEXT(programId_, GL_GEOMETRY_INPUT_TYPE_EXT, type);
    }

    inline void setGeometryOutputType(GLenum type) {
	glProgramParameteriEXT(programId_, GL_GEOMETRY_OUTPUT_TYPE_EXT, type);
    }

    inline void setUniformValue(const char *name, float val) {
	GLint loc = glGetUniformLocation(programId_, name);
	glUniform1f(loc, val);
    }

    inline void setUniformValue(const char *name, int val){
	GLint loc = glGetUniformLocation(programId_, name);
	glUniform1i(loc, val);
    }

    inline void setUniformValue(const char *name, double val){
	GLint loc = glGetUniformLocation(programId_, name);
	glUniform1d(loc, val);
    }

    inline void setUniformValue(const char *name, unsigned int val){
	GLint loc = glGetUniformLocation(programId_, name);
	glUniform1ui(loc, val);
    }

protected:

    char *readFile(const char *path, GLint &length);

    std::vector<GLuint> shaders_;
    GLuint programId_;

};

#endif // GLSHADERPROGRAM_H
