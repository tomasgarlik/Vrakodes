#ifndef VKDR_2_HPP
#define VKDR_2_HPP

#include <SDL2/SDL.h>
#include "opengl_include.h"
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <iostream>
#ifndef INCLUDES
#define INCLUDES
#include <SDL_opengl.h>
#include <SDL_video.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cstdio>
#include <cstring>
#include <SDL.h>
#include <time.h>
#include <math.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include "constants.h"
#include "general_functions.hpp"
#include "types.hpp"
#include "global_vars.hpp"
#include "renderer.hpp"
#endif

// =============================================================================
// SHADOW SETTINGS
// SHADOW_ENABLED       1 = shadows on, 0 = compile them out entirely.
// SHADOW_PCF_RADIUS    kernel half-size: 1=3x3  2=5x5  3=7x7
// SHADOW_GAUSSIAN_SIGMA blur falloff: 0.5=tight ... 3.0=very soft.
// SHADOW_SOFT_TAPS     0 = one raw depth fetch per tap (fast, blocky on GT750M)
//                      1 = 4-sample manual bilinear per tap — smooth on every GPU
// =============================================================================
#define SHADOW_ENABLED          1
#define SHADOW_PCF_RADIUS       1
#define SHADOW_GAUSSIAN_SIGMA   1.5f
#define SHADOW_SOFT_TAPS        1

// --- STRUCTURES ---
struct Mat4 { float m[16]; };
Vec3 lpos = {10.0f, 1000.0f, 10.0f};

// --- MATH ---
inline Mat4 m_identity(){ Mat4 r={0}; r.m[0]=1;r.m[5]=1;r.m[10]=1;r.m[15]=1; return r; }
inline Mat4 m_mul(const Mat4& a,const Mat4& b){
    Mat4 r={0};
    for(int i=0;i<4;i++) for(int j=0;j<4;j++) for(int k=0;k<4;k++)
        r.m[j*4+i]+=a.m[k*4+i]*b.m[j*4+k];
    return r;
}
inline Mat4 m_perspective(float fov,float asp,float n,float f){
    Mat4 r={0}; float t=tanf(fov*0.5f*PI/180.0f);
    r.m[0]=1/(asp*t); r.m[5]=1/t; r.m[10]=(f+n)/(n-f); r.m[11]=-1; r.m[14]=2*f*n/(n-f);
    return r;
}
inline Mat4 m_translate(float x,float y,float z){ Mat4 r=m_identity(); r.m[12]=x;r.m[13]=y;r.m[14]=z; return r; }
inline Mat4 m_rotateX(float a){
    Mat4 r=m_identity(); float rad=a*PI/180.0f;
    r.m[5]=cosf(rad);r.m[6]=sinf(rad);r.m[9]=-sinf(rad);r.m[10]=cosf(rad); return r;
}
inline Mat4 m_rotateY(float a){
    Mat4 r=m_identity(); float rad=a*PI/180.0f;
    r.m[0]=cosf(rad);r.m[2]=-sinf(rad);r.m[8]=sinf(rad);r.m[10]=cosf(rad); return r;
}
inline Mat4 m_ortho(float l,float r,float b,float t,float n,float f){
    Mat4 m={0};
    m.m[0]=2/(r-l); m.m[5]=2/(t-b); m.m[10]=-2/(f-n);
    m.m[12]=-(r+l)/(r-l); m.m[13]=-(t+b)/(t-b); m.m[14]=-(f+n)/(f-n); m.m[15]=1;
    return m;
}
inline Mat4 m_lookAt(Vec3 eye,Vec3 center,Vec3 up){
    Vec3 f=normalize(center-eye),s=normalize(cross(f,up)),u=cross(s,f);
    Mat4 r=m_identity();
    r.m[0]=s.x;r.m[4]=s.y;r.m[8]=s.z;
    r.m[1]=u.x;r.m[5]=u.y;r.m[9]=u.z;
    r.m[2]=-f.x;r.m[6]=-f.y;r.m[10]=-f.z;
    r.m[12]=-dot(s,eye); r.m[13]=-dot(u,eye); r.m[14]=dot(f,eye);
    return r;
}

// --- SHADOW MAP GLOBALS ---
static GLuint fboShadow=0, texShadow=0;
static Mat4 cachedLightSpaceMatrix;

// --- HDR POST PROCESS GLOBALS ---
static GLuint fboHDR=0, texHDR=0, rboHDRDepth=0;
float exposure = 1.0f;

// =============================================================================
// SHADOW SNIPPET BUILDER
// =============================================================================
struct ShadowSnippets {
    std::string func;
    std::string vertUniforms;
    std::string vertOuts;
    std::string vertMain;
    std::string fragIns;
    std::string fragUniforms;
    std::string callArgs;
};

static ShadowSnippets buildShadowSnippets(int radius,float sigma,bool enabled,bool softTaps){
    ShadowSnippets s;
    if(!enabled){
        s.func     = "float calculateShadow(float d,vec3 N,vec3 L){return 1.0;}\n";
        s.callArgs = "0.0";
        return s;
    }

    s.vertUniforms = "uniform mat4 lightSpaceMatrix;";
    s.vertOuts     = "out vec4 fragPosLightSpace;";
    s.vertMain     = "fragPosLightSpace = lightSpaceMatrix * worldPos;";
    s.fragIns      = "in vec4 fragPosLightSpace;";
    s.fragUniforms = "uniform sampler2D shadowMap;    // unit 4";
    s.callArgs     = "fragPosLightSpace";

    int diam=2*radius+1;
    std::vector<std::vector<float>> w(diam,std::vector<float>(diam));
    float sum=0;
    for(int y=-radius;y<=radius;y++)
        for(int x=-radius;x<=radius;x++){
            float v=expf(-(x*x+y*y)/(2.0f*sigma*sigma));
            w[y+radius][x+radius]=v; sum+=v;
        }

    char buf[512];
    s.func  = "float calculateShadow(vec4 fpls,vec3 N,vec3 L){\n";
    s.func += "    vec3 proj=fpls.xyz/fpls.w*0.5+0.5;\n";
    s.func += "    if(proj.z>1.0||proj.x<0.0||proj.x>1.0||proj.y<0.0||proj.y>1.0)return 1.0;\n";
    s.func += "    float cosTheta=clamp(dot(N,L),0.0,1.0);\n";
    s.func += "    float bias=mix(0.008,0.0005,cosTheta);\n";
    s.func += "    vec2 ts=1.0/textureSize(shadowMap,0);\n";
    s.func += "    float shadow=0.0;\n";

    for(int y=-radius;y<=radius;y++) for(int x=-radius;x<=radius;x++){
        float wt=w[y+radius][x+radius]/sum;
        if(softTaps){
            snprintf(buf,sizeof(buf),
                "    {vec2 tc=proj.xy+vec2(%d,%d)*ts;\n"
                "     vec2 f=fract(tc/ts);\n"
                "     float d00=texture(shadowMap,tc).r;\n"
                "     float d10=texture(shadowMap,tc+vec2(ts.x,0.0)).r;\n"
                "     float d01=texture(shadowMap,tc+vec2(0.0,ts.y)).r;\n"
                "     float d11=texture(shadowMap,tc+ts).r;\n"
                "     float s00=proj.z-bias>d00?0.0:1.0;\n"
                "     float s10=proj.z-bias>d10?0.0:1.0;\n"
                "     float s01=proj.z-bias>d01?0.0:1.0;\n"
                "     float s11=proj.z-bias>d11?0.0:1.0;\n"
                "     shadow+=mix(mix(s00,s10,f.x),mix(s01,s11,f.x),f.y)*%.6ff;}\n",
                x,y,wt);
        } else {
            snprintf(buf,sizeof(buf),
                "    {float d=texture(shadowMap,proj.xy+vec2(%d,%d)*ts).r;\n"
                "     shadow+=(proj.z-bias>d?0.0:1.0)*%.6ff;}\n",
                x,y,wt);
        }
        s.func+=buf;
    }
    s.func += "    return shadow;\n}\n";
    return s;
}

