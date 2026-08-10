#include <iostream>
#include <string>
#include <vector>
#include <locale>
#include <codecvt>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#define KMOD_COMMAND KMOD_GUI
#define STANDARTPICEHEIGHT 25
#define STANDARTBORDER 1
// #define RETINA_DISPLAY
#define SPLASHSCREENTIME 10
#define CURSORPERIOD 500
#define HOVER_DARKEN 0.6
#define TOPPICEHEIGHT 70
#define MAX_PARTS 20000
#define OBJ_POINT_MASS     1.0f
#define OBJ_JOINT_KIND     0
#define OBJ_JOINT_STIFF    50.0f
#define OBJ_JOINT_DAMP     0.8f
#define OBJ_JOINT_MINLEN   0.8f
#define OBJ_JOINT_ELASTIC  0.05f
#include "types.hpp"
#include "topbar_init.hpp"
#include "global_vars.hpp"
#include "general_functions.hpp"
#include "SDL_Addons.hpp"
#include "tinyfiledialogs.hpp"
void replace_text_range(std::string* inputptr, int start, int end, const std::string& replacement) {
    auto chars = utf8_split_graphemes(*inputptr);
    if (start < 0) start = 0;
    if (end > (int)chars.size()) end = (int)chars.size();
    if (start > end) std::swap(start, end);

    std::string result;
    for (int i = 0; i < start; ++i) result += chars[i];
    result += replacement;
    for (int i = end; i < (int)chars.size(); ++i) result += chars[i];

    *inputptr = result;
}
void openfunc()
{
    const char* path = tinyfd_openFileDialog(
        "Load physics mesh",
        "",
        0,
        NULL,
        NULL,
        0
    );

    if (!path) return;

    FILE* f = fopen(path, "r");
    if (!f) return;

    points_count = 0;
    joints_count = 0;

    char line[512];

    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "\"engine_power\"")) {
            sscanf(line, " \"engine_power\": %f,", &engine_power);
        }
        if (strstr(line, "\"steering_type\"")) {
            sscanf(line, " \"steering_type\": %d,", &steering_type);
        }
        if (strstr(line, "\"max_steer\"")) {
            sscanf(line, " \"max_steer\": %f,", &max_steer);
        }
        if (strstr(line, "\"steer_speed\"")) {
            sscanf(line, " \"steer_speed\": %f,", &steer_speed);
        }
        if (strstr(line, "\"deformation_system\"")) {
            sscanf(line, " \"deformation_system\": %d,", &deformation_system);
        }
        if (strstr(line, "\"sound_divider\"")) {
            sscanf(line, " \"sound_divider\": %f", &sound_divider);
        }
        // ---- POINT ----
        if (strstr(line, "\"x\"")) {
            point p;
            p.exists = true;
            p.selected = false;
            p.hidden = false;

            sscanf(line,
                " {\"x\":%f,\"y\":%f,\"z\":%f,\"mass\":%f,\"attribute\":%d,\"exists\":true,\"kind\":%d}",
                &p.x, &p.y, &p.z, &p.mass, &p.attribute, &p.kind
            );

            points[points_count++] = p;
        }

        // ---- JOINT ----
        if (strstr(line, "\"p1\"")) {
            joint j;
            j.exists = true;
            j.selected = false;
            j.hidden = false;

            sscanf(line,
                " {\"p1\":%d,\"p2\":%d,\"kind\":%d,"
                "\"stiffness\":%f,\"damping\":%f,"
                "\"min_len\":%f,\"elastic_margin\":%f,\"attribute\":%d,\"exists\":true}",
                &j.p1,
                &j.p2,
                &j.kind,
                &j.stiffness,
                &j.damping,
                &j.min_len,
                &j.elastic_margin,
                &j.attribute
            );

            joints[joints_count++] = j;

        }

    }
    fclose(f);
}
void newfunc(){
    return;
}
void saveasfunc(){
    const char* path = tinyfd_saveFileDialog(
        "Save physics mesh",
        "vehicle.json",
        0,
        NULL,
        NULL
    );

    if (!path) return;

    FILE* f = fopen(path, "w");
    if (!f) return;

    fprintf(f, "{\n");
    fprintf(f, "  \"properties\": {\n");
    fprintf(f, "    \"engine_power\": %f,\n", engine_power);
    fprintf(f, "    \"steering_type\": %d,\n", steering_type);
    fprintf(f, "    \"max_steer\": %f,\n", max_steer);
    fprintf(f, "    \"steer_speed\": %f,\n", steer_speed);
    fprintf(f, "    \"deformation_system\": %d,\n", deformation_system);
    fprintf(f, "    \"sound_divider\": %f\n", sound_divider);
    fprintf(f, "  },\n");
    // ---- POINTS ----
    fprintf(f, "  \"points\": [\n");
    for (int i = 0; i < points_count; i++) {
        if (!points[i].exists) continue;

        fprintf(f,
            "    {\"x\":%f,\"y\":%f,\"z\":%f,\"mass\":%f,\"attribute\":%d,\"exists\":true,\"kind\":%d}",
            points[i].x,
            points[i].y,
            points[i].z,
            points[i].mass,
            points[i].attribute,
            points[i].kind
        );

        fprintf(f, (i < points_count - 1) ? ",\n" : "\n");
    }
    fprintf(f, "  ],\n");

    // ---- JOINTS ----
    fprintf(f, "  \"joints\": [\n");
    for (int i = 0; i < joints_count; i++) {
        if (!joints[i].exists) continue;

        fprintf(f,
            "    {\"p1\":%d,\"p2\":%d,\"kind\":%d,"
            "\"stiffness\":%f,\"damping\":%f,"
            "\"min_len\":%f,\"elastic_margin\":%f,\"attribute\":%d,\"exists\":true}",
            joints[i].p1,
            joints[i].p2,
            joints[i].kind,
            joints[i].stiffness,
            joints[i].damping,
            joints[i].min_len,
            joints[i].elastic_margin,
            joints[i].attribute
        );

        fprintf(f, (i < joints_count - 1) ? ",\n" : "\n");
    }
    fprintf(f, "  ]\n");
    fprintf(f, "}\n");
    fclose(f);
}
bool joint_exists(int a, int b) {
    for (int i = 0; i < joints_count; i++) {
        if (!joints[i].exists) continue;

        if ((joints[i].p1 == a && joints[i].p2 == b) ||
            (joints[i].p1 == b && joints[i].p2 == a)) {
            return true;
        }
    }
    return false;
}
void load_obj() {
    int begin_points=points_count;
    const char* filters[] = { "*.obj" };
    const char* path = tinyfd_openFileDialog(
        "Load OBJ",
        "",
        1,
        filters,
        "Wavefront OBJ",
        0
    );

    if (!path) return;

    FILE* f = fopen(path, "r");
    if (!f) {
        printf("OBJ: failed to open %s\n", path);
        return;
    }



    char line[512];

    while (fgets(line, sizeof(line), f)) {

        // ---------- VERTEX ----------
        if (line[0] == 'v' && line[1] == ' ') {
            point* p = &points[points_count++];

            sscanf(line, "v %f %f %f", &p->x, &p->y, &p->z);

            p->mass     = OBJ_POINT_MASS;
            p->attribute    = 0;
            p->selected = false;
            p->exists   = true;
        }

        // ---------- FACE → EDGES ----------
        else if (line[0] == 'l' && line[1] == ' ') {

            int idx[16];
            int cnt = 0;

            char* tok = strtok(line + 2, " ");
            while (tok && cnt < 16) {
                idx[cnt++] = atoi(tok) - 1; // OBJ je 1-based
                tok = strtok(NULL, " ");
            }

            for (int i = 0; i < cnt; i++) {
                int a = idx[i];
                int b = idx[(i + 1) % cnt];

                if (a < 0 || b < 0) continue;
                if (a == b) continue;

                // ❗ DUPLICITY PRYČ
                // if (joint_exists(a, b)) continue;

                joint* j = &joints[joints_count++];

                j->p1 = a+begin_points;
                j->p2 = b+begin_points;

                j->kind           = OBJ_JOINT_KIND;
                j->stiffness      = OBJ_JOINT_STIFF;
                j->damping        = OBJ_JOINT_DAMP;
                j->min_len        = OBJ_JOINT_MINLEN;
                j->elastic_margin = OBJ_JOINT_ELASTIC;
                j->selected       = false;
                j->exists         = true;
            }
        }
    }

    fclose(f);

    printf("OBJ loaded: %d points, %d joints (deduplicated)\n",
           points_count, joints_count);
}
void quitfunc(){
    SDL_Quit();
    running=false;
}
void mlemfunc(){
    SDL_ShowSimpleMessageBox(
        SDL_MESSAGEBOX_ERROR,
        "Co zloušíš, kebabééé",
        "whomp whomp",
        nullptr
    );
    SDL_ShowSimpleMessageBox(
        SDL_MESSAGEBOX_ERROR,
        "Oh no, it looks like your computer is going to crash in a few seconds",
        "System error: error code 123904948 at 447878:3473847:3489384398 0x122234 0x233332 segmentation fault",
        nullptr
    );
    SDL_Delay(3000);
    SDL_Event event3;
    SDL_Rect rect3;
    SDL_Window* splashscreen2 = SDL_CreateWindow("Splash Screen",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 500, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);
    SDL_Renderer* renderer1 = SDL_CreateRenderer(splashscreen2, -1, SDL_RENDERER_ACCELERATED);
    SDL_ShowCursor(SDL_DISABLE);
    SDL_RenderClear(renderer1);
    SDL_PollEvent(&event3);
    SDL_RenderPresent(renderer1);
    SDL_PollEvent(&event3);
    SDL_Delay(2000);
    SDL_Texture* texture3 = loadTexture("icons/crashscreen.png", renderer1);
    SDL_RenderClear(renderer1);
    SDL_RenderCopy(renderer1, texture3, NULL, NULL);
    SDL_RenderPresent(renderer1);
    running=true;

    while (running) {
        SDL_PollEvent(&event3);
    }
    SDL_DestroyTexture(texture3);
    SDL_DestroyRenderer(renderer1);
    SDL_DestroyWindow(splashscreen2);
}
void updateDropMenu(SDL_Renderer* renderer, dropmenu* menuptr){
    dropmenu menu=*menuptr;
    if (!something_selected && menu.dashable){
        menu.dashed=true;
    }
    SDL_Rect rect={pxlenght(menu.xpos),pxlenght(menu.ypos),pxlenght(menu.topwidth), pxlenght(STANDARTPICEHEIGHT)};
    SDL_SetRenderDrawColor(renderer, 240, 240, 240,255);
    SDL_RenderFillRoundRect(renderer, &rect);
    if (menu.opened){
        SDL_Rect rect={pxlenght(menu.xpos),pxlenght(menu.ypos),pxlenght(menu.topwidth), pxlenght(STANDARTPICEHEIGHT)};
        SDL_SetRenderDrawColor(renderer, 240, 240, 240,255);
        SDL_RenderFillRoundRect(renderer, &rect);
        SDL_Rect rect0={pxlenght(menu.xpos),pxlenght(menu.ypos+STANDARTPICEHEIGHT),pxlenght(menu.width), pxlenght(STANDARTPICEHEIGHT*menu.lenght)};
        SDL_SetRenderDrawColor(renderer, 240, 240, 240,255);
        SDL_RenderFillRoundRect(renderer, &rect0);
        SDL_Rect rect1={pxlenght(menu.xpos+STANDARTBORDER),pxlenght(menu.ypos+STANDARTPICEHEIGHT+STANDARTBORDER),pxlenght(menu.width-STANDARTBORDER*2), pxlenght(STANDARTPICEHEIGHT*menu.lenght-STANDARTBORDER*2)};
        SDL_SetRenderDrawColor(renderer, 30, 30, 45,255);
        SDL_RenderFillRoundRect(renderer, &rect1);
        for (int i=0;i<menu.lenght;i++){
            if (mousex<pxlenght(menu.xpos+menu.width) && mousex>pxlenght(menu.xpos) && mousey<pxlenght(menu.ypos+STANDARTPICEHEIGHT*(i+2))){
                SDL_Rect rect1={pxlenght(menu.xpos+STANDARTBORDER),pxlenght(menu.ypos+STANDARTPICEHEIGHT*(i+1)+STANDARTBORDER),pxlenght(menu.width-STANDARTBORDER*2), pxlenght(STANDARTPICEHEIGHT-STANDARTBORDER*2)};
                SDL_SetRenderDrawColor(renderer, 70,80,240,255);
                if (clicked){SDL_SetRenderDrawColor(renderer, 80,80,80,255);}
                SDL_RenderFillRoundRect(renderer, &rect1);
                if (clickup){
                    menu.opened=false;
                    menu.functions[i]();
                    menu.selected=i;
                }
                break;
            }
        }
        for (int i=0;i<menu.lenght;i++){
            displayTex(renderer, menu.textures[i], pxlenght(menu.xpos+STANDARTBORDER*2), pxlenght(menu.ypos+STANDARTPICEHEIGHT*(i+1)+STANDARTBORDER), pxlenght(STANDARTPICEHEIGHT-STANDARTBORDER*2));
        }
        clickedOnSomething=true;
    }
    if (menu.opened && mousex<pxlenght(menu.xpos+menu.width) && mousex>pxlenght(menu.xpos) && mousey<pxlenght(menu.ypos+STANDARTPICEHEIGHT*(menu.lenght+1))){menu.opened=true;clickedOnSomething=true;}
    else if (clicked && menu.opened && !(mousex<pxlenght(menu.xpos+menu.width) && mousex>pxlenght(menu.xpos) && mousey<pxlenght(menu.ypos+STANDARTPICEHEIGHT*(menu.lenght+1)))){menu.opened=false;clickedOnSomething=true;}
    if (mousex<pxlenght(menu.topwidth)+pxlenght(menu.xpos) && mousex>pxlenght(menu.xpos) && mousey>pxlenght(menu.ypos) && mousey<pxlenght(menu.ypos+STANDARTPICEHEIGHT)){
        SDL_Rect rect={pxlenght(menu.xpos),pxlenght(menu.ypos),pxlenght(menu.topwidth), pxlenght(STANDARTPICEHEIGHT)};
        if (clicked){SDL_SetRenderDrawColor(renderer, 10,10,10,255);clickedOnSomething=true;}
        else {SDL_SetRenderDrawColor(renderer, 20, 20, 35,255);}
        SDL_RenderFillRoundRect(renderer, &rect);
        if (clickup && (something_selected || !menu.dashable)){
            menu.opened=true;
            menu.dashed=false;
            clickedOnSomething=true;
        }
    } else {
        SDL_Rect rect={pxlenght(menu.xpos+STANDARTBORDER),pxlenght(menu.ypos+STANDARTBORDER),pxlenght(menu.topwidth-STANDARTBORDER*2), pxlenght(STANDARTPICEHEIGHT-STANDARTBORDER*2)};
        SDL_SetRenderDrawColor(renderer, 30, 30, 45,255);
        SDL_RenderFillRoundRect(renderer, &rect);
    }
    if ((!menu.dashed)){
        if (menu.titletype=="defined"){
            displayTex(renderer,menu.title,pxlenght(menu.xpos+STANDARTBORDER*2),pxlenght(menu.ypos+STANDARTBORDER),pxlenght(STANDARTPICEHEIGHT-STANDARTBORDER*2));
        } else if (menu.titletype=="select"){
            displayTex(renderer,menu.textures[menu.selected],pxlenght(menu.xpos+STANDARTBORDER*2),pxlenght(menu.ypos+STANDARTBORDER),pxlenght(STANDARTPICEHEIGHT-STANDARTBORDER*2));
        }
    } else {
        displayTex(renderer,createTextTexture(renderer, "---"),pxlenght(menu.xpos+STANDARTBORDER*2),pxlenght(menu.ypos+STANDARTBORDER),pxlenght(STANDARTPICEHEIGHT-STANDARTBORDER*2));
    }
    *menuptr=menu;
    return;

}
void updateTextBox(SDL_Renderer* renderer, textbox* inputboxptr){
    textbox inputbox=*inputboxptr;
    std::string finaltext=inputbox.text;
    SDL_Texture* texttex=createTextTexture(renderer,finaltext, inputbox.start_of_selected, inputbox.end_of_selected);
    SDL_Rect rect={pxlenght(inputbox.xpos-(getTextureWidth(texttex,inputbox.textsize)+STANDARTBORDER*2)/2), pxlenght(inputbox.ypos), pxlenght((getTextureWidth(texttex,inputbox.textsize)+STANDARTBORDER*2)), pxlenght(inputbox.textsize+STANDARTBORDER*2)};
    inputbox.textwidth=getTextureWidth(texttex,inputbox.textsize)+STANDARTBORDER*2;
    if (inputbox.active){
        if (SDL_GetTicks()-inputbox.last_time_cursor_blink>CURSORPERIOD){
            inputbox.cursor_visible=!inputbox.cursor_visible;
            inputbox.last_time_cursor_blink=SDL_GetTicks();
        }
        SDL_SetRenderDrawColor(renderer, 50,50,50,255);
        SDL_RenderFillRoundRect(renderer, &rect);
        SDL_Rect rect2={pxlenght(inputbox.xpos-(getTextureWidth(texttex,inputbox.textsize)+STANDARTBORDER*2)/2+STANDARTBORDER), pxlenght(inputbox.ypos+STANDARTBORDER), pxlenght((getTextureWidth(texttex,inputbox.textsize)+STANDARTBORDER*2)-STANDARTBORDER*2), pxlenght(inputbox.textsize)};
        SDL_SetRenderDrawColor(renderer, 30, 30, 45,255);
        SDL_RenderFillRoundRect(renderer, &rect2, pxlenght(3));
        SDL_SetRenderDrawColor(renderer, 0,0,0,255);
        if (inputbox.cursorpos > stringlen(inputbox.text)) inputbox.cursorpos = stringlen(inputbox.text);
        if (inputbox.cursorpos < 0) inputbox.cursorpos = 0;
        int cursorlenpx=(int)((*(proportions+inputbox.cursorpos))*(rect.w-STANDARTBORDER))+STANDARTBORDER;
        if (stringlen(typedChar)!=0){
            if (inputbox.start_of_selected==-1 && inputbox.end_of_selected==-1){
                auto graphemes = utf8_split_graphemes(inputbox.text);
                graphemes.insert(graphemes.begin() + inputbox.cursorpos, typedChar);
                inputbox.text.clear();
                for (const auto& g : graphemes) inputbox.text += g;
                inputbox.cursorpos+=stringlen(typedChar);
            } else {
                replace_text_range(&inputbox.text, inputbox.start_of_selected, inputbox.end_of_selected, typedChar);
                inputbox.cursorpos=inputbox.start_of_selected+1;
                inputbox.start_of_selected=-1;
                inputbox.end_of_selected=-1;
            }
            inputbox.cursor_visible=true;
        }
        if (modkey & KMOD_SHIFT){
            if (specialkey=='l' && inputbox.cursorpos>0){
                if (inputbox.start_of_selected==-1 && inputbox.end_of_selected==-1){
                    inputbox.start_of_selected=inputbox.cursorpos-1;
                    inputbox.end_of_selected=inputbox.cursorpos;
                } else {
                    if (inputbox.cursorpos==inputbox.start_of_selected){
                        inputbox.start_of_selected--;
                    } else {
                        inputbox.end_of_selected--;
                    }
                }
                inputbox.cursorpos--;
                inputbox.cursor_visible=true;
            } else if (specialkey=='r' && inputbox.cursorpos<stringlen(inputbox.text)){
                if (inputbox.start_of_selected==-1 && inputbox.end_of_selected==-1){
                    inputbox.start_of_selected=inputbox.cursorpos;
                    inputbox.end_of_selected=inputbox.cursorpos+1;
                } else {
                    if (inputbox.cursorpos==inputbox.start_of_selected){
                        inputbox.start_of_selected++;
                    } else {
                        inputbox.end_of_selected++;
                    }
                }
                inputbox.cursorpos++;
                inputbox.cursor_visible=true;
            } else if (specialkey=='b'){
                inputbox.text="";
                inputbox.cursorpos=0;
                inputbox.cursor_visible=true;
            }
        } else {
            if (specialkey=='l' && inputbox.cursorpos>0){
                inputbox.cursorpos-=1;
                inputbox.start_of_selected=-1;
                inputbox.end_of_selected=-1;
                inputbox.cursor_visible=true;
            } else if (specialkey=='r' && inputbox.cursorpos<stringlen(inputbox.text)){
                inputbox.cursorpos+=1;
                inputbox.start_of_selected=-1;
                inputbox.end_of_selected=-1;
                inputbox.cursor_visible=true;
            } else if (specialkey=='b'){
                if (inputbox.start_of_selected==-1 && inputbox.end_of_selected==-1){
                    replace_text_range(&inputbox.text, inputbox.cursorpos-1, inputbox.cursorpos, "");
                } else {
                    replace_text_range(&inputbox.text, inputbox.start_of_selected, inputbox.end_of_selected, "");
                    inputbox.start_of_selected=-1;
                    inputbox.end_of_selected=-1;
                }
                inputbox.cursorpos--;
                inputbox.cursor_visible=true;
            }
        }
        if (inputbox.cursor_visible){SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255); SDL_RenderDrawFatLine(renderer,pxlenght(inputbox.xpos-(getTextureWidth(texttex,inputbox.textsize)+STANDARTBORDER*2)/2)+cursorlenpx, pxlenght(inputbox.ypos+STANDARTBORDER), pxlenght(inputbox.xpos-(getTextureWidth(texttex,inputbox.textsize)+STANDARTBORDER*2)/2)+cursorlenpx, pxlenght(inputbox.ypos+inputbox.textsize+STANDARTBORDER), pxlenght(3));}
    }
    if (clic){
        SDL_Point mousepoint={mousex,mousey};
        if (SDL_PointInRect(&mousepoint,&rect)){
            if (inputbox.active){
                inputbox.start_of_selected=-1;
                inputbox.end_of_selected=-1;
                for (i = 0; i < stringlen(inputbox.text) && mousex - rect.x >= (int)((*(proportions + i)) * (float)(rect.w - STANDARTBORDER * 2)); i++){inputbox.cursorpos = i;}
                inputbox.cursorpos=i;
            } else {
                inputbox.start_of_selected=0;
                inputbox.end_of_selected=stringlen(inputbox.text);
            }
            inputbox.active=true;
            clickedOnSomething=true;
        } else {
            inputbox.active=false;
            inputbox.start_of_selected=-1;
            inputbox.end_of_selected=-1;
        }
    }

    if (inputbox.start_of_selected == inputbox.end_of_selected){
        inputbox.start_of_selected=-1;
        inputbox.end_of_selected=-1;
    }
    displayTex(renderer,texttex,pxlenght(inputbox.xpos+STANDARTBORDER-(getTextureWidth(texttex,inputbox.textsize)+STANDARTBORDER*2)/2), pxlenght(inputbox.ypos+STANDARTBORDER), pxlenght(inputbox.textsize));
    SDL_DestroyTexture(texttex);
    *inputboxptr=inputbox;
    return;
}
void updateNumberInputBox(SDL_Renderer* renderer, numberinputbox* inputboxptr){
    numberinputbox inputbox=*inputboxptr;
    if (!something_selected){
        inputbox.dashed=true;
    }
    if (inputbox.dashed){
        inputbox.text="---";
    }
    std::string finaltext=inputbox.text;
    SDL_Texture* texttex=createTextTexture(renderer,finaltext, inputbox.start_of_selected, inputbox.end_of_selected);
    SDL_Rect rect={pxlenght(inputbox.xpos), pxlenght(inputbox.ypos), pxlenght((inputbox.size)), pxlenght(STANDARTPICEHEIGHT)};
    SDL_SetRenderDrawColor(renderer, 240,240,240,255);
    SDL_RenderFillRoundRect(renderer, &rect);
    SDL_Rect rect2={pxlenght(inputbox.xpos+STANDARTBORDER), pxlenght(inputbox.ypos+STANDARTBORDER), pxlenght(inputbox.size-STANDARTBORDER*2), pxlenght(STANDARTPICEHEIGHT-STANDARTBORDER*2)};
    SDL_SetRenderDrawColor(renderer, 30, 30, 45,255);
    SDL_RenderFillRoundRect(renderer, &rect2, pxlenght(3));
    SDL_SetRenderDrawColor(renderer, 240,240,240,255);
    SDL_RenderDrawFatLine(renderer,pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/2), pxlenght(inputbox.ypos+1), pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/2), pxlenght(inputbox.ypos+STANDARTPICEHEIGHT-1), pxlenght(STANDARTBORDER)/2);
    SDL_RenderDrawFatLine(renderer,pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/2), pxlenght(inputbox.ypos+STANDARTPICEHEIGHT/2), pxlenght(inputbox.xpos+inputbox.size), pxlenght(inputbox.ypos+STANDARTPICEHEIGHT/2), pxlenght(STANDARTBORDER)/2);
    SDL_SetRenderDrawColor(renderer, 170,170,170,255);
    SDL_Point mousepoint={mousex,mousey};
    if (SDL_PointInRect(&mousepoint,&rect) && mousex>pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/2)){
        if (mousey>pxlenght(inputbox.ypos+STANDARTPICEHEIGHT/2)){
            SDL_SetRenderDrawColor(renderer, 80,80,80,255);
            SDL_RenderDrawFatLine(renderer,pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/2+3), pxlenght(inputbox.ypos+STANDARTPICEHEIGHT/2+4), pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/4-1), pxlenght(inputbox.ypos+STANDARTPICEHEIGHT-4), pxlenght(STANDARTBORDER));
            SDL_RenderDrawFatLine(renderer,pxlenght(inputbox.xpos+inputbox.size-4), pxlenght(inputbox.ypos+STANDARTPICEHEIGHT/2+4), pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/4-1), pxlenght(inputbox.ypos+STANDARTPICEHEIGHT-4), pxlenght(STANDARTBORDER));
            SDL_SetRenderDrawColor(renderer, 170,170,170,255);
            SDL_RenderDrawFatLine(renderer,pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/2+3), pxlenght(inputbox.ypos+STANDARTPICEHEIGHT/2-4), pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/4-1), pxlenght(inputbox.ypos+4), pxlenght(STANDARTBORDER));
            SDL_RenderDrawFatLine(renderer,pxlenght(inputbox.xpos+inputbox.size-4), pxlenght(inputbox.ypos+STANDARTPICEHEIGHT/2-4), pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/4-1), pxlenght(inputbox.ypos+4), pxlenght(STANDARTBORDER));
        } else {
            SDL_SetRenderDrawColor(renderer, 80,80,80,255);
            SDL_RenderDrawFatLine(renderer,pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/2+3), pxlenght(inputbox.ypos+STANDARTPICEHEIGHT/2-4), pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/4-1), pxlenght(inputbox.ypos+4), pxlenght(STANDARTBORDER));
            SDL_RenderDrawFatLine(renderer,pxlenght(inputbox.xpos+inputbox.size-4), pxlenght(inputbox.ypos+STANDARTPICEHEIGHT/2-4), pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/4-1), pxlenght(inputbox.ypos+4), pxlenght(STANDARTBORDER));
            SDL_SetRenderDrawColor(renderer, 170,170,170,255);
            SDL_RenderDrawFatLine(renderer,pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/2+3), pxlenght(inputbox.ypos+STANDARTPICEHEIGHT/2+4), pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/4-1), pxlenght(inputbox.ypos+STANDARTPICEHEIGHT-4), pxlenght(STANDARTBORDER));
            SDL_RenderDrawFatLine(renderer,pxlenght(inputbox.xpos+inputbox.size-4), pxlenght(inputbox.ypos+STANDARTPICEHEIGHT/2+4), pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/4-1), pxlenght(inputbox.ypos+STANDARTPICEHEIGHT-4), pxlenght(STANDARTBORDER));
        }
    } else {
        SDL_RenderDrawFatLine(renderer,pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/2+3), pxlenght(inputbox.ypos+STANDARTPICEHEIGHT/2-4), pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/4-1), pxlenght(inputbox.ypos+4), pxlenght(STANDARTBORDER));
        SDL_RenderDrawFatLine(renderer,pxlenght(inputbox.xpos+inputbox.size-4), pxlenght(inputbox.ypos+STANDARTPICEHEIGHT/2-4), pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/4-1), pxlenght(inputbox.ypos+4), pxlenght(STANDARTBORDER));
        SDL_RenderDrawFatLine(renderer,pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/2+3), pxlenght(inputbox.ypos+STANDARTPICEHEIGHT/2+4), pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/4-1), pxlenght(inputbox.ypos+STANDARTPICEHEIGHT-4), pxlenght(STANDARTBORDER));
        SDL_RenderDrawFatLine(renderer,pxlenght(inputbox.xpos+inputbox.size-4), pxlenght(inputbox.ypos+STANDARTPICEHEIGHT/2+4), pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/4-1), pxlenght(inputbox.ypos+STANDARTPICEHEIGHT-4), pxlenght(STANDARTBORDER));
    }
    if (inputbox.active){
        inputbox.dashed=false;
        if (SDL_GetTicks()-inputbox.last_time_cursor_blink>CURSORPERIOD){
            inputbox.cursor_visible=!inputbox.cursor_visible;
            inputbox.last_time_cursor_blink=SDL_GetTicks();
        }
        if (inputbox.cursorpos > stringlen(inputbox.text)) inputbox.cursorpos = stringlen(inputbox.text);
        if (inputbox.cursorpos < 0) inputbox.cursorpos = 0;
        int cursorlenpx=(int)((*(proportions+inputbox.cursorpos))*(pxlenght((getTextureWidth(texttex,STANDARTPICEHEIGHT-STANDARTBORDER*2)+STANDARTBORDER*2))-STANDARTBORDER))+STANDARTBORDER;
        if (stringlen(typedChar)!=0 && isNumber(typedChar)){
            if (inputbox.start_of_selected==-1 && inputbox.end_of_selected==-1){
                inputbox.text+=typedChar;
                inputbox.cursorpos+=stringlen(typedChar);
            } else {
                replace_text_range(&inputbox.text, inputbox.start_of_selected, inputbox.end_of_selected, typedChar);
                inputbox.cursorpos=inputbox.start_of_selected+1;
                inputbox.start_of_selected=-1;
                inputbox.end_of_selected=-1;
            }
            inputbox.cursor_visible=true;
        }
        if (modkey & KMOD_SHIFT){
            if (specialkey=='l' && inputbox.cursorpos>0){
                if (inputbox.start_of_selected==-1 && inputbox.end_of_selected==-1){
                    inputbox.start_of_selected=inputbox.cursorpos-1;
                    inputbox.end_of_selected=inputbox.cursorpos;
                } else {
                    if (inputbox.cursorpos==inputbox.start_of_selected){
                        inputbox.start_of_selected--;
                    } else {
                        inputbox.end_of_selected--;
                    }
                }
                inputbox.cursorpos--;
                inputbox.cursor_visible=true;
            } else if (specialkey=='r' && inputbox.cursorpos<stringlen(inputbox.text)){
                if (inputbox.start_of_selected==-1 && inputbox.end_of_selected==-1){
                    inputbox.start_of_selected=inputbox.cursorpos;
                    inputbox.end_of_selected=inputbox.cursorpos+1;
                } else {
                    if (inputbox.cursorpos==inputbox.start_of_selected){
                        inputbox.start_of_selected++;
                    } else {
                        inputbox.end_of_selected++;
                    }
                }
                inputbox.cursorpos++;
                inputbox.cursor_visible=true;
            } else if (specialkey=='b'){
                inputbox.text="";
                inputbox.cursorpos=0;
                inputbox.cursor_visible=true;
            }
        } else {
            if (specialkey=='l' && inputbox.cursorpos>0){
                inputbox.cursorpos-=1;
                inputbox.start_of_selected=-1;
                inputbox.end_of_selected=-1;
                inputbox.cursor_visible=true;
            } else if (specialkey=='r' && inputbox.cursorpos<stringlen(inputbox.text)){
                inputbox.cursorpos+=1;
                inputbox.start_of_selected=-1;
                inputbox.end_of_selected=-1;
                inputbox.cursor_visible=true;
            } else if (specialkey=='b'){
                if (inputbox.start_of_selected==-1 && inputbox.end_of_selected==-1){
                    replace_text_range(&inputbox.text, inputbox.cursorpos-1, inputbox.cursorpos, "");
                } else {
                    replace_text_range(&inputbox.text, inputbox.start_of_selected, inputbox.end_of_selected, "");
                    inputbox.start_of_selected=-1;
                    inputbox.end_of_selected=-1;
                }
                inputbox.cursorpos--;
                inputbox.cursor_visible=true;
            }
        }
        if (inputbox.cursor_visible){SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255); SDL_RenderDrawFatLine(renderer,pxlenght(inputbox. xpos)+cursorlenpx, pxlenght(inputbox.ypos+STANDARTBORDER), pxlenght(inputbox. xpos)+cursorlenpx, pxlenght(inputbox.ypos+STANDARTPICEHEIGHT-STANDARTBORDER*2+STANDARTBORDER), pxlenght(3));}
    }
    if (clic){
        SDL_Point mousepoint={mousex,mousey};
        if (SDL_PointInRect(&mousepoint,&rect)){
            inputbox.dashed=false;
            if (mousex>pxlenght(inputbox.xpos+inputbox.size-STANDARTPICEHEIGHT/2)){
                if (mousey>pxlenght(inputbox.ypos+STANDARTPICEHEIGHT/2)){
                    inputbox.number--;
                    inputbox.text=std::to_string(inputbox.number);
                } else {
                    inputbox.number++;
                    inputbox.text=std::to_string(inputbox.number);
                }
            } else {
                if (inputbox.active){
                    inputbox.start_of_selected=-1;
                    inputbox.end_of_selected=-1;
                    for (i=0;mousex-rect.x>=(int)((*(proportions+i))*(float)(pxlenght((getTextureWidth(texttex,STANDARTPICEHEIGHT-STANDARTBORDER*2)+STANDARTBORDER*2))-STANDARTBORDER*2));i++)
                    inputbox.cursorpos=i;
                } else {
                    inputbox.start_of_selected=0;
                    inputbox.end_of_selected=stringlen(inputbox.text);
                }
                inputbox.active=true;
            }
            clickedOnSomething=true;
        } else {
            inputbox.active=false;
            inputbox.start_of_selected=-1;
            inputbox.end_of_selected=-1;
        }
    }

    if (inputbox.start_of_selected == inputbox.end_of_selected){
        inputbox.start_of_selected=-1;
        inputbox.end_of_selected=-1;
    }
    displayTex(renderer,texttex,pxlenght(inputbox.xpos+STANDARTBORDER), pxlenght(inputbox.ypos+STANDARTBORDER), pxlenght(STANDARTPICEHEIGHT-STANDARTBORDER*2));
    SDL_DestroyTexture(texttex);
    if (!inputbox.dashed){
        inputbox.number=stringToInt(inputbox.text);
    }
    if (inputbox.number>inputbox.maxnum){inputbox.number=inputbox.maxnum; inputbox.text=std::to_string(inputbox.maxnum);}
    if (inputbox.number<0){inputbox.number=0; inputbox.text="0";}
    *inputboxptr=inputbox;
    return;
}
void updateCheckBox(SDL_Renderer* renderer, checkbox* boxptr){
    checkbox box=*boxptr;
    SDL_Rect rect={pxlenght(box.xpos),pxlenght(box.ypos),pxlenght(15), pxlenght(15)};
    SDL_SetRenderDrawColor(renderer, 240, 240, 240,255);
    SDL_RenderFillRect(renderer, &rect);
    SDL_Rect rect1={pxlenght(box.xpos+2),pxlenght(box.ypos+2),pxlenght(11), pxlenght(11)};
    if (mousex>pxlenght(box.xpos) && mousex<pxlenght(box.xpos+15) && mousey>pxlenght(box.ypos) && mousey<pxlenght(box.ypos+15) && clicked){
        box.checked=!box.checked;         
    }   
    if (box.checked){
        SDL_SetRenderDrawColor(renderer, 0, 156, 255,255);
    } else {
        SDL_SetRenderDrawColor(renderer, 10, 15, 25,255);
    }
    if (mousex>pxlenght(box.xpos) && mousex<pxlenght(box.xpos+15) && mousey>pxlenght(box.ypos) && mousey<pxlenght(box.ypos+15)){
        if (box.checked){
            SDL_SetRenderDrawColor(renderer, 60, 110, 190,255);
        } else {
            SDL_SetRenderDrawColor(renderer, 80, 85, 85,255);
        }
    }
    SDL_RenderFillRect(renderer, &rect1);
    displayTex(renderer,box.title,pxlenght(box.xpos+20),pxlenght(box.ypos),pxlenght(15));
    *boxptr=box;
    return;

}
bool renderPoint(float x, float y, float z, bool selected, int kind) {
    int size=pxlenght(6);
    int ix = (int)x;
    int iy = (int)y;

    if (z <= 0.0f) return false;

    bool hovered = false;

    int half = size / 2;

    // hover check (AABB)
    if ((mousex >= ix - half && mousex <= ix + half && mousey-viewport_ypos >= iy - half && mousey-viewport_ypos <= iy + half) || (boxselecting && ix>boxselect_startx-viewport_xpos && iy>boxselect_starty-viewport_ypos && ix<mousex-viewport_xpos && iy<mousey-viewport_ypos)) {
        hovered = true;
        if (clicked){
            clickedOnSomething=true;
        }
    }
    for (int dx = -half; dx <= half; dx++) {
        for (int dy = -half; dy <= half; dy++) {

            int px = ix + dx;
            int py = iy + dy;

            if (px < 0 || py < 0 || px >= viewport_x || py >= viewport_y)
                continue;

            if (viewport_buffer[px][py][3] > z) {
                viewport_buffer[px][py][3] = z;

                if (hovered) {
                    viewport_buffer[px][py][0] = 150;
                    viewport_buffer[px][py][1] = 255;
                    viewport_buffer[px][py][2] = 150;
                } else if (selected){
                    viewport_buffer[px][py][0] = 255;
                    viewport_buffer[px][py][1] = 255;
                    viewport_buffer[px][py][2] = 255;
                } else {
                    if (kind==0){
                        viewport_buffer[px][py][0] = 0;
                        viewport_buffer[px][py][1] = 150;
                        viewport_buffer[px][py][2] = 255;
                    } else if (kind==1){
                        viewport_buffer[px][py][0] = 255;
                        viewport_buffer[px][py][1] = 200;
                        viewport_buffer[px][py][2] = 000;
                    } else if (kind==2){
                        viewport_buffer[px][py][0] = 255;
                        viewport_buffer[px][py][1] = 0;
                        viewport_buffer[px][py][2] = 255;
                    } else if (kind==3){
                        viewport_buffer[px][py][0] = 0;
                        viewport_buffer[px][py][1] = 255;
                        viewport_buffer[px][py][2] = 255;
                    } else if (kind==4){
                        viewport_buffer[px][py][0] = 0;
                        viewport_buffer[px][py][1] = 255;
                        viewport_buffer[px][py][2] = 0;
                    } else if (kind==5){
                        viewport_buffer[px][py][0] = 255;
                        viewport_buffer[px][py][1] = 0;
                        viewport_buffer[px][py][2] = 0;
                    } else {
                        viewport_buffer[px][py][0] = 100;
                        viewport_buffer[px][py][1] = 100;
                        viewport_buffer[px][py][2] = 0;
                    }
                }
            }
        }
    }

    return hovered;
}
bool renderPoint(float x, float y, float z, int r, int g, int b){
    bool ret=false;
    if ((int)(x)<viewport_x && (int)(y)<viewport_y && z>0.0f && x>0.0f && y>0.0f){
        if (viewport_buffer[(int)(x)][(int)(y)][3]>z){
            viewport_buffer[(int)(x)][(int)(y)][0]=r;
            viewport_buffer[(int)(x)][(int)(y)][1]=g;
            viewport_buffer[(int)(x)][(int)(y)][2]=b;
            if (((((int)x)/10)==(mousex-viewport_xpos)/10 && (((int)y)/10)==(mousey-viewport_ypos)/10) || (boxselecting && (int)x>boxselect_startx-viewport_xpos && (int)y>boxselect_starty-viewport_ypos && (int)x<mousex-viewport_xpos && (int)y<mousey-viewport_ypos)){
                ret = true;
                if (clicked){
                    clickedOnSomething=true;
                }
            }
        }
    }
    return ret;
}
bool renderLine(float x1,float y1,float z1,float x2,float y2,float z2,int r,int g,int b){
    if (((int)z1<0.0f && (int)z2<0.0f) || ((int)x1<0.0f && (int)x2<0.0f) || ((int)y1<0.0f && (int)y2<0.0f) || ((int)x1>viewport_x && (int)x2>viewport_x) || ((int)y1>viewport_y && (int)y2>viewport_y)){return false;}
    float xf=x1;
    float yf=y1;
    float zf=z1;
    int steps;
    bool ret=false;
    if (max(x2-x1)>max(y2-y1)){
        steps=(int)(max(x2-x1)*1.1);
    } else {
        steps=(int)(max(y2-y1)*1.1);
    }
    for (int step=0;step<steps;step++){
        if (renderPoint(xf,yf,zf, r, g, b)){ret=true;}
        xf+=(x2-x1)/(float)steps;
        yf+=(y2-y1)/(float)steps;
        zf+=(z2-z1)/(float)steps;
    }
    return ret;
}
void init_viewport_texture(SDL_Renderer* renderer) {
    viewport_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        viewport_x,
        viewport_y
    );
}
void resize_viewport(SDL_Renderer* renderer, int new_w, int new_h)
{
    viewport_x = new_w;
    viewport_y = new_h;

    // zabij starou texture
    if (viewport_texture) {
        SDL_DestroyTexture(viewport_texture);
        viewport_texture = NULL;
    }

    // vytvoř novou texture s novým rozměrem
    viewport_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        viewport_x,
        viewport_y
    );

    if (!viewport_texture) {
        printf("texture resize failed: %s\n", SDL_GetError());
    }
}
void display_viewport(SDL_Renderer* renderer){
    void* pixels_void;
    int pitch;

    SDL_LockTexture(viewport_texture, NULL, &pixels_void, &pitch);

    uint8_t* dst = (uint8_t*)pixels_void;

    for (int y = 0; y < viewport_y; y++) {
        uint32_t* row = (uint32_t*)(dst + y * pitch);

        for (int x = 0; x < viewport_x; x++) {
            int* src = viewport_buffer[x][y]; // <-- int[4]

            row[x] =
                0xFF000000 |
                ((uint8_t)src[0] << 16) |
                ((uint8_t)src[1] << 8)  |
                ((uint8_t)src[2]);
        }
    }

    SDL_UnlockTexture(viewport_texture);

    SDL_Rect dstrect = {
        viewport_xpos,
        viewport_ypos,
        viewport_x,
        viewport_y
    };

    SDL_RenderCopy(renderer, viewport_texture, NULL, &dstrect);
}
void project_point(float x, float y, float z, float* out_x, float* out_y){
    if (z <= 0.001f) z = 0.001f; // aby to neexplodovalo

    float viewport_fov_rad = viewport_fov * (3.141592f / 180.0f);
    float f = 1.0f / tanf(viewport_fov_rad * 0.5f);

    *out_x = (x * f / z) * viewport_aspect_ratio;
    *out_y = (y * f / z);
}
void project_point_ortho(float x, float y, float z, float* out_x, float* out_y){
    float fov_rad = viewport_fov * (3.14159265f / 180.0f);
    float f = 1.0f / tanf(fov_rad * 0.5f);

    float scale = f / z_pos;

    *out_x = (x * scale) * viewport_aspect_ratio;
    *out_y = (y * scale);
}
void render(SDL_Renderer* renderer){
    if (typedChar.c_str()[0]=='s'){
        for (i=0;i<MAX_PARTS;i++){
            render_points[i].x=points[i].x;
            render_points[i].y=points[i].y;
            render_points[i].z=points[i].z;
            render_points[i].exists=points[i].exists;
            points[i].hidden=false;
            joints[i].hidden=false;
        }
    } else {
        for (i=0;i<MAX_PARTS;i++){
            render_points[i].x=points[i].x;
            render_points[i].y=points[i].y;
            render_points[i].z=points[i].z;
            render_points[i].exists=points[i].exists;
        }
    }
    for (int x = 0; x < viewport_x; x++){
        for (int y = 0; y < viewport_y; y++) {
            viewport_buffer[x][y][0] = 0;
            viewport_buffer[x][y][1] = 5;
            viewport_buffer[x][y][2] = 20;
            viewport_buffer[x][y][3] = 1e9; // Z-buffer
        }
    }


    if (rotatingviewport){
        x_rot+=(float)(mousex-viewport_drag_start_mousex)/viewport_scale*3.0f;
        y_rot-=(float)(mousey-viewport_drag_start_mousey)/viewport_scale*3.0f;
        viewport_drag_start_mousex=mousex;
        viewport_drag_start_mousey=mousey;
    }
    for (i = 0; i < MAX_PARTS; i++) {
        if (!render_points[i].exists) continue;

        float x = render_points[i].x;
        float y = render_points[i].y;
        float z = render_points[i].z;

        // --- ROT Y ---
        float cy = cosf(-x_rot);
        float sy = sinf(-x_rot);
        float x2 =  x * cy + z * sy;
        float z2 = -x * sy + z * cy;

        x = x2;
        z = z2;

        // --- ROT X ---
        float cx = cosf(y_rot);
        float sx = sinf(y_rot);
        float y1 = y * cx - z * sx;
        float z1 = y * sx + z * cx;

        y = y1;
        z = z1;


        // --- ROT Z ---
        float cz = cosf(z_rot);
        float sz = sinf(z_rot);
        float x3 = x * cz - y * sz;
        float y3 = x * sz + y * cz;

        render_points[i].x = x3;
        render_points[i].y = y3;
        render_points[i].z = z;
    }





    if (movingviewport){
        x_pos+=(float)(mousex-viewport_drag_start_mousex)/viewport_scale*2.5f;
        y_pos-=(float)(mousey-viewport_drag_start_mousey)/viewport_scale*2.5f;
        viewport_drag_start_mousex=mousex;
        viewport_drag_start_mousey=mousey;
    }
    part_index=0;
    for (i=0;i<MAX_PARTS;i++){
        if (render_points[i].exists){
            render_points[i].x+=x_pos;
            render_points[i].y+=y_pos;
            render_points[i].z+=z_pos;
            if (projection.selected==0){
                project_point(
                    render_points[i].x,
                    render_points[i].y,
                    render_points[i].z,
                    &render_points[i].x,
                    &render_points[i].y
                );
            } else {
                project_point_ortho(
                    render_points[i].x,
                    render_points[i].y,
                    render_points[i].z,
                    &render_points[i].x,
                    &render_points[i].y
                );
            }
            render_points[i].x*=viewport_scale;
            render_points[i].y*=-viewport_scale;
            render_points[i].z*=viewport_scale;
            render_points[i].x+=(float)(viewport_x/2);
            render_points[i].y+=(float)(viewport_y/2);
            if (points[i].selected && typedChar.c_str()[0]=='h'){
                points[i].hidden=true;
                points[i].selected=false;
            }
            if (show_points.checked && !points[i].hidden){
                if ((points[i].kind==0 && show_body.checked==true) || 
                    (points[i].kind==1 && show_suspention.checked==true) || 
                    (points[i].kind==2 && show_construction.checked==true) || 
                    (points[i].kind==3 && show_g1.checked==true) || 
                    (points[i].kind==4 && show_g2.checked==true) ||
                    (points[i].kind==5 && show_g3.checked==true)){
                    if (renderPoint(render_points[i].x,render_points[i].y,render_points[i].z, points[i].selected, points[i].kind) && clicked){
                        points[i].selected=!points[i].selected;
                        if (boxselect){
                            points[i].selected=true;
                        }
                    }
                    if (points[i].selected){
                        if (selected_index==-1){
                            selected_index=part_index;
                        } else {
                            selected_index=-2;
                        }

                        if (kind.dashed){
                            kind.selected=points[i].kind;
                        }
                        if (attribute.dashed){
                            attribute.selected=points[i].attribute;
                        }                        
                        if (mass.dashed){
                            mass.text=std::to_string((int)(points[i].mass*100.0f));
                        }
                        kind.dashed=false;
                        attribute.dashed=false;
                        mass.dashed=false;
                        points[i].kind=kind.selected;
                        points[i].attribute=attribute.selected;
                        points[i].mass=(float)mass.number/100.0f;
                        something_selected=true;
                        if (p1ind==-1){
                            p1ind=i;
                        } else if (p2ind==-1){
                            p2ind=i;
                        }
                    }
                }
            }
            part_index++;
        }
    }
    part_index=0;
    for (i=0;i<MAX_PARTS;i++){
        if (joints[i].exists){
            if (show_joints.checked && !joints[i].hidden){
                j=0;
                if (joints[i].kind==0 && show_body.checked){
                    jr=0;
                    jg=150;
                    jb=255;
                    j=1;
                } else if (joints[i].kind==1 && show_suspention.checked){
                    jr=255;
                    jg=200;
                    jb=0;
                    j=1;
                } else if (joints[i].kind==2 && show_construction.checked){
                    jr=255;
                    jg=0;
                    jb=255;
                    j=1;
                } else if (joints[i].kind==3 && show_g1.checked){
                    jr=0;
                    jg=255;
                    jb=255;
                    j=1;
                } else if (joints[i].kind==4 && show_g2.checked){
                    jr=0;
                    jg=255;
                    jb=0;
                    j=1;
                } else if (joints[i].kind==5 && show_g3.checked){
                    jr=255;
                    jg=0;
                    jb=0;
                    j=1;
                }
                if (j==1){
                    if (i==0){
                    }
                    if (renderLine(render_points[joints[i].p1].x, render_points[joints[i].p1].y, render_points[joints[i].p1].z, render_points[joints[i].p2].x, render_points[joints[i].p2].y, render_points[joints[i].p2].z, jr,jg,jb)){
                        renderLine(render_points[joints[i].p1].x, render_points[joints[i].p1].y, render_points[joints[i].p1].z, render_points[joints[i].p2].x, render_points[joints[i].p2].y, render_points[joints[i].p2].z, 150,255,150);
                        if (clicked){
                            joints[i].selected=!joints[i].selected;
                        }
                        if (boxselect){
                            joints[i].selected=true;
                        }
                    } else if (joints[i].selected){
                        renderLine(render_points[joints[i].p1].x, render_points[joints[i].p1].y, render_points[joints[i].p1].z, render_points[joints[i].p2].x, render_points[joints[i].p2].y, render_points[joints[i].p2].z, 255,255,255);
                        if (selected_index==-1){
                            selected_index=part_index;
                        } else {
                            selected_index=-2;
                        }
                        if (kind.dashed){
                            kind.selected=joints[i].kind;
                        }
                        if (attribute.dashed){
                            attribute.selected=joints[i].attribute;
                        }
                        if (stiffness.dashed){
                            stiffness.text=std::to_string((int)(joints[i].stiffness*100.0f));
                        }
                        if (damping.dashed){
                            damping.text=std::to_string((int)(joints[i].damping*100.0f));
                        }
                        if (min_len.dashed){
                            min_len.text=std::to_string((int)(joints[i].min_len*100.0f));
                        }
                        if (elastic_margin.dashed){
                            elastic_margin.text=std::to_string((int)(joints[i].elastic_margin*100.0f));
                        }
                        kind.dashed=false;
                        attribute.dashed=false;
                        stiffness.dashed=false;
                        damping.dashed=false;
                        min_len.dashed=false;
                        elastic_margin.dashed=false;
                        joints[i].kind=kind.selected;
                        joints[i].attribute=attribute.selected;
                        joints[i].stiffness=(float)stiffness.number/100.0f;
                        joints[i].damping=(float)damping.number/100.0f;
                        joints[i].min_len=(float)min_len.number/100.0f;
                        joints[i].elastic_margin=(float)elastic_margin.number/100.0f;
                        something_selected=true;
                    }
                }
            }
            if (joints[i].selected && typedChar.c_str()[0] == 'h'){
                joints[i].hidden=true;
                joints[i].selected=false;
            }
            part_index++;
        }
    }
    if (typedChar.c_str()[0]=='j' && p1ind!=-1 && p2ind!=-1){
        joints[joints_count].p1=p1ind;
        joints[joints_count].p2=p2ind;
        joints[joints_count].stiffness=stiffness.number;
        joints[joints_count].damping=damping.number;
        joints[joints_count].elastic_margin=elastic_margin.number;
        joints[joints_count].exists=true;
        joints[joints_count].kind=kind.selected;
        joints_count++;
        printf("added joint");
    }
    display_viewport(renderer);
}
void fps_update(void)
{
    Uint32 now = SDL_GetTicks();
    fps_frames++;

    if (now - fps_last_time >= 500) { // update 2× za sekundu
        fps = fps_frames * 1000.0f / (now - fps_last_time);
        fps_frames = 0;
        fps_last_time = now;
    }
}
int main() {
    #include "init.cpp"
    #include "topbar.hpp"
    for (i=0;i<MAX_PARTS;i++){
        points[i].exists=false;
        joints[i].exists=false;
    }
    points[0]={-1, -1, -1, .kind=2};
    points[1]={1, -1, -1, .kind=1};
    points[2]={1, -1, 1, .kind=3};
    points[3]={-1, -1, 1};
    points[4]={-1, 1, -1};
    points[5]={1, 1, -1};
    points[6]={1, 1, 1};
    points[7]={-1, 1, 1};
    joints[0]={0,1, .kind=2};
    joints[1]={1,2, .kind=1};
    joints[2]={2,3};
    joints[3]={3,0};
    joints[4]={4,5};
    joints[5]={5,6};
    joints[6]={6,7};
    joints[7]={7,4};
    joints[8]={0,4};
    joints[9]={1,5};
    joints[10]={2,6};
    joints[11]={3,7};
    for (i=0;i<8;i++){
        points[i].exists=true;
    }
    for (i=0;i<12;i++){
        joints[i].exists=true;
    }
    z_pos=4.0f;
    viewport_x=640;
    viewport_y=480;
    viewport_scale=200.0f;
    viewport_xpos=0;
    viewport_ypos=pxlenght(TOPPICEHEIGHT);
    viewport_aspect_ratio=1.0f;
    viewport_fov=80.0f;
    movingviewport=false;
    rotatingviewport=false;
    init_viewport_texture(renderer);
    movestep=0.05f;
    rotstep=1.0f;
    z_rot=0;
    wait_for_boxselect_end=0;
    while (running){
        if (clickup){clickup=false;}
        SDL_GetRendererOutputSize(renderer, &width, &height);
        clic=false;
        clicked=false;
        typedChar="";
        specialkey=0;
        modkey=KMOD_NONE;
        clickedOnSomething=false;
        boxselect=false;
        something_selected=false;
        p1ind=-1;
        p2ind=-1;
        while (SDL_PollEvent(&event)) {
            switch (event.type){
                case SDL_QUIT:
                    SDL_DestroyWindow(window);
                    SDL_Quit();
                    return 0;
                    break;
                case SDL_MOUSEMOTION:
                    if (retina){
                        mousex = event.motion.x*2;
                        mousey = event.motion.y*2;
                    } else {
                        mousex = event.motion.x;
                       mousey = event.motion.y;
                    }
                    gonna_delelect=false;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        if (mousex>viewport_xpos && mousey>viewport_ypos && mousex<viewport_xpos+viewport_x && mousey<viewport_ypos+viewport_y){
                            rotatingviewport=!shift_down;
                            if (boxselecting && !shift_down){clickedOnSomething=true;boxselect=true;}
                            if (shift_down){
                                boxselecting=true;
                                boxselect_startx=mousex;
                                boxselect_starty=mousey;
                            } else {
                                wait_for_boxselect_end=1;
                            }
                            viewport_drag_start_mousex=mousex;
                            viewport_drag_start_mousey=mousey;
                        }
                        clicked=true;
                        clic=true;
                    } else if (event.button.button == SDL_BUTTON_RIGHT){
                        if (mousex>viewport_xpos && mousey>viewport_ypos && mousex<viewport_xpos+viewport_x && mousey<viewport_ypos+viewport_y){
                            movingviewport=!shift_down;
                            viewport_drag_start_mousex=mousex;
                            viewport_drag_start_mousey=mousey;
                        }
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    clickup=true;
                    clicked=false;
                    movingviewport=false;
                    rotatingviewport=false;
                    break;
                case SDL_MOUSEWHEEL:
                    if (event.wheel.y > 0) {
                        z_pos += ((float)event.wheel.y/10.0f);
                    } else if (event.wheel.y < 0) {
                        z_pos += ((float)event.wheel.y/10.0f);
                    }
                case SDL_TEXTINPUT:
                    typedChar=event.text.text;
                    break;
                case SDL_KEYDOWN:
                    modkey = SDL_GetModState();
                    shift_down = modkey & KMOD_SHIFT;
                    if (modkey & KMOD_COMMAND) {
                        switch (event.key.keysym.sym){
                            case SDLK_EQUALS:
                                making_new_part=false;
                                scale+=0.2;
                                break;
                            case SDLK_MINUS:
                                making_new_part=false;
                                scale-=0.2;
                                if (scale<=0.0){scale=0.2;}
                                break;
                            }
                    } else {
                        switch (event.key.keysym.sym){
                            case SDLK_LEFT:
                                specialkey='l';
                                break;
                            case SDLK_RIGHT:
                                specialkey='r';
                                break;
                            case SDLK_BACKSPACE:
                                specialkey='b';
                                break;
                    }
                    break;
                case SDL_KEYUP:
                    shift_down=false;
                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    {
                        int new_w = pxlenght(event.window.data1);
                        int new_h = pxlenght(event.window.data2-TOPPICEHEIGHT);
                        viewport_scale=new_w/2.0;
                        resize_viewport(renderer, new_w, new_h);
                    }
                    break;
                }
            }
        }
        selected_index=-1;
        // viewport_x, viewport_y=SDL_GetWindowSizeInPixels(window);
        SDL_RenderClear(renderer);
        SDL_Rect rect = { 0, 0, width, height };
        SDL_SetRenderDrawColor(renderer, 15, 20, 35, 255);
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 50,50,60,255);
        SDL_Rect rect1 = {0,0,width,pxlenght(TOPPICEHEIGHT)};
        SDL_RenderFillRect(renderer, &rect1);
        displayTex(renderer, createTextTexture(renderer, "Stiffness(%)   Min. lenght(%)   Damping(%)  Elastic margin(%)              Kind            Vertex mass   attribute              Projection"), pxlenght(120), pxlenght(5), pxlenght(18));
        render(renderer);
        if (boxselecting){
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer,50, 200, 50, 50);
            SDL_Rect rect2 = {boxselect_startx,boxselect_starty,mousex-boxselect_startx, mousey-boxselect_starty};
            SDL_RenderFillRect(renderer, &rect2);
        }
        if (wait_for_boxselect_end>0 && wait_for_boxselect_end<2){
            wait_for_boxselect_end++;
        } else if (wait_for_boxselect_end>=2){
            wait_for_boxselect_end=0;
            boxselecting=false;
        }
        fps_update();
        snprintf(fpstext, sizeof(fpstext), "FPS: %d", (int)fps);
        displayTex(renderer, createTextTexture(renderer, fpstext), viewport_xpos+pxlenght(10), viewport_ypos+pxlenght(10), pxlenght(14));
        if (selected_index>=0){
        snprintf(fpstext, sizeof(fpstext), "Selected index: %d", selected_index);
            displayTex(renderer, createTextTexture(renderer, fpstext), viewport_xpos+pxlenght(10), viewport_ypos+pxlenght(30), pxlenght(14));
        }
        updateNumberInputBox(renderer,&stiffness);
        updateNumberInputBox(renderer,&min_len);
        updateNumberInputBox(renderer,&mass);
        updateNumberInputBox(renderer,&damping);
        updateNumberInputBox(renderer,&elastic_margin);
        updateDropMenu(renderer, &filemenu);
        updateDropMenu(renderer, &kind);
        updateDropMenu(renderer, &projection);
        updateDropMenu(renderer, &attribute);
        updateCheckBox(renderer, &show_points);
        updateCheckBox(renderer, &show_joints);
        updateCheckBox(renderer, &show_body);
        updateCheckBox(renderer, &show_construction);
        updateCheckBox(renderer, &show_suspention);
        updateCheckBox(renderer, &show_g1);
        updateCheckBox(renderer, &show_g2);
        updateCheckBox(renderer, &show_g3);
        if (!clickedOnSomething && clicked && mousex>viewport_xpos && mousey>viewport_ypos && mousex<viewport_x+viewport_xpos && mousey<viewport_y+viewport_ypos && !boxselecting){
            gonna_delelect=true;
        }
        if (clickup && gonna_delelect && !boxselecting){
            for (i=0;i<MAX_PARTS;i++){
                points[i].selected=false;
                joints[i].selected=false;
            }
        }
        SDL_RenderPresent(renderer);

    }
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}