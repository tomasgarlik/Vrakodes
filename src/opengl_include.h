#ifdef __APPLE__
    #include <OpenGL/gl.h>
    #include <OpenGL/gl3.h>
#else
    #ifdef _WIN32
        #define GLEW_STATIC
        #include <GL/glew.h>
    #else
        #include <GL/gl.h>
        #include <GL/gl3.h>
        #define GL_GLEXT_PROTOTYPES
        #include <GL/glext.h>
    #endif
#endif