static std::string injectAll(const char* tmpl,const ShadowSnippets& s){
    std::string out(tmpl);
    auto rep=[&](const std::string& tok,const std::string& val){
        size_t pos=0;
        while((pos=out.find(tok,pos))!=std::string::npos){
            out.replace(pos,tok.size(),val);
            pos+=val.size();
        }
    };
    rep("%%SHADOW_FUNC%%",          s.func);
    rep("%%SHADOW_VERT_UNIFORMS%%", s.vertUniforms);
    rep("%%SHADOW_VERT_OUTS%%",     s.vertOuts);
    rep("%%SHADOW_VERT_MAIN%%",     s.vertMain);
    rep("%%SHADOW_FRAG_INS%%",      s.fragIns);
    rep("%%SHADOW_FRAG_UNIFORMS%%", s.fragUniforms);
    rep("%%SHADOW_CALL_ARGS%%",     s.callArgs);
    return out;
}
#include "shaders.hpp"
// =============================================================================
// BUILT SHADER STRINGS (filled at startup)
// =============================================================================
static std::string v_shader_3d_1_built;
static std::string f_shader_3d_1_map_built;   // map: no skybox reflections
static std::string f_shader_3d_1_car_built;   // cars: skybox reflections

// =============================================================================
// RENDERER GLOBALS
// prog3DMap  — level-1 map shader (no reflections)
// prog3DCar  — level-1 car shader (with reflections)
// prog3D0    — level-0 shader (albedo only), used for both if shader_level==0
// =============================================================================
static GLuint prog3DMap=0, prog3DCar=0, prog3D0=0;
static GLuint progHUD=0, progSkybox=0, progDepth=0, progPostProcess=0;
static GLuint hudVAO=0, hudVBO=0;
static GLuint skyboxVAO=0, skyboxVBO=0;
static GLuint quadVAO=0, quadVBO=0;
static GLuint hud_texture_id=0;
static GLuint fallbackTex=0;


// --- static unit geometry VAOs ---
static GLuint dbg_sphere_vao=0,   dbg_sphere_vbo=0,   dbg_sphere_vcount=0;
static GLuint dbg_cylinder_vao=0, dbg_cylinder_vbo=0, dbg_cylinder_vcount=0;
static GLuint dbg_cube_vao=0,     dbg_cube_vbo=0,     dbg_cube_vcount=0;
static GLuint dbg_mesh_vao=0,     dbg_mesh_vbo=0;
static GLuint dbg_prog=0;



// Uniform locations — map program
// Texture units: 0=albedo  1=roughness  4=shadowMap
static GLint uM_map=-1, uV_map=-1, uP_map=-1;
static GLint uLPos_map=-1, uCamPos_map=-1, uAmbientStrength_map=-1;
static GLint uTexAlbedo_map=-1, uTexRoughness_map=-1;
static GLint uLightSpaceMatrix_map=-1, uShadowMap_map=-1;

// Uniform locations — car program
// Texture units: 0=albedo  1=roughness  4=shadowMap  6=skybox
static GLint uM_car=-1, uV_car=-1, uP_car=-1;
static GLint uLPos_car=-1, uCamPos_car=-1, uAmbientStrength_car=-1;
static GLint uTexAlbedo_car=-1, uTexRoughness_car=-1, uTexSkybox_car=-1;
static GLint uLightSpaceMatrix_car=-1, uShadowMap_car=-1;

static GLint uVPSkybox=-1;
static GLint uInvVP_skybox=-1;

// Auto exposure globals
static const int AE_GRID_SIZE = 8;
static GLuint ae_pbo[2] = {0, 0};
static int    ae_pbo_index = 0;
static int    ae_frame_counter = 0;
static int    ae_sample_interval = 8;
static float  ae_target_exposure = 1.0f;
static float  ae_adaptation_speed = 1.5f;

static const int AE_HISTORY_MAX = 2048;
static float ae_history_brightness[AE_HISTORY_MAX] = {};
static float ae_history_time[AE_HISTORY_MAX] = {};
static int   ae_history_index = 0;
static int   ae_history_count = 0;
static float ae_history_seconds = 3.0f;

// =============================================================================
// HELPERS
// =============================================================================
static GLuint createProg(const char* vs,const char* fs){
    auto compile=[](GLenum type,const char* src)->GLuint{
        GLuint s=glCreateShader(type);
        glShaderSource(s,1,&src,NULL); glCompileShader(s);
        GLint ok=0; glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
        if(!ok){char log[2048];glGetShaderInfoLog(s,sizeof(log),NULL,log);printf("[SHADER ERROR]\n%s\n",log);}
        return s;
    };
    GLuint v=compile(GL_VERTEX_SHADER,vs),f=compile(GL_FRAGMENT_SHADER,fs);
    GLuint p=glCreateProgram();
    glAttachShader(p,v); glAttachShader(p,f); glLinkProgram(p);
    GLint ok=0; glGetProgramiv(p,GL_LINK_STATUS,&ok);
    if(!ok){char log[1024];glGetProgramInfoLog(p,sizeof(log),NULL,log);printf("[LINK ERROR]\n%s\n",log);}
    glDeleteShader(v); glDeleteShader(f);
    return p;
}

static void initSkyboxMesh(){
    float verts[] = {
    -1, 1,-1,  -1,-1,-1,   1,-1,-1,   1,-1,-1,   1, 1,-1,  -1, 1,-1,
    -1,-1, 1,  -1,-1,-1,  -1, 1,-1,  -1, 1,-1,  -1, 1, 1,  -1,-1, 1,
     1,-1,-1,   1,-1, 1,   1, 1, 1,   1, 1, 1,   1, 1,-1,   1,-1,-1,
    -1,-1, 1,  -1, 1, 1,   1, 1, 1,   1, 1, 1,   1,-1, 1,  -1,-1, 1,
    -1, 1,-1,   1, 1,-1,   1, 1, 1,   1, 1, 1,  -1, 1, 1,  -1, 1,-1,
    -1,-1,-1,  -1,-1, 1,   1,-1,-1,   1,-1,-1,  -1,-1, 1,   1,-1, 1
};
    glGenVertexArrays(1,&skyboxVAO); glGenBuffers(1,&skyboxVBO);
    glBindVertexArray(skyboxVAO); glBindBuffer(GL_ARRAY_BUFFER,skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(verts),verts,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,5*sizeof(float),0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,5*sizeof(float),(void*)(3*sizeof(float))); glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

static void initQuadMesh(){
    float v[]={-1,1,0,1, -1,-1,0,0, 1,1,1,1, 1,-1,1,0};
    glGenVertexArrays(1,&quadVAO); glGenBuffers(1,&quadVBO);
    glBindVertexArray(quadVAO); glBindBuffer(GL_ARRAY_BUFFER,quadVBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(v),v,GL_STATIC_DRAW);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4*sizeof(float),0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)(2*sizeof(float))); glEnableVertexAttribArray(1);
    glBindVertexArray(0);
}

