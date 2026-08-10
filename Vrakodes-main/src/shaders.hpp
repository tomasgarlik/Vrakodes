// =============================================================================
// SHADERS — LEVEL 0 (albedo only, no lighting)
// =============================================================================
static const char* v_shader_3d_0 =
"#version 330 core\n"
"layout(location=0) in vec3 p; layout(location=2) in vec2 u; out vec2 uv;\n"
"uniform mat4 m,v,pr;\n"
"void main(){uv=u;gl_Position=pr*v*m*vec4(p,1.0);}\n";

static const char* f_shader_3d_0 =
"#version 330 core\n"
"in vec2 uv; out vec4 color; uniform sampler2D texAlbedo;\n"
"void main(){color=texture(texAlbedo,uv);}\n";

// =============================================================================
// SHADERS — LEVEL 1 VERTEX (shared by both level-1 fragment shaders)
// =============================================================================
static const char* v_shader_3d_1_tmpl = R"(
#version 330 core
layout(location=0) in vec3 p;
layout(location=1) in vec3 n;
layout(location=2) in vec2 u;
uniform mat4 m,v,pr;
%%SHADOW_VERT_UNIFORMS%%
out vec2 uv; out vec3 fragPos; out vec3 normal;
%%SHADOW_VERT_OUTS%%
void main(){
    vec4 worldPos=m*vec4(p,1.0);
    gl_Position=pr*v*worldPos;
    uv=u; fragPos=worldPos.xyz; normal=n;
    %%SHADOW_VERT_MAIN%%
}
)";

// =============================================================================
// SHADERS — LEVEL 1 MAP (Phong + shadows, NO skybox reflections)
// Used for static map geometry.
// =============================================================================
static const char* f_shader_3d_1_map_tmpl = R"(
#version 330 core
in vec2 uv; in vec3 fragPos; in vec3 normal;
%%SHADOW_FRAG_INS%%
uniform sampler2D texAlbedo;
%%SHADOW_FRAG_UNIFORMS%%
uniform vec3 lpos,camPos;
uniform float ambientStrength;
out vec4 color;

%%SHADOW_FUNC%%

void main(){
    vec4 texColor = texture(texAlbedo, uv);
    vec3 albedo = texColor.rgb;
    float alpha = texColor.a;
    if(alpha < 0.01) discard;

    vec3 N = normalize(normal);
    vec3 L = normalize(lpos - fragPos);

    float diff = max(dot(N, L), 0.0);

    float shadow = calculateShadow(%%SHADOW_CALL_ARGS%%, N, L);

    vec3 ambient = albedo * ambientStrength;
    vec3 lighting = ambient + (albedo * diff * 0.7) * shadow;

    color = vec4(lighting, alpha);
}
)";

// =============================================================================
// SHADERS — LEVEL 1 CAR (Phong + shadows + skybox reflections)
// Used for car geometry only.
// =============================================================================
static const char* f_shader_3d_1_car_tmpl = R"(
#version 330 core
in vec2 uv; in vec3 fragPos; in vec3 normal;
%%SHADOW_FRAG_INS%%
uniform sampler2D texAlbedo;
uniform sampler2D texRoughness;
uniform sampler2D texSkybox;
%%SHADOW_FRAG_UNIFORMS%%
uniform vec3 lpos,camPos;
uniform float ambientStrength;
out vec4 color;

%%SHADOW_FUNC%%

void main(){
    vec4 texColor = texture(texAlbedo, uv);
    vec3 albedo = texColor.rgb;
    float alpha = texColor.a;
    if(alpha < 0.01) discard;

    float roughness = texture(texRoughness, uv).r;
    vec3 N = normalize(normal);
    vec3 V = normalize(camPos - fragPos);
    vec3 L = normalize(lpos - fragPos);
    vec3 R = reflect(-V, N);

    float diff = max(dot(N, L), 0.0);

    vec2 skyUV = vec2(atan(R.z, R.x) / 6.2831 + 0.5, acos(clamp(R.y, -1.0, 1.0)) / 3.1415);
    vec3 reflection = texture(texSkybox, skyUV).rgb;

    if(roughness > 0.05) {
        vec3 blurredRef = texture(texSkybox, skyUV + vec2(0.01 * roughness)).rgb;
        blurredRef += texture(texSkybox, skyUV - vec2(0.01 * roughness)).rgb;
        reflection = mix(reflection, blurredRef * 0.5, roughness);
    }

    float specExp = mix(128.0, 2.0, roughness);
    float spec = pow(max(dot(V, reflect(-L, N)), 0.0), specExp);
    if (roughness >= 0.8) spec = 0.0;

    float shadow = calculateShadow(%%SHADOW_CALL_ARGS%%, N, L);

    vec3 ambient = albedo * ambientStrength + (reflection * (1.0 - roughness) * 0.2);
    vec3 lighting = ambient + (albedo * diff * 0.7 + vec3(spec * 0.5)) * shadow;

    color = vec4(lighting, alpha);
}
)";



