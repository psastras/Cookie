uniform sampler2D tex;

#ifdef _VERTEX_
void main(void) {
    gl_TexCoord[0]  = gl_MultiTexCoord0;
    gl_Position = ftransform();
}

#endif

#ifdef _FRAGMENT_
void main() {
    vec4 color = texture2D(tex, gl_TexCoord[0].st);
    gl_FragColor = color;
}
#endif