static GLuint createFallbackTex(){
    GLuint tex; glGenTextures(1,&tex); glBindTexture(GL_TEXTURE_2D,tex);
    unsigned char px[8*8*3];
    for(int y=0;y<8;y++) for(int x=0;x<8;x++){
        int i=(y*8+x)*3,t=((x/2)+(y/2))%2;
        px[i]=t?255:0; px[i+1]=0; px[i+2]=t?255:0;
    }
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,8,8,0,GL_RGB,GL_UNSIGNED_BYTE,px);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    return tex;
}

// Cache all uniform locations for a level-1 program.
// Pass a valid uTexSkybox pointer only for the car program.
static void cacheLevel1Uniforms(GLuint prog,
    GLint& uM, GLint& uV, GLint& uP,
    GLint& uLPos, GLint& uCamPos, GLint& uAmbientStrength,
    GLint& uTexAlbedo, GLint& uTexRoughness,
    GLint& uLightSpaceMatrix, GLint& uShadowMap,
    GLint* uTexSkybox = nullptr)
{
    uM                = glGetUniformLocation(prog,"m");
    uV                = glGetUniformLocation(prog,"v");
    uP                = glGetUniformLocation(prog,"pr");
    uLPos             = glGetUniformLocation(prog,"lpos");
    uCamPos           = glGetUniformLocation(prog,"camPos");
    uAmbientStrength  = glGetUniformLocation(prog,"ambientStrength");
    uTexAlbedo        = glGetUniformLocation(prog,"texAlbedo");
    uTexRoughness     = glGetUniformLocation(prog,"texRoughness");
    uLightSpaceMatrix = glGetUniformLocation(prog,"lightSpaceMatrix");
    uShadowMap        = glGetUniformLocation(prog,"shadowMap");
    if(uTexSkybox) *uTexSkybox = glGetUniformLocation(prog,"texSkybox");
}

// =============================================================================
// PUBLIC API
// =============================================================================

