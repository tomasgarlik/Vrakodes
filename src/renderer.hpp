#include "VKDR_1.hpp"
#include "VKDR_2.hpp"
void setup_renderer(int width, int height){
    if (renderer==0){
        VKDR1_setup(width, height);
    } else if (renderer=1){
        VKDR2_setup(width, height);
    }
}
void render(){
    log("render");
    if (renderer==0){
        VKDR1_render();
    } else if (renderer==1){
        VKDR2_render();
    }
    log("render done");
}
void get_gl_info(){
    if (renderer==0){
        VKDR1_get_gl_info();
    } else if (renderer==1){
        VKDR2_get_gl_info();
    }
}
void makeStaticBuffers(std::vector<mesh> objects){
    if (renderer==0){
        VKDR1_MakeStaticBuffers(objects);
    } else if (renderer==1){
        VKDR2_MakeStaticBuffers(objects);
    }
}
void create_car_buffers(){
    if (renderer==1){
        VKDR2_CreateCarBuffers();
    }
}