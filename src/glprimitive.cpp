#include "glprimitive.h"
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

GLPrimitive::GLPrimitive() : vertexId_(0), indexId_(0), arrayId_(0) {

}

GLPrimitive::~GLPrimitive() {
    glDeleteBuffers(1, &vertexId_);
    glDeleteBuffers(1, &indexId_);
    glDeleteVertexArrays(1, &arrayId_);
}

void GLPrimitive::draw(GLShaderProgram *program, int instances) {
    glBindBuffer(GL_ARRAY_BUFFER, vertexId_);
    glVertexAttribPointer(program->getAttributeLocation("in_Position"), 3,
			  GL_FLOAT, GL_FALSE, sizeof(GLVertex), (GLvoid *)0);
    glVertexAttribPointer(program->getAttributeLocation("in_Normal"), 3,
			  GL_FLOAT, GL_FALSE, sizeof(GLVertex), (GLvoid *)12);
    glVertexAttribPointer(program->getAttributeLocation("in_TexCoord"), 3,
			  GL_FLOAT, GL_FALSE, sizeof(GLVertex), (GLvoid *)24);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glEnableVertexAttribArray(program->getAttributeLocation("in_Position"));
    glEnableVertexAttribArray(program->getAttributeLocation("in_Normal"));
    glEnableVertexAttribArray(program->getAttributeLocation("in_TexCoord"));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId_);
    glDrawElementsInstanced(type_, idxCount_, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0), instances);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(program->getAttributeLocation("in_Position"));
    glDisableVertexAttribArray(program->getAttributeLocation("in_Normal"));
    glDisableVertexAttribArray(program->getAttributeLocation("in_TexCoord"));
}

void GLPrimitive::draw(GLShaderProgram *program) {
    glBindBuffer(GL_ARRAY_BUFFER, vertexId_);
    glVertexAttribPointer(program->getAttributeLocation("in_Position"), 3,
			  GL_FLOAT, GL_FALSE, sizeof(GLVertex), (GLvoid *)0);
    glVertexAttribPointer(program->getAttributeLocation("in_Normal"), 3,
			  GL_FLOAT, GL_FALSE, sizeof(GLVertex), (GLvoid *)12);
    glVertexAttribPointer(program->getAttributeLocation("in_TexCoord"), 3,
			  GL_FLOAT, GL_FALSE, sizeof(GLVertex), (GLvoid *)24);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    glEnableVertexAttribArray(program->getAttributeLocation("in_Position"));
    glEnableVertexAttribArray(program->getAttributeLocation("in_Normal"));
    glEnableVertexAttribArray(program->getAttributeLocation("in_TexCoord"));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId_);
    glDrawElements(type_, idxCount_, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(program->getAttributeLocation("in_Position"));
    glDisableVertexAttribArray(program->getAttributeLocation("in_Normal"));
    glDisableVertexAttribArray(program->getAttributeLocation("in_TexCoord"));
}

void GLPrimitive::draw() {
     glBindBuffer(GL_ARRAY_BUFFER, vertexId_);
     glEnableClientState(GL_VERTEX_ARRAY);
     glVertexPointer(3, GL_FLOAT, sizeof(GLVertex), BUFFER_OFFSET(vOffset_));   //The starting point of the VBO, for the vertices
     glEnableClientState(GL_NORMAL_ARRAY);
     glNormalPointer(GL_FLOAT, sizeof(GLVertex), BUFFER_OFFSET(nOffset_));   //The starting point of normals, 12 bytes away
     glClientActiveTexture(GL_TEXTURE0);
     glEnableClientState(GL_TEXTURE_COORD_ARRAY);
     glTexCoordPointer(3, GL_FLOAT, sizeof(GLVertex), BUFFER_OFFSET(tOffset_));   //The starting point of texcoords, 24 bytes away
     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId_);
     //To render, we can either use glDrawElements or glDrawRangeElements
     //The is the number of indices. 3 indices needed to make a single triangle
     glDrawElements(type_, idxCount_, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));   //The starting point of the IBO
     glDisableClientState(GL_VERTEX_ARRAY);
     glBindBuffer(GL_ARRAY_BUFFER, 0);
     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

