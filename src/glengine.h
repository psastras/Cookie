#ifndef GLENGINE_H
#define GLENGINE_H

#include "glcommon.h"
#include <hash_map>
#include <cstring>
using namespace std;
using namespace __gnu_cxx;
class KeyboardController;
class GLPrimitive;
class GLShaderProgram;
class VSML;
class GLFFTWater;

struct WindowProperties {
    int width, height;
};

struct eqstr{
  bool operator()(const char* s1, const char* s2) const {
    return strcmp(s1,s2)==0;
  }
};

class GLEngine
{
public:
    GLEngine(WindowProperties &properties);
    ~GLEngine();
    void resize(int w, int h);
    void draw(float time, float dt, const KeyboardController *keycontroller); //time in s, dt in fraction of sec
    void mouseMove(float dx, float dy, float dt);

    int width() { return width_; }
    int height() { return height_; }

protected:

    void processKeyEvents(const KeyboardController *keycontroller, float dt);
    GLFFTWater *fftwater_;
    GLPrimitive *quad0_, *quad1_, *plane0_;
    int width_, height_;
    Camera camera_;
    VSML *vsml_;
    hash_map<const char*, GLShaderProgram *, hash<const char*>, eqstr> shaderPrograms_;
};

#endif // GLENGINE_H
