#ifndef GLPRIMITIVE_H
#define GLPRIMITIVE_H

#include "glcommon.h"

struct GLVertex
{
  float3 p, n, t;
};


class GLPrimitive {
  public:
      ~GLPrimitive();

      virtual void tesselate(float3 tess, float3 translate, float3 scale) = 0; //tesselates and reuploads into vbo
      void draw();
  protected:
      GLPrimitive();

      GLuint vertexId_, indexId_, arrayId_;
      GLenum type_;
      GLuint idxCount_;
      int vOffset_, tOffset_, nOffset_;
};

class GLQuad : public GLPrimitive {
    public:
	GLQuad(float3 tess, float3 translate, float3 scale);
	~GLQuad();

	void tesselate(float3 tess, float3 translate, float3 scale);
};

class GLPlane : public GLPrimitive {
    public:
	GLPlane(float3 tess, float3 translate, float3 scale);
	~GLPlane();

	void tesselate(float3 tess, float3 translate, float3 scale);
};

#endif // GLPRIMITIVE_H