void VKDR2_get_gl_info(){
    const char* vendor  =(const char*)glGetString(GL_VENDOR);
    const char* renderer=(const char*)glGetString(GL_RENDERER);
    const char* version =(const char*)glGetString(GL_VERSION);
    if(vendor)   strncpy(info_vendor,   vendor,  sizeof(info_vendor)-1);
    if(renderer) strncpy(info_renderer, renderer,sizeof(info_renderer)-1);
    if(version)  strncpy(info_glversion,version, sizeof(info_glversion)-1);
    strncpy(info_vkdr,"Vrakodes PBR renderer 2.0",sizeof(info_vkdr)-1);
}
inline Mat4 m_inverse(const Mat4& m) {
    float inv[16], det;
    const float* s = m.m;
    inv[0]  =  s[5]*s[10]*s[15] - s[5]*s[11]*s[14] - s[9]*s[6]*s[15] + s[9]*s[7]*s[14] + s[13]*s[6]*s[11] - s[13]*s[7]*s[10];
    inv[4]  = -s[4]*s[10]*s[15] + s[4]*s[11]*s[14] + s[8]*s[6]*s[15] - s[8]*s[7]*s[14] - s[12]*s[6]*s[11] + s[12]*s[7]*s[10];
    inv[8]  =  s[4]*s[9]*s[15]  - s[4]*s[11]*s[13] - s[8]*s[5]*s[15] + s[8]*s[7]*s[13] + s[12]*s[5]*s[11] - s[12]*s[7]*s[9];
    inv[12] = -s[4]*s[9]*s[14]  + s[4]*s[10]*s[13] + s[8]*s[5]*s[14] - s[8]*s[6]*s[13] - s[12]*s[5]*s[10] + s[12]*s[6]*s[9];
    inv[1]  = -s[1]*s[10]*s[15] + s[1]*s[11]*s[14] + s[9]*s[2]*s[15] - s[9]*s[3]*s[14] - s[13]*s[2]*s[11] + s[13]*s[3]*s[10];
    inv[5]  =  s[0]*s[10]*s[15] - s[0]*s[11]*s[14] - s[8]*s[2]*s[15] + s[8]*s[3]*s[14] + s[12]*s[2]*s[11] - s[12]*s[3]*s[10];
    inv[9]  = -s[0]*s[9]*s[15]  + s[0]*s[11]*s[13] + s[8]*s[1]*s[15] - s[8]*s[3]*s[13] - s[12]*s[1]*s[11] + s[12]*s[3]*s[9];
    inv[13] =  s[0]*s[9]*s[14]  - s[0]*s[10]*s[13] - s[8]*s[1]*s[14] + s[8]*s[2]*s[13] + s[12]*s[1]*s[10] - s[12]*s[2]*s[9];
    inv[2]  =  s[1]*s[6]*s[15]  - s[1]*s[7]*s[14]  - s[5]*s[2]*s[15] + s[5]*s[3]*s[14] + s[13]*s[2]*s[7]  - s[13]*s[3]*s[6];
    inv[6]  = -s[0]*s[6]*s[15]  + s[0]*s[7]*s[14]  + s[4]*s[2]*s[15] - s[4]*s[3]*s[14] - s[12]*s[2]*s[7]  + s[12]*s[3]*s[6];
    inv[10] =  s[0]*s[5]*s[15]  - s[0]*s[7]*s[13]  - s[4]*s[1]*s[15] + s[4]*s[3]*s[13] + s[12]*s[1]*s[7]  - s[12]*s[3]*s[5];
    inv[14] = -s[0]*s[5]*s[14]  + s[0]*s[6]*s[13]  + s[4]*s[1]*s[14] - s[4]*s[2]*s[13] - s[12]*s[1]*s[6]  + s[12]*s[2]*s[5];
    inv[3]  = -s[1]*s[6]*s[11]  + s[1]*s[7]*s[10]  + s[5]*s[2]*s[11] - s[5]*s[3]*s[10] - s[9]*s[2]*s[7]   + s[9]*s[3]*s[6];
    inv[7]  =  s[0]*s[6]*s[11]  - s[0]*s[7]*s[10]  - s[4]*s[2]*s[11] + s[4]*s[3]*s[10] + s[8]*s[2]*s[7]   - s[8]*s[3]*s[6];
    inv[11] = -s[0]*s[5]*s[11]  + s[0]*s[7]*s[9]   + s[4]*s[1]*s[11] - s[4]*s[3]*s[9]  - s[8]*s[1]*s[7]   + s[8]*s[3]*s[5];
    inv[15] =  s[0]*s[5]*s[10]  - s[0]*s[6]*s[9]   - s[4]*s[1]*s[10] + s[4]*s[2]*s[9]  + s[8]*s[1]*s[6]   - s[8]*s[2]*s[5];
    det = s[0]*inv[0] + s[1]*inv[4] + s[2]*inv[8] + s[3]*inv[12];
    if(det == 0) return m_identity();
    det = 1.0f / det;
    Mat4 r;
    for(int i = 0; i < 16; i++) r.m[i] = inv[i] * det;
    return r;
}
void debug_init() {
    printf("Initializing debug...\n");
    dbg_prog = createProg(v_shader_debug, f_shader_debug);

    // --- sphere ---
    {
        std::vector<float> v;
        int stacks=8, slices=8;
        for(int i=0;i<stacks;i++){
            float p0=PI*(-0.5f+(float)i/stacks);
            float p1=PI*(-0.5f+(float)(i+1)/stacks);
            for(int j=0;j<slices;j++){
                float t0=2*PI*(float)j/slices;
                float t1=2*PI*(float)(j+1)/slices;
                float q[4][6]={
                    {cosf(p0)*cosf(t0),sinf(p0),cosf(p0)*sinf(t0), cosf(p0)*cosf(t0),sinf(p0),cosf(p0)*sinf(t0)},
                    {cosf(p0)*cosf(t1),sinf(p0),cosf(p0)*sinf(t1), cosf(p0)*cosf(t1),sinf(p0),cosf(p0)*sinf(t1)},
                    {cosf(p1)*cosf(t1),sinf(p1),cosf(p1)*sinf(t1), cosf(p1)*cosf(t1),sinf(p1),cosf(p1)*sinf(t1)},
                    {cosf(p1)*cosf(t0),sinf(p1),cosf(p1)*sinf(t0), cosf(p1)*cosf(t0),sinf(p1),cosf(p1)*sinf(t0)},
                };
                for(int k:{0,1,2,0,2,3}) v.insert(v.end(),q[k],q[k]+6);
            }
        }
        dbg_sphere_vcount=v.size()/6;
        glGenVertexArrays(1,&dbg_sphere_vao); glGenBuffers(1,&dbg_sphere_vbo);
        glBindVertexArray(dbg_sphere_vao);
        glBindBuffer(GL_ARRAY_BUFFER,dbg_sphere_vbo);
        glBufferData(GL_ARRAY_BUFFER,v.size()*4,v.data(),GL_STATIC_DRAW);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,24,(void*)0);  glEnableVertexAttribArray(0);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,24,(void*)12); glEnableVertexAttribArray(1);
    }

    // --- cylinder (unit: base at origin, tip at y=1, radius=1) ---
    {
        std::vector<float> v;
        int slices=8;
        for(int j=0;j<slices;j++){
            float t0=2*PI*(float)j/slices;
            float t1=2*PI*(float)(j+1)/slices;
            float c0=cosf(t0),s0=sinf(t0),c1=cosf(t1),s1=sinf(t1);
            float q[4][6]={
                {c0,0,s0, c0,0,s0},{c1,0,s1, c1,0,s1},
                {c1,1,s1, c1,0,s1},{c0,1,s0, c0,0,s0},
            };
            for(int k:{0,1,2,0,2,3}) v.insert(v.end(),q[k],q[k]+6);
        }
        dbg_cylinder_vcount=v.size()/6;
        glGenVertexArrays(1,&dbg_cylinder_vao); glGenBuffers(1,&dbg_cylinder_vbo);
        glBindVertexArray(dbg_cylinder_vao);
        glBindBuffer(GL_ARRAY_BUFFER,dbg_cylinder_vbo);
        glBufferData(GL_ARRAY_BUFFER,v.size()*4,v.data(),GL_STATIC_DRAW);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,24,(void*)0);  glEnableVertexAttribArray(0);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,24,(void*)12); glEnableVertexAttribArray(1);
    }

    // --- cube ---
    {
        float f[6][4][6]={
            {{-1,-1, 1,0,0,1},{1,-1,1,0,0,1},{1,1,1,0,0,1},{-1,1,1,0,0,1}},
            {{1,-1,-1,0,0,-1},{-1,-1,-1,0,0,-1},{-1,1,-1,0,0,-1},{1,1,-1,0,0,-1}},
            {{-1,-1,-1,-1,0,0},{-1,-1,1,-1,0,0},{-1,1,1,-1,0,0},{-1,1,-1,-1,0,0}},
            {{1,-1,1,1,0,0},{1,-1,-1,1,0,0},{1,1,-1,1,0,0},{1,1,1,1,0,0}},
            {{-1,1,1,0,1,0},{1,1,1,0,1,0},{1,1,-1,0,1,0},{-1,1,-1,0,1,0}},
            {{-1,-1,-1,0,-1,0},{1,-1,-1,0,-1,0},{1,-1,1,0,-1,0},{-1,-1,1,0,-1,0}},
        };
        std::vector<float> v;
        for(auto& face:f) for(int k:{0,1,2,0,2,3}) v.insert(v.end(),face[k],face[k]+6);
        dbg_cube_vcount=v.size()/6;
        glGenVertexArrays(1,&dbg_cube_vao); glGenBuffers(1,&dbg_cube_vbo);
        glBindVertexArray(dbg_cube_vao);
        glBindBuffer(GL_ARRAY_BUFFER,dbg_cube_vbo);
        glBufferData(GL_ARRAY_BUFFER,v.size()*4,v.data(),GL_STATIC_DRAW);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,24,(void*)0);  glEnableVertexAttribArray(0);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,24,(void*)12); glEnableVertexAttribArray(1);
    }

    // --- mesh vao (dynamic, just set up attribs) ---
    glGenVertexArrays(1,&dbg_mesh_vao); glGenBuffers(1,&dbg_mesh_vbo);
    glBindVertexArray(dbg_mesh_vao);
    glBindBuffer(GL_ARRAY_BUFFER,dbg_mesh_vbo);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,24,(void*)0);  glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,24,(void*)12); glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}
