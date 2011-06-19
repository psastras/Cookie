#version 130

uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;
uniform sampler2D fftTex;

#ifdef _VERTEX_

void main(void) {
    gl_TexCoord[0]  = gl_MultiTexCoord0;
    gl_Position = ftransform();
}

#endif

#ifdef _FRAGMENT_


void main() {
    gl_FragColor = vec4(0.4, 0.1, 0.5, 1.0);
}
#endif
