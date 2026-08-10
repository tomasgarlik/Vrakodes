void updateDropMenu(SDL_Renderer* renderer, dropmenu* menuptr){
    dropmenu menu=*menuptr;
    if (!something_selected && menu.dashable){
        menu.dashed=true;
    }
    SDL_Rect rect={pxlenght(menu.xpos),pxlenght(menu.ypos),pxlenght(menu.topwidth), pxlenght(STANDARTPICEHEIGHT)};
    SDL_SetRenderDrawColor(renderer, 240, 240, 240,255);
    SDL_RenderFillRoundRect(renderer, &rect);
    if (menu.opened){
        some_dropmenu_opened=true;
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
        if (clickup && (something_selected || !menu.dashable) && !some_dropmenu_opened){
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
    box.just_changed=false;
    SDL_Rect rect={pxlenght(box.xpos),pxlenght(box.ypos),pxlenght(15), pxlenght(15)};
    SDL_SetRenderDrawColor(renderer, 240, 240, 240,255);
    SDL_RenderFillRect(renderer, &rect);
    SDL_Rect rect1={pxlenght(box.xpos+2),pxlenght(box.ypos+2),pxlenght(11), pxlenght(11)};
    if (mousex>pxlenght(box.xpos) && mousex<pxlenght(box.xpos+15) && mousey>pxlenght(box.ypos) && mousey<pxlenght(box.ypos+15) && clicked){
        box.checked=!box.checked;   
        box.just_changed=true;
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