void VKDR2_setup(int width,int height){
    printf("[VKDR2] shadow:%s pcfR:%d sigma:%.1f softTaps:%s\n",
        SHADOW_ENABLED?"ON":"OFF", SHADOW_PCF_RADIUS,
        (double)SHADOW_GAUSSIAN_SIGMA, SHADOW_SOFT_TAPS?"ON":"OFF");

    ShadowSnippets snip=buildShadowSnippets(
        SHADOW_PCF_RADIUS,SHADOW_GAUSSIAN_SIGMA,
        SHADOW_ENABLED!=0,SHADOW_SOFT_TAPS!=0);

    // Build all shader variants from templates
    v_shader_3d_1_built     = injectAll(v_shader_3d_1_tmpl,     snip);
    f_shader_3d_1_map_built = injectAll(f_shader_3d_1_map_tmpl, snip);
    f_shader_3d_1_car_built = injectAll(f_shader_3d_1_car_tmpl, snip);

    if(shader_level==0){
        // Level-0: single program for everything
        prog3D0   = createProg(v_shader_3d_0,           f_shader_3d_0);
        prog3DMap = prog3D0;
        prog3DCar = prog3D0;
    } else {
        // Level-1: separate programs — map (no reflection) vs cars (reflection)
        prog3DMap = createProg(v_shader_3d_1_built.c_str(), f_shader_3d_1_map_built.c_str());
        prog3DCar = createProg(v_shader_3d_1_built.c_str(), f_shader_3d_1_car_built.c_str());
    }

    progHUD         = createProg(v_shader_hud,    f_shader_hud);
    progSkybox      = createProg(v_shader_skybox, f_shader_skybox);
    progDepth       = createProg(v_shader_depth,  f_shader_depth);
    progPostProcess = createProg(v_shader_quad,   post_process_shader);

    uVPSkybox = glGetUniformLocation(progSkybox,"vp");
    uInvVP_skybox = glGetUniformLocation(progSkybox,"invVP");

    if(shader_level>=1){
        // Cache + bind texture units for map program
        cacheLevel1Uniforms(prog3DMap,
            uM_map, uV_map, uP_map,
            uLPos_map, uCamPos_map, uAmbientStrength_map,
            uTexAlbedo_map, uTexRoughness_map,
            uLightSpaceMatrix_map, uShadowMap_map);

        glUseProgram(prog3DMap);
        glUniform1i(uTexAlbedo_map,   0);
        if(SHADOW_ENABLED) glUniform1i(uShadowMap_map,4);
        glUseProgram(0);

        // Cache + bind texture units for car program
        cacheLevel1Uniforms(prog3DCar,
            uM_car, uV_car, uP_car,
            uLPos_car, uCamPos_car, uAmbientStrength_car,
            uTexAlbedo_car, uTexRoughness_car,
            uLightSpaceMatrix_car, uShadowMap_car,
            &uTexSkybox_car);

        glUseProgram(prog3DCar);
        glUniform1i(uTexAlbedo_car,   0);
        glUniform1i(uTexRoughness_car,1);
        if(SHADOW_ENABLED) glUniform1i(uShadowMap_car,4);
        glUniform1i(uTexSkybox_car,   6);
        glUseProgram(0);
    }

    fallbackTex=createFallbackTex();

    float hudVerts[]={-1,1,0,0, 1,1,1,0, -1,-1,0,1, 1,-1,1,1};
    glGenVertexArrays(1,&hudVAO); glGenBuffers(1,&hudVBO);
    glBindVertexArray(hudVAO); glBindBuffer(GL_ARRAY_BUFFER,hudVBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(hudVerts),hudVerts,GL_STATIC_DRAW);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4*sizeof(float),0); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)(2*sizeof(float))); glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    initSkyboxMesh();
    initQuadMesh();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    // Shadow map FBO
    glGenFramebuffers(1,&fboShadow); glGenTextures(1,&texShadow);
    glBindTexture(GL_TEXTURE_2D,texShadow);
    glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24,SHADOW_RESOLUTION,SHADOW_RESOLUTION,0,
                 GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_BORDER);
    float bc[]={1,1,1,1}; glTexParameterfv(GL_TEXTURE_2D,GL_TEXTURE_BORDER_COLOR,bc);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_COMPARE_MODE,GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER,fboShadow);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,texShadow,0);
    glDrawBuffer(GL_NONE); glReadBuffer(GL_NONE);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)
        printf("[SHADOW] FBO incomplete!\n");
    else printf("[SHADOW] FBO OK\n");
    glBindFramebuffer(GL_FRAMEBUFFER,0);

    // HDR framebuffer
    glGenFramebuffers(1,&fboHDR); glGenTextures(1,&texHDR);
    glBindTexture(GL_TEXTURE_2D,texHDR);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA16F,width,height,0,GL_RGBA,GL_FLOAT,NULL);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glGenRenderbuffers(1,&rboHDRDepth);
    glBindRenderbuffer(GL_RENDERBUFFER,rboHDRDepth);
    glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT24,width,height);
    glBindFramebuffer(GL_FRAMEBUFFER,fboHDR);
    glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texHDR,0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_RENDERBUFFER,rboHDRDepth);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)
        printf("[HDR] FBO incomplete!\n");
    else printf("[HDR] FBO OK\n");
    glBindFramebuffer(GL_FRAMEBUFFER,0);

    // Auto exposure PBOs
    glGenBuffers(2, ae_pbo);
    for(int i = 0; i < 2; i++){
        glBindBuffer(GL_PIXEL_PACK_BUFFER, ae_pbo[i]);
        glBufferData(GL_PIXEL_PACK_BUFFER, AE_GRID_SIZE*AE_GRID_SIZE*4*sizeof(float), NULL, GL_STREAM_READ);
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);


    debug_init();
}
void __VKDR2_debug_draw(Mat4 vp) {
    glUseProgram(dbg_prog);
    GLint u_mvp   = glGetUniformLocation(dbg_prog,"mvp");
    GLint u_model = glGetUniformLocation(dbg_prog,"model");
    GLint u_col   = glGetUniformLocation(dbg_prog,"col");
    GLint u_lpos  = glGetUniformLocation(dbg_prog,"lpos");
    glUniform3f(u_lpos, lpos.x, lpos.y, lpos.z); // your global lpos

    float col[3]={1,1,1};

    for(auto& obj : debug_data){
        if(obj.type==DEBUG_COLOR){
            col[0]=obj.data[0]; col[1]=obj.data[1]; col[2]=obj.data[2];
            continue;
        }
        glUniform3fv(u_col,1,col);

        if(obj.type==DEBUG_SPHERE){
            float x=obj.data[0],y=obj.data[1],z=obj.data[2],r=obj.data[3];
            Mat4 model=m_mul(m_translate(x,y,z), Mat4{r,0,0,0, 0,r,0,0, 0,0,r,0, 0,0,0,1});
            Mat4 mvp=m_mul(vp,model);
            glUniformMatrix4fv(u_model,1,GL_FALSE,model.m);
            glUniformMatrix4fv(u_mvp,  1,GL_FALSE,mvp.m);
            glBindVertexArray(dbg_sphere_vao);
            glDrawArrays(GL_TRIANGLES,0,dbg_sphere_vcount);

        } else if(obj.type==DEBUG_CYLINDER||obj.type==DEBUG_LINE){
            float x1=obj.data[0],y1=obj.data[1],z1=obj.data[2];
            float x2=obj.data[3],y2=obj.data[4],z2=obj.data[5],r=obj.data[6];
            float dx=x2-x1,dy=y2-y1,dz=z2-z1;
            float len=sqrtf(dx*dx+dy*dy+dz*dz); if(len<0.0001f) continue;
            dx/=len;dy/=len;dz/=len;
            float ux=0,uy=1,uz=0; if(fabs(dy)>0.99f){ux=1;uy=0;uz=0;}
            float rx=dy*uz-dz*uy,ry=dz*ux-dx*uz,rz=dx*uy-dy*ux;
            float rl=sqrtf(rx*rx+ry*ry+rz*rz);rx/=rl;ry/=rl;rz/=rl;
            float ux2=ry*dz-rz*dy,uy2=rz*dx-rx*dz,uz2=rx*dy-ry*dx;
            // col0=right*r, col1=dir*len, col2=up2*r, col3=origin  (column major)
            Mat4 model;
            model.m[0]=rx*r;  model.m[1]=ry*r;  model.m[2]=rz*r;  model.m[3]=0;
            model.m[4]=dx*len;model.m[5]=dy*len;model.m[6]=dz*len;model.m[7]=0;
            model.m[8]=ux2*r; model.m[9]=uy2*r; model.m[10]=uz2*r;model.m[11]=0;
            model.m[12]=x1;   model.m[13]=y1;   model.m[14]=z1;   model.m[15]=1;
            Mat4 mvp=m_mul(vp,model);
            glUniformMatrix4fv(u_model,1,GL_FALSE,model.m);
            glUniformMatrix4fv(u_mvp,  1,GL_FALSE,mvp.m);
            glBindVertexArray(dbg_cylinder_vao);
            glDrawArrays(GL_TRIANGLES,0,dbg_cylinder_vcount);

        } else if(obj.type==DEBUG_CUBE){
            float x=obj.data[0],y=obj.data[1],z=obj.data[2];
            float sx=obj.data[3],sy=obj.data[4],sz=obj.data[5];
            Mat4 model=m_mul(m_translate(x,y,z), Mat4{sx*.5f,0,0,0, 0,sy*.5f,0,0, 0,0,sz*.5f,0, 0,0,0,1});
            Mat4 mvp=m_mul(vp,model);
            glUniformMatrix4fv(u_model,1,GL_FALSE,model.m);
            glUniformMatrix4fv(u_mvp,  1,GL_FALSE,mvp.m);
            glBindVertexArray(dbg_cube_vao);
            glDrawArrays(GL_TRIANGLES,0,dbg_cube_vcount);

        } else if(obj.type==DEBUG_MESH){
            std::vector<float> v;
            for(int t=0;t+8<(int)obj.data.size();t+=9){
                float ax=obj.data[t],ay=obj.data[t+1],az=obj.data[t+2];
                float bx=obj.data[t+3],by=obj.data[t+4],bz=obj.data[t+5];
                float cx=obj.data[t+6],cy=obj.data[t+7],cz=obj.data[t+8];
                float ex=bx-ax,ey=by-ay,ez=bz-az,fx=cx-ax,fy=cy-ay,fz=cz-az;
                float nx=ey*fz-ez*fy,ny=ez*fx-ex*fz,nz=ex*fy-ey*fx;
                float nl=sqrtf(nx*nx+ny*ny+nz*nz);if(nl>0){nx/=nl;ny/=nl;nz/=nl;}
                v.insert(v.end(),{ax,ay,az,nx,ny,nz,bx,by,bz,nx,ny,nz,cx,cy,cz,nx,ny,nz});
            }
            Mat4 model=m_identity();
            Mat4 mvp=m_mul(vp,model);
            glUniformMatrix4fv(u_model,1,GL_FALSE,model.m);
            glUniformMatrix4fv(u_mvp,  1,GL_FALSE,mvp.m);
            glBindVertexArray(dbg_mesh_vao);
            glBindBuffer(GL_ARRAY_BUFFER,dbg_mesh_vbo);
            glBufferData(GL_ARRAY_BUFFER,v.size()*4,v.data(),GL_DYNAMIC_DRAW);
            glDrawArrays(GL_TRIANGLES,0,v.size()/6);
        }
    }
    glBindVertexArray(0);
    debug_data.clear(); // clear every frame
}
void VKDR2_MakeStaticBuffers(std::vector<mesh> objects){
    staticBuffers.clear();
    if(objects.empty()) return;
    std::map<int,std::vector<mesh*>> grouped;
    for(auto &o:objects) grouped[o.texture].push_back(&o);
    for(auto const&[tex,list]:grouped){
        std::vector<float> data; int vc=0;
        for(auto m:list) for(auto &f:m->faces) for(int i=0;i<3;i++){
            data.push_back(m->vertices[f.v[i]].x+m->x);
            data.push_back(m->vertices[f.v[i]].y+m->y);
            data.push_back(m->vertices[f.v[i]].z+m->z);
            if(!m->normals.empty()&&f.vn[i]<(int)m->normals.size()){
                data.push_back(m->normals[f.vn[i]].x);
                data.push_back(m->normals[f.vn[i]].y);
                data.push_back(m->normals[f.vn[i]].z);
            } else { data.push_back(0);data.push_back(1);data.push_back(0); }
            if(!m->uvs.empty()&&f.vt[i]<(int)m->uvs.size()){
                data.push_back(m->uvs[f.vt[i]].u);
                data.push_back(m->uvs[f.vt[i]].v);
            } else { data.push_back(0);data.push_back(0); }
            vc++;
        }
        if(vc==0) continue;
        MeshBuffer b; b.textureID=tex; b.indexCount=vc;
        glGenVertexArrays(1,&b.vao); glGenBuffers(1,&b.vbo);
        glBindVertexArray(b.vao); glBindBuffer(GL_ARRAY_BUFFER,b.vbo);
        glBufferData(GL_ARRAY_BUFFER,data.size()*sizeof(float),data.data(),GL_STATIC_DRAW);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),0); glEnableVertexAttribArray(0);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float))); glEnableVertexAttribArray(1);
        glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(6*sizeof(float))); glEnableVertexAttribArray(2);
        glBindVertexArray(0);
        staticBuffers.push_back(b);
    }
}

