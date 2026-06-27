typedef void (*funcPtr)(void);
typedef struct
{
    SDL_Texture* textures[20];
    SDL_Texture* title;
    int lenght;
    int xpos;
    int ypos;
    int width;
    bool opened;
    int topwidth;
    int selected;
    bool dashed;
    bool dashable;
    std::string titletype;
    funcPtr functions[20];
} dropmenu;
typedef struct
{
    int xpos;
    int ypos;
    std::string text;
    Uint32 last_time_cursor_blink;
    bool cursor_visible;
    int start_of_selected;
    int end_of_selected;
    bool active;
    int cursorpos;
    int textsize;
    int textwidth;
} textbox;
typedef struct
{
    SDL_Texture* title;
    int xpos;
    int ypos;
    bool checked;
    bool just_changed;
} checkbox;
typedef struct
{
    int xpos;
    int ypos;
    std::string text;
    Uint32 last_time_cursor_blink;
    bool cursor_visible;
    int start_of_selected;
    int end_of_selected;
    bool active;
    int cursorpos;
    int number;
    int size;
    int maxnum;
    bool dashed;
} numberinputbox;
typedef struct {
    float x;
    float y;
    float z;
    float mass;
    bool selected;
    bool exists;
    bool hidden;
    int group;
    int attribute;
    float friction;
    bool collide;
} point;
typedef struct {
    int p1;
    int p2;
    int group;
    float stiffness;
    float damping;
    float min_len;
    float elastic_margin;
    bool selected;
    bool exists;
    bool hidden;
    int snap;
    int attribute;
} joint;
typedef struct {
    int p1;
    int p2;
    int p3;
    bool exists;
    bool hidden;
    bool selected;
    int group;
    int friction;
} face;
typedef struct {
    int p;
    float radius;
    int target;
    bool exists;
} ball;