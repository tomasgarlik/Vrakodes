#ifdef __APPLE__
    #include <OpenGL/gl.h>
    #include <OpenGL/gl3.h>
#else
    #ifdef _WIN32
        #define GLEW_STATIC
        #include <GL/glew.h>
    #else
        // Společné pro Linux i Raspberry Pi
        #define GLEW_STATIC // Pokud linkuješ staticky, jinak zakomentuj
        #include <GL/glew.h>
    #endif
#endif