GLQuad::GLQuad(float3 tess, float3 translate, float3 scale) {
     this->tesselate(tess, translate, scale);
}


void GLQuad::tesselate(float3 tess, float3 translate, float3 scale) {

    if(vertexId_) glDeleteBuffers(1, &vertexId_);
    if(indexId_) glDeleteBuffers(1, &indexId_);
    if(arrayId_) glDeleteVertexArrays(1, &arrayId_);


    type_ = GL_QUADS;
    idxCount_ = 4 * tess.x * tess.y;
    float3 delta = scale / tess;
    float3 tdelta = 1.0 / tess;
    delta.z = 0;

    glGenVertexArrays(1, &arrayId_);
    glBindVertexArray(arrayId_);

    GLVertex *pVertex = new GLVertex[(int)((tess.x + 1) * (tess.y + 1))];
    for(int y=0, i=0; y<=tess.y; y++) {
	for(int x=0; x<=tess.x; x++, i++) {
	    pVertex[i].p = float3(-0.5, -0.5, 0.0) * scale  + translate + delta * float3(x, y, 0);
	    pVertex[i].n = float3(0.0, 0.0, 1.0);
	    pVertex[i].t = float3(x, tess.y - y, 0) * tdelta;
	}
    }
    glGenBuffers(1, &vertexId_);
    glBindBuffer(GL_ARRAY_BUFFER, vertexId_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLVertex)*((tess.x + 1) * (tess.y + 1)), &pVertex[0].p.x, GL_STATIC_DRAW);

    unsigned short *pIndices = new unsigned short[idxCount_];
    for(int y=0, i=0; y<tess.y; y++) {
	for(int x=0; x<tess.x; x++, i+=4) {
	   pIndices[i] = y*(tess.x+1)+x;
	   pIndices[i+1] = y*(tess.x+1)+x+1;
	   pIndices[i+2] = (y+1)*(tess.x+1)+x+1;
	   pIndices[i+3] = (y+1)*(tess.x+1)+x;

	}
    }

    glGenBuffers(1, &indexId_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*idxCount_, &pIndices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
    delete[] pVertex, delete[] pIndices;

    //byte offsets in the vertex struct...this may as well be hard coded for now.
    vOffset_ = 0;
    nOffset_ = 12;
    tOffset_ = 24;
}

GLPlane::GLPlane(float3 tess, float3 translate, float3 scale) {
     this->tesselate(tess, translate, scale);
}

void GLPlane::tesselate(float3 tess, float3 translate, float3 scale) {

    if(vertexId_) glDeleteBuffers(1, &vertexId_);
    if(indexId_) glDeleteBuffers(1, &indexId_);
    if(arrayId_) glDeleteVertexArrays(1, &arrayId_);

    glGenVertexArrays(1, &arrayId_);
    glBindVertexArray(arrayId_);

    type_ = GL_TRIANGLES;
    idxCount_ = 6 * tess.x * tess.z;
    float3 delta = scale / tess;
    float3 tdelta = 1.0 / tess;
    delta.y = 0;
    GLVertex *pVertex = new GLVertex[(int)((tess.x + 1) * (tess.z + 1))];
    for(int z=0, i=0; z<=tess.z; z++) {
	for(int x=0; x<=tess.x; x++, i++) {
	    pVertex[i].p = float3(-0.5, 0.0, -0.5) * scale + translate + delta * float3(x, 0, z);
	    pVertex[i].n = float3(0.0, 1.0, 0.0);
	    pVertex[i].t = float3(x, z, 0) * tdelta;
	}
    }
    glGenBuffers(1, &vertexId_);
    glBindBuffer(GL_ARRAY_BUFFER, vertexId_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLVertex)*((tess.x + 1) * (tess.z + 1)), &pVertex[0].p.x, GL_STATIC_DRAW);

    unsigned short *pIndices = new unsigned short[idxCount_];
    for(int y=0, i=0; y<tess.z; y++) {
	for(int x=0; x<tess.x; x++, i+=6) {
	   pIndices[i] = y*(tess.x+1)+x;
	   pIndices[i+1] = y*(tess.x+1)+x+1;
	   pIndices[i+2] = (y+1)*(tess.x+1)+x+1;

	   pIndices[i+3] = y*(tess.x+1)+x;
	   pIndices[i+4] = (y+1)*(tess.x+1)+x+1;
	   pIndices[i+5] = (y+1)*(tess.x+1)+x;

	}
    }

    glGenBuffers(1, &indexId_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexId_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*idxCount_, &pIndices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);
    delete[] pVertex, delete[] pIndices;

    //byte offsets in the vertex struct...this may as well be hard coded for now.
    vOffset_ = 0;
    nOffset_ = 12;
    tOffset_ = 24;
}