void VKDR2_CreateCarBuffers(){
    for(auto &car:cars){
        std::vector<float> vd;
        for(int i=0;i<car.faces_count;i++) for(int j=0;j<3;j++){
            car_vertex &v=car.vertices[car.faces[i].vertices[j]];
            vd.push_back(v.x); vd.push_back(v.y); vd.push_back(v.z);
            vd.push_back(car.faces[i].nx); vd.push_back(car.faces[i].ny); vd.push_back(car.faces[i].nz);
            vd.push_back(v.u); vd.push_back(v.v);
        }
        glGenVertexArrays(1,&car.vao); glGenBuffers(1,&car.vbo);
        glBindVertexArray(car.vao); glBindBuffer(GL_ARRAY_BUFFER,car.vbo);
        glBufferData(GL_ARRAY_BUFFER,vd.size()*sizeof(float),vd.data(),GL_STATIC_DRAW);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),0); glEnableVertexAttribArray(0);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float))); glEnableVertexAttribArray(1);
        glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(6*sizeof(float))); glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }
}

void __VKDR2_UpdateCarBuffers(){
    for(auto &car:cars){
        if(car.vao==0){printf("Warning: car has no buffer!\n");continue;}
        std::vector<float> vd; vd.reserve(car.faces_count*3*8);
        for(int i=0;i<car.faces_count;i++) for(int j=0;j<3;j++){
            car_vertex &v=car.vertices[car.faces[i].vertices[j]];
            vd.push_back(v.x); vd.push_back(v.y); vd.push_back(v.z);
            vd.push_back(car.faces[i].nx); vd.push_back(car.faces[i].ny); vd.push_back(car.faces[i].nz);
            vd.push_back(v.u); vd.push_back(v.v);
        }
        glBindBuffer(GL_ARRAY_BUFFER,car.vbo);
        glBufferSubData(GL_ARRAY_BUFFER,0,vd.size()*sizeof(float),vd.data());
    }
}

void VKDR2_UpdateHUDTexture(SDL_Surface* surf){
    if(!surf) return;
    if(hud_texture_id==0){
        glGenTextures(1,&hud_texture_id); glBindTexture(GL_TEXTURE_2D,hud_texture_id);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    }
    glBindTexture(GL_TEXTURE_2D,hud_texture_id);
    glPixelStorei(GL_UNPACK_ROW_LENGTH,surf->pitch/4);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,surf->w,surf->h,0,GL_BGRA,GL_UNSIGNED_BYTE,surf->pixels);
    glPixelStorei(GL_UNPACK_ROW_LENGTH,0);
}

