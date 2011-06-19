#ifndef GLFFTWATER_H
#define GLFFTWATER_H

#include "glcommon.h"
#include <fftw3.h>

struct GLFFTWaterParams{
	int N; //fourier grid size;
	float L; //patch size
	float V; //
	float A; //global amplitude control
	float w; //wind direction in radians
};

class GLFFTWater
{
public:
    GLFFTWater(GLFFTWaterParams &params);
    float4 *computeHeightfield(float t);
    GLuint heightfieldTexture();

protected:
    float phillips(float kx, float ky, double& w);

    GLFFTWaterParams m_params;
    fftw_complex *m_htilde0, *m_htilde;
    fftw_plan m_fftplan;
    bool m_haveFFTPlan;
    double *m_w, *m_h, *m_dx, *m_dz; //the dispersion relation
    float4 *m_heightmap;
    GLuint m_texId;
};

#endif // GLFFTWATER_H
