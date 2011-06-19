#include "glfftwater.h"
#include <tr1/random>
#include <malloc.h>

GLFFTWater::GLFFTWater(GLFFTWaterParams &params) {
    posix_memalign((void **)&m_h, 16, sizeof(double) * (params.N+2)*(params.N));
    posix_memalign((void **)&m_dx, 16, sizeof(double) * (params.N+2)*(params.N));
    posix_memalign((void **)&m_dz, 16, sizeof(double) * (params.N+2)*(params.N));
    posix_memalign((void **)&m_w, 16, sizeof(double) * (params.N)*(params.N));
    m_htilde0 = (fftw_complex *)fftw_malloc(sizeof(fftw_complex)*(params.N)*(params.N));
    m_htilde = (fftw_complex *)fftw_malloc(sizeof(fftw_complex)*(params.N)*(params.N));
    m_heightmap = new float4[(params.N)*(params.N)];
    m_params = params;
    std::tr1::mt19937 prng(1000);
    std::tr1::normal_distribution<double> normal;
    std::tr1::uniform_real<double> uniform;
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::normal_distribution<double> > randn(prng,normal);
    std::tr1::variate_generator<std::tr1::mt19937, std::tr1::uniform_real<double> > randu(prng,uniform);
    for(int i=0, k=0; i<params.N; i++) {
	    float k_x = (-(params.N-1)*0.5f+i)*(2.f*M_PI / params.L);
	    for(int j=0; j<params.N; j++, k++) {
		    float k_y = (-(params.N-1)*0.5f+j)*(2.f*M_PI / params.L);
		    float A = randn();
		    float theta = randu() * 2.f * M_PI;
		    float P = (k_x==0.f && k_y==0.0f) ? 0.f : sqrtf(phillips(k_x,k_y,m_w[k]));
		    m_htilde0[k][0] = m_htilde0[k][1] = P*A*sinf(theta);
	    }
    }

    glGenTextures(1, &m_texId);
    glBindTexture(GL_TEXTURE_2D, m_texId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, params.N, params.N, 0, GL_RGBA, GL_FLOAT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

float GLFFTWater::phillips(float kx, float ky, double& w) {
	const float damping = 1.f / 50.f;
	float kk = kx*kx+ky*ky;
	float kw = kx*cosf(m_params.w)+ky*sinf(m_params.w);
	float l = m_params.V*m_params.V /  9.81;
	w = powf( 9.81*sqrtf(kk), 0.85f); //compute the dispersion relation
	float p = m_params.A*expf(-1.f / (l*l*kk))/(kk*kk*kk)*(kw*kw);
	float d = expf(-kk*damping*damping);
	return kw < 0.f ? p*0.25f*d : p*d;
}

GLuint GLFFTWater::heightfieldTexture() {
    glBindTexture(GL_TEXTURE_2D, m_texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, m_params.N, m_params.N, 0, GL_RGBA, GL_FLOAT, m_heightmap);
    glBindTexture(GL_TEXTURE_2D, 0);
}

/*
For an in-place transform, in
and out are aliased to the same array, which must be big enough to hold both; so, the
real array would actually have 2*(n/2+1) elements, where the elements beyond the first n
are unused padding. The kth element of the complex array is exactly the same as the kthss
element of the corresponding complex DFT.
*/
float4 *GLFFTWater::computeHeightfield(float t) {
	for(int i=0, k=0; i<m_params.N;i++) {
		int nk_x=m_params.N-1-i;
		for(int j=0; j<m_params.N; j++, k++) {
			int nk_y=m_params.N-1-j;
			int idx = (nk_x)*(m_params.N)+nk_y;
			float pcos = cosf(m_w[k]*t);
			float psin = sinf(m_w[k]*t);
			float t = m_htilde0[idx][0]*pcos-m_htilde0[idx][0]*psin;
			m_htilde[k][0] = m_htilde0[k][0]*pcos-m_htilde0[k][0]*psin+t;
			m_htilde[k][1] = -2.0f*m_htilde0[idx][0]*psin;//-m_htilde0[idx][0]*pcos+t;
		}
	}
	for(int y=0; y<m_params.N; y++) {
		float kz = (float) (y - m_params.N/2);
		for(int x=0; x<=m_params.N; x+=2) {
		float kx = (float) (x/2 - m_params.N/2);
			m_h[x+(m_params.N+2)*y] = m_htilde[y*(m_params.N)+x/2][0];
			m_h[x+1+(m_params.N+2)*y] = m_htilde[y*(m_params.N)+x/2][1];

			float k = (float) sqrtf(kx * kx + kz * kz);
			m_dx[x+(m_params.N+2)*y] =  -m_htilde[y*(m_params.N)+x/2][1]*kx/k;
			m_dx[x+1+(m_params.N+2)*y] =  m_htilde[y*(m_params.N)+x/2][0]*kx/k;
			m_dz[x+(m_params.N+2)*y] =  -m_htilde[y*(m_params.N)+x/2][1]*kz/k;
			m_dz[x+1+(m_params.N+2)*y] =  m_htilde[y*(m_params.N)+x/2][0]*kz/k;
		}
	}

	m_dx[m_params.N + (m_params.N+2)*(m_params.N/2)] = 0.f;
	m_dz[m_params.N + (m_params.N+2)*(m_params.N/2)] = 0.f;
	m_dx[m_params.N + (m_params.N+2)*(m_params.N/2)+1] = 0.f;
	m_dz[m_params.N + (m_params.N+2)*(m_params.N/2)+1] = 0.f;

	if(!m_haveFFTPlan) {
		m_fftplan = fftw_plan_dft_c2r_2d(m_params.N, m_params.N, (fftw_complex *)m_h, m_h, FFTW_MEASURE);
		m_haveFFTPlan = true;
	}

	fftw_execute_dft_c2r(m_fftplan, (fftw_complex *)m_h, m_h);
	fftw_execute_dft_c2r(m_fftplan, (fftw_complex *)m_dx, m_dx);
	fftw_execute_dft_c2r(m_fftplan, (fftw_complex *)m_dz, m_dz);

	for(int y=0; y<m_params.N;y++) {
		for(int x=0; x<m_params.N; x++) {
			if((x+y)%2==0) {
				m_heightmap[x+m_params.N*y].x = m_dx[x+(m_params.N+2)*y]*m_params.L / (float)m_params.N;
				m_heightmap[x+m_params.N*y].y = m_h[x+(m_params.N+2)*y];
				m_heightmap[x+m_params.N*y].z = m_dz[x+(m_params.N+2)*y]*m_params.L / (float)m_params.N;
			} else {
				m_heightmap[x+m_params.N*y].x = -m_dx[x+(m_params.N+2)*y]*m_params.L / (float)m_params.N;
				m_heightmap[x+m_params.N*y].y = -m_h[x+(m_params.N+2)*y];
				m_heightmap[x+m_params.N*y].z = -m_dz[x+(m_params.N+2)*y]*m_params.L / (float)m_params.N;
			}
		}
	}
	return m_heightmap;
}