GLIcosohedron::GLIcosohedron(float3 tess, float3 translate, float3 scale) {
    this->tesselate(tess, translate, scale);
}

void GLIcosohedron::tesselate(float3 tess, float3 translate, float3 scale) {
    const int pIndices[] = {
	2, 1, 0,
	3, 2, 0,
	4, 3, 0,
	5, 4, 0,
	1, 5, 0,
	11, 6,  7,
	11, 7,  8,
	11, 8,  9,
	11, 9,  10,
	11, 10, 6,
	1, 2, 6,
	2, 3, 7,
	3, 4, 8,
	4, 5, 9,
	5, 1, 10,
	2,  7, 6,
	3,  8, 7,
	4,  9, 8,
	5, 10, 9,
	1, 6, 10 };
    GLVertex pVertex[12];
    pVertex[0].p = pVertex[0].n = float3(0.000f,  0.000f, 1.000f);
    pVertex[1].p = pVertex[1].n = float3(0.894f,  0.000f,  0.447f);
    pVertex[2].p = pVertex[2].n = float3(0.276f,  0.851f,  0.447f);
    pVertex[3].p = pVertex[3].n = float3(-0.724f,  0.526f,  0.447f);
    pVertex[4].p = pVertex[4].n = float3(-0.724f, -0.526f,  0.447f);
    pVertex[5].p = pVertex[5].n = float3( 0.276f, -0.851f,  0.447f);
    pVertex[6].p = pVertex[6].n = float3( 0.724f,  0.526f, -0.447f);
    pVertex[7].p = pVertex[7].n = float3(-0.276f,  0.851f, -0.447f);
    pVertex[8].p = pVertex[8].n = float3(-0.894f,  0.000f, -0.447f);
    pVertex[9].p = pVertex[9].n = float3(-0.276f, -0.851f, -0.447f);
    pVertex[10].p = pVertex[10].n = float3(0.724f, -0.526f, -0.447f);
    pVertex[11].p = pVertex[11].n = float3(0.000f,  0.000f, -1.000f);
/*
    const float Verts[] = {
	 0.000f,  0.000f,  1.000f,
	 0.894f,  0.000f,  0.447f,
	 0.276f,  0.851f,  0.447f,
	-0.724f,  0.526f,  0.447f,
	-0.724f, -0.526f,  0.447f,
	 0.276f, -0.851f,  0.447f,
	 0.724f,  0.526f, -0.447f,
	-0.276f,  0.851f, -0.447f,
	-0.894f,  0.000f, -0.447f,
	-0.276f, -0.851f, -0.447f,
	 0.724f, -0.526f, -0.447f,
	 0.000f,  0.000f, -1.000f };
*/
    idxCount_ = sizeof(pIndices) / sizeof(pIndices[0]);

    // Create the VAO:
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create the VBO for positions:
    GLuint positions;
    GLsizei stride = 3 * sizeof(float);
    glGenBuffers(1, &positions);
    glBindBuffer(GL_ARRAY_BUFFER, positions);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pVertex), pVertex, GL_STATIC_DRAW);
    //glEnableVertexAttribArray(PositionSlot);
    //glVertexAttribPointer(PositionSlot, 3, GL_FLOAT, GL_FALSE, stride, 0);

    // Create the VBO for indices:
    GLuint indices;
    glGenBuffers(1, &indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pIndices), pIndices, GL_STATIC_DRAW);
}
