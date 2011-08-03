#version 120

uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;
uniform sampler2D fftTex;
uniform float N;
uniform float L;
uniform float chop;
uniform vec3 eyePos;
uniform vec3 sunPos; // this should be normalized

varying vec4 wspos;
varying vec3 normal;

#ifdef _VERTEX_

void main(void) {
    gl_TexCoord[0]  = gl_MultiTexCoord0;
    float delta = 1.0 / N;
    float tile = .1;
    vec2 sampleCoord = gl_Vertex.xz * tile;
    vec4 offset = texture2D(fftTex, gl_Vertex.xz * tile);
    wspos += gl_Vertex+offset;

    float p0 = texture2D(fftTex, (wspos.xz * tile + vec2(0.0, -delta)) ).y;
    float p1 = texture2D(fftTex, (wspos.xz * tile + vec2(-delta, 0.0) )).y;
    float p2 = texture2D(fftTex, (wspos.xz * tile + vec2(delta, 0.0) ) ).y;
    float p3 = texture2D(fftTex, (wspos.xz * tile + vec2(0.0, delta) ) ).y;

    normal = vec3(p1-p2, 2.0*L*delta, p0-p3);

    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * wspos;
}

#endif

#ifdef _FRAGMENT_


void main() {
    vec4 baseColor = vec4(0.07, 0.15, 0.2, 0.0);
    vec3 norm = normalize(normal);
    vec3 eyeDir	=  normalize(eyePos - wspos.xyz);
    vec3 reflDir = reflect(eyeDir, norm);
    if(reflDir.y < 0.0) reflDir.y *= reflDir.y;
    reflDir.y = max(0.1, reflDir.y);
    vec4 atmoColor = vec4(0.22, 0.25, 0.35, 0.0) / (reflDir.y*2.0); //@todo: sample from skybox
    float cos_angle = dot(norm, eyeDir);
    vec4 waterColor = mix(baseColor, atmoColor, abs(cos_angle));

    gl_FragColor = waterColor;// vec4(0.4, 0.1, 0.5, 1.0);
}
#endif
