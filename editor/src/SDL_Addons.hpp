#include <cmath>
#define BORDER_HEIGHT 5  // výška průhledného pruhu nahoře a dole v pixelech

bool SDL_RenderCircleWithHover(SDL_Renderer* renderer, int cx, int cy, int radius, SDL_Color baseColor) {
    // Rect = čtverec opsaný kruhu
    radius /=2;
    SDL_Rect boundingBox = { cx - radius, cy - radius, radius * 2, radius * 2 };
    
    // Zjisti, jestli bod je uvnitř toho čtverce
    SDL_Point poi={mousex,mousey};
    bool hovered = SDL_PointInRect(&poi, &boundingBox);

    // Ztmavení barvy pokud hover
    SDL_Color finalColor = baseColor;
    if (hovered) {
        finalColor.r = (Uint8)(baseColor.r * HOVER_DARKEN);
        finalColor.g = (Uint8)(baseColor.g * HOVER_DARKEN);
        finalColor.b = (Uint8)(baseColor.b * HOVER_DARKEN);
    }

    SDL_SetRenderDrawColor(renderer, finalColor.r, finalColor.g, finalColor.b, finalColor.a);

    int r2 = radius * radius;
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= r2) {
                SDL_RenderDrawPoint(renderer, cx + x, cy + y);
            }
        }
    }
    return hovered;
}
void SDL_RenderCircle(SDL_Renderer* renderer, int cx, int cy, int radius, SDL_Color color) {
    // Rect = čtverec opsaný kruhu
    radius /=2;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    int r2 = radius * radius;
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= r2) {
                SDL_RenderDrawPoint(renderer, cx + x, cy + y);
            }
        }
    }
}
void SDL_RenderFillRoundRect(SDL_Renderer* renderer, const SDL_Rect* rect, int radius=pxlenght(5)) {
    if (round_rect){
        if (!renderer || !rect || radius <= 0) {
            SDL_RenderFillRect(renderer, rect);
            return;
        }
        const int x = rect->x;
        const int y = rect->y;
        const int w = rect->w;
        const int h = rect->h;
        radius = SDL_min(radius, SDL_min(w, h) / 2);
        SDL_Rect center = {x + radius, y, w - 2 * radius, h};
        SDL_RenderFillRect(renderer, &center);
        SDL_Rect center2 = {x, y + radius, w, h - 2 * radius};
        SDL_RenderFillRect(renderer, &center2);
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (dx*dx + dy*dy <= radius*radius) {
                    SDL_RenderDrawPoint(renderer, x + radius + dx, y + radius + dy); // levý horní
                    SDL_RenderDrawPoint(renderer, x + w - radius + dx - 1, y + radius + dy); // pravý horní
                    SDL_RenderDrawPoint(renderer, x + radius + dx, y + h - radius + dy - 1); // levý dolní
                    SDL_RenderDrawPoint(renderer, x + w - radius + dx - 1, y + h - radius + dy - 1); // pravý dolní
                }
            }
        }
    } else {
        SDL_RenderFillRect(renderer, rect);
    }
}
bool SDL_RenderFatLineAndCheckPoint(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, int fatness, int skipEnds, SDL_Color baseColor, int brushSize = 2, int pointx=mousex, int pointy=mousey, bool hover=false) {
    bool hit=false;

    float dx = x2 - x1;
    float dy = y2 - y1;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len == 0) return false;

    dx /= len;
    dy /= len;

    float nx = -dy;
    float ny = dx;

    int start = skipEnds;
    int end = (int)len - skipEnds;

    if (end <= start) return false;

    // Kolizní test
    for (int i = 0; i < (int)len; i++) {
        float px = x1 + dx * i;
        float py = y1 + dy * i;

        for (int f = -fatness / 2; f <= fatness / 2; f++) {
            int fx = (int)std::round(px + nx * f);
            int fy = (int)std::round(py + ny * f);

            SDL_Rect brush = { fx - brushSize / 2, fy - brushSize / 2, brushSize, brushSize };
            if (i >= start && i <= end &&
                pointx >= brush.x && pointx < brush.x + brush.w &&
                pointy >= brush.y && pointy < brush.y + brush.h) {
                hit = true;
            }
        }
    }

    // Barva
    SDL_Color color = baseColor;
    if (hit || hover) {
        color.r = (Uint8)(baseColor.r * HOVER_DARKEN);
        color.g = (Uint8)(baseColor.g * HOVER_DARKEN);
        color.b = (Uint8)(baseColor.b * HOVER_DARKEN);
    }
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    // Kreslení čar pomocí čtverců
    for (int i = 0; i < (int)len; i++) {
        float px = x1 + dx * i;
        float py = y1 + dy * i;

        for (int f = -fatness / 2; f <= fatness / 2; f++) {
            int fx = (int)std::round(px + nx * f);
            int fy = (int)std::round(py + ny * f);

            SDL_Rect brush = { fx - brushSize / 2, fy - brushSize / 2, brushSize, brushSize };
            SDL_RenderFillRect(renderer, &brush);
        }
    }

    return hit;
}
void SDL_RenderDrawFatLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, int fatness, int brushSize=2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float len = std::sqrt(dx * dx + dy * dy);

    dx /= len;
    dy /= len;

    float nx = -dy;
    float ny = dx;

    for (int i = 0; i < (int)len; i++) {
        float px = x1 + dx * i;
        float py = y1 + dy * i;

        for (int f = -fatness / 2; f <= fatness / 2; f++) {
            int fx = (int)std::round(px + nx * f);
            int fy = (int)std::round(py + ny * f);

            SDL_Rect brush = { fx - brushSize / 2, fy - brushSize / 2, brushSize, brushSize };
            SDL_RenderFillRect(renderer, &brush);
        }
    }

}
SDL_Texture* createSolidColorTexture(SDL_Renderer* renderer, int width, int height, Uint8 r, Uint8 g, Uint8 b) {
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, width, height);
    if (!texture) {
        // SDL_Log("Failed to create texture: %s", SDL_GetError());
        return nullptr;
    }

    // Umožní průhlednost
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

    // Přepni render cíl na tu texturu
    SDL_SetRenderTarget(renderer, texture);

    // Celou texturu vyplň průhlednou barvou
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    // Vykresli neprůhledný obdélník (střed bez horního a dolního okraje)
    SDL_Rect solidRect = { 0, BORDER_HEIGHT, width, height - 2 * BORDER_HEIGHT };
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    SDL_RenderFillRect(renderer, &solidRect);

    // Vrať zpět render target
    SDL_SetRenderTarget(renderer, nullptr);

    return texture;
}