// =============================================================================
// SHADERS — POST PROCESS
// =============================================================================
static const char* post_process_shader = R"(
#version 330 core
in vec2 uv;
uniform sampler2D hdrBuffer;
uniform float exposure;
out vec4 color;

vec3 aces_tonemap(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

void main() {
    vec3 hdr = texture(hdrBuffer, uv).rgb;
    color = vec4(aces_tonemap(hdr * exposure), 1.0);
}
)";

// =============================================================================
// SHADERS — DEPTH / HUD / SKYBOX / QUAD
// =============================================================================
static const char* v_shader_depth =
"#version 330 core\n"
"layout(location=0) in vec3 p;\n"
"uniform mat4 lightSpaceMatrix,m;\n"
"void main(){gl_Position=lightSpaceMatrix*m*vec4(p,1.0);}\n";
static const char* f_shader_depth =
"#version 330 core\nvoid main(){}\n";

static const char* v_shader_hud =
"#version 330 core\n"
"layout(location=0) in vec2 p;layout(location=1) in vec2 u;out vec2 uv;\n"
"void main(){uv=u;gl_Position=vec4(p,0.0,1.0);}";
static const char* f_shader_hud =
"#version 330 core\n"
"in vec2 uv;out vec4 color;uniform sampler2D tex;\n"
"void main(){color=texture(tex,uv);if(color.a<0.1)discard;}";

static const char* v_shader_skybox = R"(

#version 330 core

// layout(location=0) in vec3 p;layout(location=1) in vec2 u;out vec2 uv;uniform mat4 vp;
// void main(){uv=u;gl_Position=vp*vec4(p,1.0);}

layout(location=0) in vec2 p;
out vec2 screenPos;
void main() {
    screenPos = p;
    gl_Position = vec4(p, 1.0, 1.0); // depth = 1.0, renders behind everything
}

)";

static const char* f_shader_skybox = R"(
#version 330 core
// in vec2 uv;out vec4 color;uniform sampler2D tex;
// void main(){color=texture(tex,uv);}

in vec2 screenPos;
uniform sampler2D texSkybox;
uniform mat4 invVP;
out vec4 color;

void main() {
    vec4 dir = invVP * vec4(screenPos, 1.0, 1.0);
    vec3 d = normalize(dir.xyz);
    vec2 uv = vec2(atan(d.z, d.x) / 6.2831 + 0.5, acos(clamp(d.y, -1.0, 1.0)) / 3.1415);
    uv.x = fract(uv.x);  // <-- this
    color = texture(texSkybox, uv);
}

)";

static const char* v_shader_quad =
"#version 330 core\n"
"layout(location=0) in vec2 p; layout(location=1) in vec2 u; out vec2 uv;\n"
"void main(){uv=u;gl_Position=vec4(p,0.0,1.0);}\n";

static const char* v_shader_debug = R"(
#version 330 core
layout(location=0) in vec3 pos;
layout(location=1) in vec3 norm;
uniform mat4 mvp;
uniform mat4 model;
out vec3 n;
out vec3 fp;
void main(){
    fp = vec3(model * vec4(pos,1.0));
    n = normalize(mat3(transpose(inverse(model))) * norm);
    gl_Position = mvp * vec4(pos,1.0);
}
)";

static const char* f_shader_debug = R"(
#version 330 core
in vec3 n; in vec3 fp;
uniform vec3 col;
uniform vec3 lpos;
out vec4 o;
void main(){
    float d = max(dot(normalize(n), normalize(lpos-fp)), 0.0);
    o = vec4(col * (0.3 + 0.7*d), 1.0);
}
)";