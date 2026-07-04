typedef struct { float w, x, y, z; } Vec4;
typedef struct Vec3 {
    float x, y, z;
    
	Vec3 operator-() const {
        return Vec3{-x, -y, -z};
    }
	
    Vec3 operator-(const Vec3& v) const {
        return {x - v.x, y - v.y, z - v.z};
    }
    
    Vec3 operator+(const Vec3& v) const {
        return {x + v.x, y + v.y, z + v.z};
    }
    
    Vec3 operator*(float s) const {
        return {x * s, y * s, z * s};
    }
};
typedef struct { float x, y; } Vec2;
typedef struct {
    float x;
    float y;
    float z;
	float oldx;
	float oldy;
	float oldz;
	float vx;
	float vy;
	float vz;
    float mass;
	float velocity;
    bool exists;
    int group;
	int attribute;
	float accX, accY, accZ;
	bool collide;
	float friction;
} point;
typedef struct {
	float x;
	float y;
	float z;
	float nx;
	float ny;
	float nz;
	int fyz_ref_points[4];
	Vec3 fyz_ref_offsets[4];
	float weights[6];
	float height_offset;
	float blend_factor;
	int group;
	float u;
	float v;
	Vec3 relative_offset;  // Offset from reference point in local space
} car_vertex;
typedef struct {
	float x;
	float y;
	float z;
    float nx;
    float ny;
    float nz;
	float u;
	float v;
} vertex;
typedef struct {
	int vertices[3];
	float nx;
	float ny;
	float nz;
	int group;
} face;
typedef struct {
    int p1;
    int p2;
    int group;
    float stiffness;
	float default_stiffness;
    float damping;
    float min_len;
    float elastic_margin;
	float rest_len;
	float default_rest_len;
	int attribute;
    bool exists;
	bool clamped;
	bool snap;
	bool snapped;
	float hydro_filtered = 0.0f;
} joint;
typedef struct {
	int p;
	float radius;
	int target;
} ball;
typedef struct {
    int idx[4];
	int group;
} tetrahedron;
typedef struct {
    Vec3 normal;
    int pt_idx;
} HullPlane;
typedef struct {
    int point_indices[MAX_PARTS];
    int point_count = 0;
    HullPlane planes[MAX_PARTS];
    int plane_count = 0;
} convex_hull;
typedef struct {
	point points[MAX_PARTS];
	joint joints[MAX_PARTS];
	face col_faces[MAX_PARTS];
	ball balls[MAX_PARTS];
	car_vertex vertices[CAR_MAX_VERTICES];
	face faces[CAR_MAX_POLY];
	int faces_count;
	int vertices_count;
	bool outlines;
	int crs;
	float pos_x;
	float pos_y;
	float pos_z;
	float oldx;
	float oldy;
	float oldz;
	float velocity;
	float rot_x;
	float rot_y;
	float rot_z;
	float volant_pos;
	float engine_force;
	float breaking_score;
	int points_count;
	int joints_count;
	int col_faces_count;
	int balls_count;
	int wheel_LB;
	int wheel_RB;
	int wheel_LF;
	int wheel_RF;
	GLuint texture;
	GLuint roughness;
	GLuint specular;
	GLuint metallic;
	GLuint normal;
	int deformation_system;
	float sound_divider;
	tetrahedron* tetrahedra;
	int tetrahedra_count;
	int deformation_method;

	float engine_power=200.0f;
	int steering_type=0;
	float max_steer=0.2f;
	float steer_speed=1.0f;
	char name[64];
	float niceni=0.0;
	GLuint vbo;
	GLuint vao;
	Vec3 v3rd={-3.5, 1.5};
	Vec3 vcab={0.5, 0.2};
	int spawn_type=0;
	int up_point=0;
	Vec3 volume_poses[MAX_PARTS];
	int volumes_count;
	convex_hull* hulls;
	int hulls_count=0;
	float bound;
	float force_clamp=50000.0f;
	float x_shift=0.0f;
	float z_shift=0.0f;
} cardata;
typedef struct {
	float sx;
	float sy;
	float sz;
	float ex;
	float ey;
	float ez;
	float rx;
	float ry;
	float rz;

	float cx, cy, cz;
    float hx, hy, hz;
    float cosY, sinY, cosX, sinX, cosZ, sinZ;

	float friction;

	
	float m0, m1, m2;
    float m3, m4, m5;
    float m6, m7, m8;

	float im0, im1, im2;
    float im3, im4, im5;
    float im6, im7, im8;

    float max_r;
} collision_box;
struct ObjVertex { float x,y,z; };
struct ObjUV     { float u,v; };
struct ObjNormal { float x,y,z; };
struct ObjFace {
    int v[3];
    int vt[3];
    int vn[3];
};
typedef struct {
	int x;
	int y;
	float h;
	int group;
} roadpoint;
typedef struct {
	int p1;
	int p2;
	float width;
	int subdivisions;
} roadpart;
struct RoadDebugTriangle {
    Vec3 a;
    Vec3 b;
    Vec3 c;
};

struct RoadDebugSegment {
    Vec3 a;
    Vec3 b;
};

typedef struct {
	collision_box collision_boxes[500];
	std::vector<int> objects;
	std::vector<Vec3> object_poses;
	int collision_boxes_count;
	std::vector<std::vector<float>> heightmap;
	std::vector<std::vector<uint8_t>> heightmap_tex;
	int magic = 0xDEADBEEF;
	bool loaded;
	bool generated;
	bool permanent;
	int pres;
} chunk;
struct TerrainVertex {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};
struct TerrainChunk {
    GLuint vbo;
    GLuint ebo;
    GLsizei indexCount;
};
TerrainChunk terrainChunks[10];
typedef struct {
	std::vector<ObjVertex> vertices;
	std::vector<ObjNormal> normals;
	std::vector<ObjUV> uvs;
	std::vector<ObjFace> faces;
	unsigned int texture;
	unsigned int texture_type;
	float x;
	float y;
	float z;
} mesh;
struct MeshBuffer {
    GLuint vbo = 0;
    GLuint vao = 0;
    GLuint ebo = 0;
    GLsizei indexCount = 0;
    GLuint textureID = 0;
    GLuint roughnessID = 0;
    GLuint metallicID = 0;
    GLuint specularID = 0;
    GLuint normalID = 0;
	GLuint vertexCount = 0; //not used
};
struct RoadTarget {
    Vec3 start;
    Vec3 end;
};
typedef struct {
    int type;
    std::vector<float> data;
} debug_obj;
struct Candidate {
	Vec3 pos;
	float last_angle;
	float total_score;
	std::vector<Vec3> path; // steps taken so far in this lookahead
};
typedef struct {
    std::string path;              // Cesta ke složce
    std::string name;              // Název auta / kategorie
    SDL_Surface* icon = nullptr;   // Načtená ikona
    bool is_category = false;      // True, pokud je to kategorie
    
    // Indexy aut, která patří do této kategorie (odkazují do vektoru menu_items)
    std::vector<int> sub_items;
} MenuNode;