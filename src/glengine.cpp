#include "glengine.h"
#include "common.h"
#include "glcommon.h"
#include "glframebufferobject.h"
#include "glprimitive.h"
#include "glshaderprogram.h"
#include "keyboardcontroller.h"

GLFramebufferObject *pMultisampleFramebuffer, *pFramebuffer;
GLPrimitive *pQuad;
GLEngine::GLEngine(WindowProperties &properties) {

    //init gl setup
    glClearColor(0.0, 0.0, 0.0, 1.0);
    //glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glDisable(GL_DITHER);

    camera_.center = float3(0.0, 0.0, 0.0);
    camera_.eye = float3(0.0, 0.0, 1.0);
    camera_.up = float3(0.0, 1.0, 0.0);
    camera_.near = 0.1;
    camera_.far = 100.0;
    camera_.fovy = 60.0;

    GLFramebufferObjectParams params;
    params.width = properties.width;
    params.height = properties.height;
    params.hasDepth = true;
    params.depthFormat = GL_DEPTH_COMPONENT;
    params.format = GL_RGBA;
    params.nColorAttachments = 1;
    params.nSamples = 16;

    pMultisampleFramebuffer = new GLFramebufferObject(params);

    params.hasDepth = true;
    params.nSamples = 0;
    params.format = GL_RGBA;

    pFramebuffer = new GLFramebufferObject(params);


    quad0_ = new GLQuad(float3(136, 77, 0),
			float3(0.5, 0.5, 0),
			float3(1366, 768, 1));

    quad1_ = new GLQuad(float3(1, 1, 0),
			float3(0.5, 0.5, 0),
			float3(1366, 768, 1));

    plane0_ = new GLPlane(float3(10, 0, 10),
			 float3(0, 0, 0),
			 float3(100, 1, 100));

    //load shader programs
    shaderPrograms_["default"] = new GLShaderProgram();
    shaderPrograms_["default"]->loadShaderFromSource(GL_VERTEX_SHADER, "shaders/default.glsl");
    shaderPrograms_["default"]->loadShaderFromSource(GL_FRAGMENT_SHADER, "shaders/default.glsl");
    shaderPrograms_["default"]->link();

    shaderPrograms_["water"] = new GLShaderProgram();
    shaderPrograms_["water"]->loadShaderFromSource(GL_VERTEX_SHADER, "shaders/water.glsl");
    shaderPrograms_["water"]->loadShaderFromSource(GL_FRAGMENT_SHADER, "shaders/water.glsl");
    shaderPrograms_["water"]->link();
}


GLEngine::~GLEngine() {
    delete shaderPrograms_["default"];
}

void GLEngine::resize(int w, int h) {
    width_ = w; height_ = h;
    glViewport(0, 0, w, h);
}

void GLEngine::draw(int time, float dt, const KeyboardController *keyController) {
    processKeyEvents(keyController, dt);

    glPolygonMode(GL_FRONT, GL_LINE);
    glEnable(GL_DEPTH_TEST);
    int w = width_, h = height_;
    camera_.perspective_camera(width_, height_);
    pMultisampleFramebuffer->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shaderPrograms_["water"]->bind();
    plane0_->draw();
    shaderPrograms_["water"]->release();
    pMultisampleFramebuffer->release();
    pMultisampleFramebuffer->blit(*pFramebuffer);

    glDisable(GL_DEPTH_TEST);
    camera_.orthogonal_camera(width_, height_);
    glPolygonMode(GL_FRONT, GL_FILL);
    shaderPrograms_["default"]->bind();
    glActiveTexture(GL_TEXTURE0);
    pFramebuffer->bindsurface(0);
    //glBindTexture(GL_TEXTURE_2D, pFramebuffer->texture()[0]);
    //glBindTexture(GL_TEXTURE_2D, pFramebuffer->depth());
    shaderPrograms_["default"]->setUniformValue("tex", 0);
    quad1_->draw();
    pFramebuffer->unbindsurface();
    shaderPrograms_["default"]->release();

}


void GLEngine::mouseMove(float dx, float dy, float dt) {
    float deltax = -dx*dt*1000.f;
    float deltay = -dy*dt*1000.f;
    float3 look = (camera_.center - camera_.eye).getNormalized();
    float3 lookNew = look.rotateY(deltax);
    camera_.center = camera_.eye + lookNew;
    //look = (camera_.center - camera_.eye).getNormalized();
    //lookNew = look.rotateX(deltay);
    //camera_.center = camera_.eye + lookNew;*/
}

void GLEngine::processKeyEvents(const KeyboardController *keycontroller, float dt) {
    float delta = dt*100;
    if(keycontroller->isKeyDown(25)) { //W
	const float3 &look = (camera_.center - camera_.eye).getNormalized() * delta;
	camera_.eye += look;
	camera_.center += look;
    } if(keycontroller->isKeyDown(38)) { //A
	const float3 &look = (camera_.center - camera_.eye).getNormalized();
	const float3 &perp = look.cross(camera_.up) * delta;
	camera_.eye -= perp;
	camera_.center -= perp;
    } if(keycontroller->isKeyDown(39)) { //S
	const float3 &look = (camera_.center - camera_.eye).getNormalized() * delta;
	camera_.eye -= look;
	camera_.center -= look;
    } if(keycontroller->isKeyDown(40)) { //D
	const float3 &look = (camera_.center - camera_.eye).getNormalized();
	const float3 &perp = look.cross(camera_.up) * delta;
	camera_.eye += perp;
	camera_.center += perp;
    } if(keycontroller->isKeyDown(65)) { //space
	camera_.eye += camera_.up*delta;
	camera_.center += camera_.up*delta;
    }
}
