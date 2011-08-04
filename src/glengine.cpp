#include "glengine.h"
#include "common.h"
#include "glcommon.h"
#include "glframebufferobject.h"
#include "glprimitive.h"
#include "glshaderprogram.h"
#include "glfftwater.h"
#include "keyboardcontroller.h"
#include "../3rdparty/VSML/vsml.h"

GLFramebufferObject *pMultisampleFramebuffer, *pFramebuffer;
GLPrimitive *pQuad;
GLEngine::GLEngine(WindowProperties &properties) {

    renderMode_ = FILL;

    //init gl setup
    vsml_ = VSML::getInstance();
    width_ = properties.width;
    height_ = properties.height;


    glClearColor(0.0, 0.0, 0.0, 1.0);
    glViewport(0,0,width_,height_);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glDisable(GL_DITHER);

    camera_.center = float3(0.0, 10.0, 0.0);
    camera_.eye = float3(0.0, 30.0, 50.0);
    camera_.up = float3(0.0, 1.0, 0.0);
    camera_.near = 0.1;
    camera_.far = 500.0;
    camera_.rotx = camera_.roty = 0.f;
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

    params.hasDepth = false;
    params.nSamples = 0;
    params.format = GL_RGBA;

    pFramebuffer = new GLFramebufferObject(params);


    quad0_ = new GLQuad(float3(136, 77, 0),
			float3(0.5, 0.5, 0),
			float3(width_, height_, 1));

    quad1_ = new GLQuad(float3(1, 1, 0),
			float3(width_ * 0.5, height_ * 0.5, 0),
			float3(width_, height_, 1));

    plane0_ = new GLPlane(float3(150, 0, 150),
			 float3(0, 0, 0),
			 float3(20, 1, 20));

    GLFFTWaterParams fftparams;
    fftparams.A = 0.000000075f;
    fftparams.V = 30.0f;
    fftparams.w = 200 * 3.14159f / 180.0f;
    fftparams.L = 250.0;
    fftparams.N = 256;
    fftparams.chop = 2.75;
    fftwater_ = new GLFFTWater(fftparams);

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
   // std::cout << w << " x " << h << std::endl;
}

void GLEngine::draw(float time, float dt, const KeyboardController *keyController) {

    //glClear(GL_COLOR_BUFFER_BIT);

    //compute ocean heightfield
    float3 *data = fftwater_->computeHeightfield(time*2);
    GLuint tex = fftwater_->heightfieldTexture();

    processKeyEvents(keyController, dt);

    if(renderMode_ == WIREFRAME) glPolygonMode(GL_FRONT, GL_LINE);
    glEnable(GL_DEPTH_TEST);
    int w = width_, h = height_;
    vsml_->perspective(camera_.fovy, w / (float)h, camera_.near, camera_.far);
    vsml_->loadIdentity(VSML::MODELVIEW);
    vsml_->rotate(camera_.rotx, 1.f, 0.f, 0.f);
    vsml_->rotate(camera_.roty, 0.f, 1.f, 0.f);
    vsml_->translate(-camera_.eye.x, -camera_.eye.y, -camera_.eye.z);

    pMultisampleFramebuffer->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shaderPrograms_["water"]->bind();
    vsml_->initUniformLocs(shaderPrograms_["water"]->getUniformLocation("modelviewMatrix"),
			   shaderPrograms_["water"]->getUniformLocation("projMatrix"));
    vsml_->matrixToUniform(VSML::MODELVIEW);
    vsml_->matrixToUniform(VSML::PROJECTION);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    shaderPrograms_["water"]->setUniformValue("fftTex", 0);
    shaderPrograms_["water"]->setUniformValue("N", (float)(fftwater_->params().N));
    shaderPrograms_["water"]->setUniformValue("L", (float)(fftwater_->params().L));
    shaderPrograms_["water"]->setUniformValue("D", 20.f);
    shaderPrograms_["water"]->setUniformValue("grid", float2(7.f, 7.f));
    shaderPrograms_["water"]->setUniformValue("eyePos", camera_.eye);
    shaderPrograms_["water"]->setUniformValue("sunPos", float3(0.0, 1.0, 0.0));
    plane0_->draw(shaderPrograms_["water"], 49);
    shaderPrograms_["water"]->release();
    pMultisampleFramebuffer->release();
    pMultisampleFramebuffer->blit(*pFramebuffer);

    glDisable(GL_DEPTH_TEST);
    if(renderMode_ == WIREFRAME) glPolygonMode(GL_FRONT, GL_FILL);
    vsml_->loadIdentity(VSML::PROJECTION);
    vsml_->ortho(0.f,(float)width_,(float)height_,0.f);
    vsml_->loadIdentity(VSML::MODELVIEW);

    shaderPrograms_["default"]->bind();
    vsml_->initUniformLocs(shaderPrograms_["default"]->getUniformLocation("modelviewMatrix"),
			   shaderPrograms_["default"]->getUniformLocation("projMatrix"));
    vsml_->matrixToUniform(VSML::MODELVIEW);
    vsml_->matrixToUniform(VSML::PROJECTION);
    glActiveTexture(GL_TEXTURE0);
    pFramebuffer->bindsurface(0);
    shaderPrograms_["default"]->setUniformValue("tex", 0);
    quad1_->draw(shaderPrograms_["default"]);
    pFramebuffer->unbindsurface();
    shaderPrograms_["default"]->release();

    camera_.orthogonal_camera(w, h);
}