SDL_Texture* createTextTexture(SDL_Renderer* renderer, const std::string& text, int start_of_selected = -1, int end_of_selected = -1)
{

    if (!font) {
        SDL_Log("TTF_OpenFont failed: %s", TTF_GetError());
        return nullptr;
    }

    auto chars = utf8_split_graphemes(text);

    // Renderuj každý znak zvlášť
    std::vector<SDL_Surface*> surfaces;
    std::vector<SDL_Surface*> bg_surfaces;

    for (size_t i = 0; i < chars.size(); ++i) {
        SDL_Color fg, bg;
        bool selected = (start_of_selected >= 0 && end_of_selected >= 0 &&
                         (int)i >= start_of_selected && (int)i < end_of_selected);

        if (selected) {
            fg = {255, 255, 255, 255};
            bg = {20, 120, 255, 255};  // modré pozadí
        } else {
            fg = {255, 255, 255, 255};
            bg = {255, 255, 255, 0};   // průhledné
        }

        SDL_Surface* glyph = TTF_RenderUTF8_Blended(font, chars[i].c_str(), fg);
        if (!glyph) continue;

        SDL_Surface* combined = SDL_CreateRGBSurfaceWithFormat(0, glyph->w, glyph->h, 32, SDL_PIXELFORMAT_RGBA32);
        SDL_FillRect(combined, NULL, SDL_MapRGBA(combined->format, bg.r, bg.g, bg.b, bg.a));
        SDL_BlitSurface(glyph, NULL, combined, NULL);

        surfaces.push_back(combined);
        SDL_FreeSurface(glyph);
    }

    // Spočítáme celkovou velikost textu
    int total_w = 0, max_h = 0;
    for (auto& s : surfaces) {
        total_w += s->w;
        if (s->h > max_h) max_h = s->h;
    }

    // Složíme výsledný surface
    SDL_Surface* result = SDL_CreateRGBSurfaceWithFormat(0, total_w, max_h, 32, SDL_PIXELFORMAT_RGBA32);
    int x = 0;
    int positions[stringlen(text)+2];
    int shit=0;
    for (auto& s : surfaces) {
        SDL_Rect dst = {x, 0, s->w, s->h};
        SDL_BlitSurface(s, NULL, result, &dst);
        positions[shit]=x;
        x += s->w;
        SDL_FreeSurface(s);
        shit++;
    }

    for (shit = 0; shit < stringlen(text); shit++) {
        proportions[shit] = (float)positions[shit] / (float)total_w;
    }
    proportions[shit]=1.0;


    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, result);
    SDL_FreeSurface(result);

    if (!tex) {
        // printf("SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
    }
    return tex;
    // return createSolidColorTexture(renderer, 100,20,24,3,45);

}
SDL_Texture* loadTexture(const char* file, SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load(file);
    if (!surface) {
        // SDL_Log("IMG_Load failed: %s", IMG_GetError());
        return nullptr;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    if (!texture) {
        // SDL_Log("SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
        return nullptr;
    }

    return texture;
}
void displayTex(SDL_Renderer* renderer, SDL_Texture* texture, int x, int y, int targetH) {
    int texW, texH;
    if (SDL_QueryTexture(texture, NULL, NULL, &texW, &texH) != 0) {
        // SDL_Log("Chyba při získávání velikosti textury: %s", SDL_GetError());
        return;
    }

    float aspect = (float)texW / texH;
    int targetW = (int)(targetH * aspect);

    SDL_Rect dstRect = { x, y, targetW, targetH };
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
}
void drawTexture(SDL_Renderer* renderer, SDL_Texture* texture, int x, int y, int width, int height) {
    SDL_Rect dstRect = { x, y, width, height };
    SDL_RenderCopy(renderer, texture, nullptr, &dstRect);
}
int getTextureWidth(SDL_Texture* texture, int height){
    int txw;
    int txh;
    SDL_QueryTexture(texture, nullptr, nullptr, &txw, &txh);
    return (int)(txw/((float)txh/(float)height));
}