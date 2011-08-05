#version 150 core

uniform mat4 modelviewMatrix;
uniform mat4 projMatrix;

uniform sampler2D fftTex;
uniform float N;
uniform float L;
uniform float D;
uniform float chop;
uniform vec3 eyePos;
uniform vec3 sunPos; // this should be normalized
uniform vec2 grid;
#ifdef _VERTEX_
in vec3 in_Position;
in vec3 in_Normal;
in vec3 in_TexCoord;
out vec3 pass_Normal;
out vec4 pass_WsPos;
void main(void) {

    float tile =0.01;
    float delta =(1.0/N);
    vec3 pos = in_Position;
    float idxX = gl_InstanceID / int(grid.x) - (grid.x / 2.f);
    float idxY = gl_InstanceID % int(grid.y) - (grid.y / 2.f);
    pos.xz += vec2(D*idxX,D*idxY);
    vec2 sampleCoord = pos.xz * tile;
    vec4 offset = texture2D(fftTex, pos.xz * tile);
    pass_WsPos = (vec4(pos,1.0)+offset);


    float p0 = texture2D(fftTex, (pass_WsPos.xz*tile + vec2(0.0, -delta)) ).y;
    float p1 = texture2D(fftTex, (pass_WsPos.xz*tile + vec2(-delta, 0.0) )).y;
    float p2 = texture2D(fftTex, (pass_WsPos.xz*tile + vec2(delta, 0.0) ) ).y;
    float p3 = texture2D(fftTex, (pass_WsPos.xz*tile + vec2(0.0, delta) ) ).y;

    pass_Normal = vec3(p1-p2,2.0*L*delta,p0-p3);

    gl_Position = projMatrix * modelviewMatrix * pass_WsPos;
}

#endif

#ifdef _FRAGMENT_
in vec3 pass_Normal;
in vec4 pass_WsPos;
out vec4 out_Color;

vec4 atmosphere(float y) {
    vec4 c0 = vec4(0.172, 0.290, 0.486, 1.000);
    vec4 c1 = vec4(0.321, 0.482, 0.607, 1.000);
    if(y >= 0.0)
	return mix(c1,c0,y/1000.0);
    else
	return c1;
}

void main() {
    vec4 baseColor = vec4(0.133, 0.411, 0.498, 0.0);
    vec3 norm = normalize(pass_Normal);
    vec3 eyeDir	=  normalize(eyePos - pass_WsPos.yyy);
    vec3 reflDir = reflect((eyePos - pass_WsPos.xyz), norm);
    if(reflDir.y < 0.0) reflDir.y *= reflDir.y;
    reflDir.y = max(0.1, reflDir.y);
    vec4 transColor = vec4(0.0, 0.278, 0.321, 0.0);// / (reflDir.y*2.0); //@todo: sample from skybox

    float cos_angle = dot(norm, eyeDir);
    vec4 waterColor = mix(baseColor, transColor*transColor, cos_angle);
    reflDir = normalize(reflDir);
    vec4 atmoColor = atmosphere(reflDir.y*1000.0);
    out_Color =  mix(waterColor, atmoColor, 0.3);// vec4(0.4, 0.1, 0.5, 1.0);
}
#endif

#ifdef _TESSEVAL_
layout(quads) in;
in vec3 tcPosition[];
out vec3 tePosition;

void main()
{
    float u = gl_TessCoord.x, v = gl_TessCoord.y;
    vec3 a = mix(tcPosition[0], tcPosition[3], u);
    vec3 b = mix(tcPosition[12], tcPosition[15], u);
    tePosition = mix(a, b, v);
    gl_Position = projMatrix * modelviewMatrix * vec4(tePosition, 1);
}
#endif