// Draw all geometry through an arbitrary program (depth pass etc.)
static void drawAllGeometry(GLuint prog){
    GLint locM=glGetUniformLocation(prog,"m");
    for(auto &b:staticBuffers){
        Mat4 model=m_identity(); glUniformMatrix4fv(locM,1,GL_FALSE,model.m);
        glBindVertexArray(b.vao); glDrawArrays(GL_TRIANGLES,0,b.indexCount);
    }
    for(auto &car:cars){
        if(car.vao==0) continue;
        Mat4 model=m_identity(); glUniformMatrix4fv(locM,1,GL_FALSE,model.m);
        glBindVertexArray(car.vao); glDrawArrays(GL_TRIANGLES,0,car.faces_count*3);
    }
}
void rl(std::string text){
    text="[VKDR2] "+text;
    log(text);
}
void VKDR2_render(){
    rl("vkdr2 render");
    if(!progHUD||!progSkybox){rl("proghud or progskybox are cooked");return;};
    rl("matrices");
    float sxpos=(float)((int)(x_pos/shadow_jump))*shadow_jump;
    float sypos=(float)((int)(y_pos/shadow_jump))*shadow_jump;
    float szpos=(float)((int)(z_pos/shadow_jump))*shadow_jump;
    rl("1");
    Mat4 view=m_mul(m_rotateX(-y_rot),m_mul(m_rotateY(x_rot+90.0f),m_translate(-x_pos,-y_pos,-z_pos)));
    Mat4 proj=m_perspective(FOV,(float)WIDTH/HEIGHT,NEAR,FAR_DISTANCE);
    rl("2");

    // cachedLightSpaceMatrix=m_mul(
    //     m_ortho(-shadow_range,shadow_range,-shadow_range,shadow_range,1.0f,1010.0f),
    //     m_lookAt({lpos.x+sxpos, lpos.y, lpos.z+szpos},{sxpos,0.0f,szpos},{0,1,0}));
    float terrain_h = get_heightmap_height(sxpos, szpos);
    rl("3");


    // 2. Nastavíme ořezové roviny ortografické projekce
    // Chceme rozsah od -50 do +200 vzhledem k výšce terénu
    float shadow_near = 1.0f;           // Kousek před světlem, aby se neodřezávalo
    float shadow_far  = 50.0f;          // Celkový rozsah (50 pod + 200 nad = 250)
    rl("4");

    // 3. Světlo musíme posunout vertikálně (Y) tak, aby viselo přesně 200 jednotek NAD terénem.
    // Tím pádem bude svítit dolů a pokryje rozsah do -50 pod terén.
    float light_y_pos = terrain_h + 30.0f;

    cachedLightSpaceMatrix = m_mul(
        m_ortho(-shadow_range, shadow_range, -shadow_range, shadow_range, shadow_near, shadow_far),
        m_lookAt(
            {lpos.x + sxpos, light_y_pos, lpos.z + szpos}, // Pozice světla (posunutá o výšku)
            {sxpos, terrain_h, szpos},                    // Kam se dívá (střed stínu na terénu)
            {0, 1, 0}                                     // Up vektor
        )
    );
    if(!paused){rl("update car buffers");__VKDR2_UpdateCarBuffers();}

    // =========================================================
    // PASS 1: Shadow map
    // =========================================================
    rl("shadow pass");
    if(SHADOW_ENABLED && shadows && shader_level>0){
        glBindFramebuffer(GL_FRAMEBUFFER,fboShadow);
        glViewport(0,0,SHADOW_RESOLUTION,SHADOW_RESOLUTION);
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_POLYGON_OFFSET_FILL); glPolygonOffset(2.0f,4.0f);
        glUseProgram(progDepth);
        glUniformMatrix4fv(glGetUniformLocation(progDepth,"lightSpaceMatrix"),1,GL_FALSE,cachedLightSpaceMatrix.m);
        drawAllGeometry(progDepth);
        glDisable(GL_POLYGON_OFFSET_FILL);
        glBindFramebuffer(GL_FRAMEBUFFER,0);
        glViewport(0,0,WIDTH,HEIGHT);
    }

    // =========================================================
    // PASS 2: Main render — into HDR buffer
    // =========================================================
    rl("main render");
    glBindFramebuffer(GL_FRAMEBUFFER, fboHDR);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    // --- Skybox ---
