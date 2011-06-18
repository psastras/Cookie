uniform sampler2D fft; //input height field
uniform int N; //grid size

#ifdef _VERTEX_
void main(void) {
    gl_TexCoord[0]  = gl_MultiTexCoord0;
    //vec4 pos = gl_Vertex;
    //vec4 wspos = gl_ProjectionMatrixInverse*gl_ModelViewMatrixInverse*pos;
    // project the screen vertex into the world

    gl_Position = ftransform();// gl_ProjectionMatrix*gl_ModelViewMatrix*pos;
}

#endif

#ifdef _FRAGMENT_
void main() {
    //vec4 color = texture2D(fft, gl_TexCoord[0].st);
    gl_FragColor = vec4(0.4, 0.1, 0.5, 1.0);//color;
}
#endif