float sensitivity = 50.f;
void GLEngine::mouseMove(float dx, float dy, float dt) {
    float deltax = -dx*dt*sensitivity;
    float deltay = -dy*dt*sensitivity;
    camera_.roty -= deltax;
    camera_.rotx += deltay;
}

void GLEngine::processKeyEvents(const KeyboardController *keycontroller, float dt) {

#ifdef _WIN32
#define KEY_W 87
#define KEY_A 65
#define KEY_S 83
#define KEY_D 68
#define KEY_SPACE 32
#define KEY_1 49
#define KEY_2 50
#else
#define KEY_W 25
#define KEY_A 38
#define KEY_S 39
#define KEY_D 40
#define KEY_SPACE 65
#endif

    float delta = dt*sensitivity;
    if(keycontroller->isKeyDown(KEY_W)) { //W
	float yrotrad = camera_.roty / 180 * 3.141592654f;
	float xrotrad = camera_.rotx / 180 * 3.141592654f;
	camera_.eye.x += sinf(yrotrad)*delta;
	camera_.eye.z -= cosf(yrotrad)*delta;
	camera_.eye.y -= sinf(xrotrad)*delta;
    } if(keycontroller->isKeyDown(KEY_A)) { //A
	float yrotrad = (camera_.roty / 180 * 3.141592654f);
	camera_.eye.x -= cosf(yrotrad)*delta;
	camera_.eye.z -= sinf(yrotrad)*delta;
    } if(keycontroller->isKeyDown(KEY_S)) { //S
	float yrotrad = camera_.roty / 180 * 3.141592654f;
	float xrotrad = camera_.rotx / 180 * 3.141592654f;
	camera_.eye.x -= sinf(yrotrad)*delta;
	camera_.eye.z += cosf(yrotrad)*delta;
	camera_.eye.y += sinf(xrotrad)*delta;
    } if(keycontroller->isKeyDown(KEY_D)) { //D
	float yrotrad = (camera_.roty / 180 * 3.141592654f);
	camera_.eye.x += cosf(yrotrad)*delta;
	camera_.eye.z += sinf(yrotrad)*delta;
    } if(keycontroller->isKeyDown(KEY_SPACE)) { //space
	camera_.eye.y += delta;
    } if(keycontroller->isKeyPress(KEY_1)) {
	this->setRenderMode(FILL);
    } if(keycontroller->isKeyPress(KEY_2)) {
	this->setRenderMode(WIREFRAME);
    }
}