glDepthMask(GL_FALSE); glDisable(GL_DEPTH_TEST);
glUseProgram(progSkybox);
{
    Mat4 skyView = m_mul(m_rotateX(-y_rot), m_rotateY(x_rot + 90.0f));
    Mat4 skyVP = m_mul(proj, skyView);
    Mat4 invSkyVP = m_inverse(skyVP);
    glUniformMatrix4fv(uInvVP_skybox, 1, GL_FALSE, invSkyVP.m);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textures[4] ? textures[4] : fallbackTex);
    glUniform1i(glGetUniformLocation(progSkybox, "texSkybox"), 0);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);  // quad, not skyboxVAO!
}
    glDepthMask(GL_TRUE); glEnable(GL_DEPTH_TEST);

    // ----------------------------------------------------------
    // MAP pass — prog3DMap
    // No skybox reflections.
    // Texture units: 0=albedo  1=roughness  4=shadowMap
    // ----------------------------------------------------------
    if(shader_level>=1 && !staticBuffers.empty()){
        glUseProgram(prog3DMap);
        glUniformMatrix4fv(uV_map,1,GL_FALSE,view.m);
        glUniformMatrix4fv(uP_map,1,GL_FALSE,proj.m);
        glUniform3f(uLPos_map,  lpos.x,lpos.y,lpos.z);
        glUniform3f(uCamPos_map,x_pos, y_pos, z_pos);
        glUniform1f(uAmbientStrength_map,light_ambient);
        if(SHADOW_ENABLED){
            glUniformMatrix4fv(uLightSpaceMatrix_map,1,GL_FALSE,cachedLightSpaceMatrix.m);
            glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D,texShadow);
        }

        for(auto &b:staticBuffers){
            glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,b.textureID  ?b.textureID  :generic_texture);
            glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D,b.roughnessID?b.roughnessID:generic_texture);
            glBindVertexArray(b.vao);
            Mat4 model=m_identity(); glUniformMatrix4fv(uM_map,1,GL_FALSE,model.m);
            glDrawArrays(GL_TRIANGLES,0,b.indexCount);
        }
    } else if (shader_level==0){
        glUseProgram(prog3D0);
        GLint uM0 = glGetUniformLocation(prog3D0,"m");
        GLint uV0 = glGetUniformLocation(prog3D0,"v");
        GLint uP0 = glGetUniformLocation(prog3D0,"pr");
        glUniformMatrix4fv(uV0,1,GL_FALSE,view.m);
        glUniformMatrix4fv(uP0,1,GL_FALSE,proj.m);
        for(auto &b:staticBuffers){
            glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,b.textureID?b.textureID:generic_texture);
            glUniform1i(glGetUniformLocation(prog3D0,"texAlbedo"),0);
            Mat4 model=m_identity(); glUniformMatrix4fv(uM0,1,GL_FALSE,model.m);
            glBindVertexArray(b.vao); glDrawArrays(GL_TRIANGLES,0,b.indexCount);
        }
    }

    // ----------------------------------------------------------
    // CAR pass — prog3DCar
    // Includes skybox reflections.
    // Texture units: 0=albedo  1=roughness  4=shadowMap  6=skybox
    // ----------------------------------------------------------
    rl("car pass");
    if(shader_level>=1 && (debug == 0 || debug==1)){
        glUseProgram(prog3DCar);
        glUniformMatrix4fv(uV_car,1,GL_FALSE,view.m);
        glUniformMatrix4fv(uP_car,1,GL_FALSE,proj.m);
        glUniform3f(uLPos_car,  lpos.x,lpos.y,lpos.z);
        glUniform3f(uCamPos_car,x_pos, y_pos, z_pos);
        glUniform1f(uAmbientStrength_car,light_ambient);
        if(SHADOW_ENABLED){
            glUniformMatrix4fv(uLightSpaceMatrix_car,1,GL_FALSE,cachedLightSpaceMatrix.m);
            glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D,texShadow);
        }
        // Bind skybox to unit 6
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, textures[4] ? textures[4] : fallbackTex);

        for(auto &car:cars){
            if(car.vao==0) continue;
            glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,car.texture  ?car.texture  :generic_texture);
            glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D,car.roughness?car.roughness:generic_texture);
            glBindVertexArray(car.vao);
            Mat4 model=m_identity(); glUniformMatrix4fv(uM_car,1,GL_FALSE,model.m);
            glDrawArrays(GL_TRIANGLES,0,car.faces_count*3);
        }
    } else if (shader_level==0){
        glUseProgram(prog3D0);
        GLint uM0 = glGetUniformLocation(prog3D0,"m");
        GLint uV0 = glGetUniformLocation(prog3D0,"v");
        GLint uP0 = glGetUniformLocation(prog3D0,"pr");
        glUniformMatrix4fv(uV0,1,GL_FALSE,view.m);
        glUniformMatrix4fv(uP0,1,GL_FALSE,proj.m);
        for(auto &car:cars){
            if(car.vao==0) continue;
            glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,car.texture?car.texture:generic_texture);
            glUniform1i(glGetUniformLocation(prog3D0,"texAlbedo"),0);
            Mat4 model=m_identity(); glUniformMatrix4fv(uM0,1,GL_FALSE,model.m);
            glBindVertexArray(car.vao); glDrawArrays(GL_TRIANGLES,0,car.faces_count*3);
        }
    }

    // =========================================================
    // AUTO EXPOSURE
    // =========================================================
    if(auto_exposure){
        ae_frame_counter++;
        if(ae_frame_counter >= ae_sample_interval){
            ae_frame_counter = 0;

            int read_pbo  = ae_pbo_index;
            int write_pbo = 1 - ae_pbo_index;
            ae_pbo_index  = 1 - ae_pbo_index;

            int x = clamp((int)(WIDTH  * 0.1f), 0, WIDTH  - AE_GRID_SIZE);
            int y = clamp((int)(HEIGHT * 0.1f), 0, HEIGHT - AE_GRID_SIZE);
            int w = clamp((int)(WIDTH  * 0.8f), 1, WIDTH  - x);
            int h = clamp((int)(HEIGHT * 0.8f), 1, HEIGHT - y);

            glBindFramebuffer(GL_FRAMEBUFFER, fboHDR);

            // Kick off async readback — no stall
            glBindBuffer(GL_PIXEL_PACK_BUFFER, ae_pbo[write_pbo]);
            glReadPixels(x, y, AE_GRID_SIZE, AE_GRID_SIZE, GL_RGBA, GL_FLOAT, 0);

            // Read previous frame's result
            glBindBuffer(GL_PIXEL_PACK_BUFFER, ae_pbo[read_pbo]);
            float* pixels = (float*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
            if(pixels){
                const int total = AE_GRID_SIZE * AE_GRID_SIZE;
                float samples[AE_GRID_SIZE * AE_GRID_SIZE];
                for(int i = 0; i < total; i++)
                    samples[i] = pixels[i*4+0]*0.2126f
                               + pixels[i*4+1]*0.7152f
                               + pixels[i*4+2]*0.0722f;

                std::sort(samples, samples + total);
                int lo = (int)(total * 0.2f);
                int hi = (int)(total * 0.8f);
                float avg = 0;
                for(int i = lo; i < hi; i++) avg += samples[i];
                avg /= (float)(hi - lo);

                float now = (float)SDL_GetTicks() / 1000.0f;
                ae_history_brightness[ae_history_index] = avg;
                ae_history_time[ae_history_index]       = now;
                ae_history_index = (ae_history_index + 1) % AE_HISTORY_MAX;
                ae_history_count = std::min(ae_history_count + 1, AE_HISTORY_MAX);

                float smoothed = 0; int valid = 0;
                for(int i = 0; i < ae_history_count; i++){
                    if((now - ae_history_time[i]) <= ae_history_seconds){
                        smoothed += ae_history_brightness[i]; valid++;
                    }
                }
                smoothed = valid > 0 ? smoothed / (float)valid : avg;

                ae_target_exposure = clamp(0.45f / (smoothed + 0.001f), 0.1f, 8.0f);
                glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            }

            glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        float delta = 1.0f / (float)(fps > 0 ? fps : 60);
        exposure += (ae_target_exposure - exposure) * ae_adaptation_speed * delta;
        exposure  = clamp(exposure, 0.1f, 8.0f);
    } else {
        ae_target_exposure = exposure;
        exposure=1.0f;
    }

    // debug pass
    rl("debug pass");
    draw_car_debug();
    __VKDR2_debug_draw(m_mul(proj,view));



    // =========================================================
    // PASS 3: Post process (tonemap HDR -> screen)
    // =========================================================
    rl("post process");
    exposure*=exposure_multiplyer;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glUseProgram(progPostProcess);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texHDR);
    glUniform1i(glGetUniformLocation(progPostProcess,"hdrBuffer"),0);
    glUniform1f(glGetUniformLocation(progPostProcess,"exposure"), exposure);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);
    glEnable(GL_DEPTH_TEST);


    // HUD
    if(hud_surf){
        VKDR2_UpdateHUDTexture(hud_surf);
        glDisable(GL_DEPTH_TEST); glUseProgram(progHUD);
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,hud_texture_id);
        glUniform1i(glGetUniformLocation(progHUD,"tex"),0);
        glBindVertexArray(hudVAO); glDrawArrays(GL_TRIANGLE_STRIP,0,4);
        glEnable(GL_DEPTH_TEST);
    }

    // Shadow map debug overlay
    if(shadowmap_debug){
        glDisable(GL_DEPTH_TEST); glUseProgram(progHUD);
        glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D,texShadow);
        glUniform1i(glGetUniformLocation(progHUD,"tex"),0);
        glBindVertexArray(hudVAO); glDrawArrays(GL_TRIANGLE_STRIP,0,4);
        glEnable(GL_DEPTH_TEST);
    }
}

#endif