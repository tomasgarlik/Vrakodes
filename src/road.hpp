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
#include "opengl_include.h"
#include "constants.h"
#include "general_functions.hpp"
#include "types.hpp"
#include "global_vars.hpp"
#include "renderer.hpp"
#endif
#define ROAD_LOGGING 1
void subdivide_roads(int start_part_idx, int end_part_idx) {
    // return;
    if (start_part_idx >= end_part_idx) return;
    if (start_part_idx < 0 || end_part_idx > roadparts_len) return;

    // --- Stupeň (degree) každého bodu přes CELOU síť silnic ---
    // (křižovatka může být napojena i segmenty mimo tento úsek)
    std::vector<int> point_degree(roadpoints.size(), 0);
    for (int i = 0; i < roadparts_len; i++) {
        point_degree[roadparts[i].p1]++;
        point_degree[roadparts[i].p2]++;
    }

    // --- Body se stejnou pozicí (x,y) = fakticky taky křižovatka, ---
    // --- jen reprezentovaná dvěma různými indexy.                 ---
    std::unordered_map<long long, int> pos_count;
    pos_count.reserve(roadpoints.size() * 2);
    for (int i = 0; i < (int)roadpoints.size(); i++) {
        long long key = ((long long)roadpoints[i].x << 32) ^ (unsigned int)roadpoints[i].y;
        pos_count[key]++;
    }

    auto is_locked = [&](int idx) {
        if (point_degree[idx] > 2) return true; // křižovatka
        long long key = ((long long)roadpoints[idx].x << 32) ^ (unsigned int)roadpoints[idx].y;
        return pos_count[key] > 1; // duplicitní bod na stejné pozici
    };

    std::vector<roadpart> new_parts;
    new_parts.reserve((end_part_idx - start_part_idx) * 2);

    for (int pi = start_part_idx; pi < end_part_idx; pi++) {
        int p1 = roadparts[pi].p1;
        int p2 = roadparts[pi].p2;
        float width = roadparts[pi].width;

        // Segment se nedělí, pokud se dotýká křižovatky / duplicitního bodu.
        // Původní body se v takovém případě vůbec nehýbou ani neduplikují.
        if (is_locked(p1) || is_locked(p2)) {
            new_parts.push_back(roadparts[pi]); // beze změny, původní indexy
            continue;
        }

        Vec3 a = {(float)roadpoints[p1].x, (float)roadpoints[p1].h, (float)roadpoints[p1].y};
        Vec3 b = {(float)roadpoints[p2].x, (float)roadpoints[p2].h, (float)roadpoints[p2].y};

        // Sousedé pro Catmull-Rom (hledáme jen v rámci zpracovávaného úseku)
        Vec3 p0 = a, p3 = b;
        for (int pj = start_part_idx; pj < end_part_idx; pj++) {
            if (roadparts[pj].p2 == p1) {
                int np = roadparts[pj].p1;
                p0 = {(float)roadpoints[np].x, (float)roadpoints[np].h, (float)roadpoints[np].y};
                break;
            }
        }
        for (int pj = start_part_idx; pj < end_part_idx; pj++) {
            if (roadparts[pj].p1 == p2) {
                int np = roadparts[pj].p2;
                p3 = {(float)roadpoints[np].x, (float)roadpoints[np].h, (float)roadpoints[np].y};
                break;
            }
        }

        // Jediný nový bod uprostřed (t = 0.5), Catmull-Rom interpolace.
        // 'a' a 'b' se nikdy nemění - zůstávají na svých původních indexech.
        float tt = 0.5f, tt2 = tt*tt, tt3 = tt2*tt;
        float cx = 0.5f * ((2.0f*a.x) + (-p0.x + b.x)*tt + (2.0f*p0.x - 5.0f*a.x + 4.0f*b.x - p3.x)*tt2 + (-p0.x + 3.0f*a.x - 3.0f*b.x + p3.x)*tt3);
        float cz = 0.5f * ((2.0f*a.z) + (-p0.z + b.z)*tt + (2.0f*p0.z - 5.0f*a.z + 4.0f*b.z - p3.z)*tt2 + (-p0.z + 3.0f*a.z - 3.0f*b.z + p3.z)*tt3);
        float cy = 0.5f * ((2.0f*a.y) + (-p0.y + b.y)*tt + (2.0f*p0.y - 5.0f*a.y + 4.0f*b.y - p3.y)*tt2 + (-p0.y + 3.0f*a.y - 3.0f*b.y + p3.y)*tt3);

        cx = fmaxf(0.0f, fminf((float)MAP_SIZE, cx));
        cz = fmaxf(0.0f, fminf((float)MAP_SIZE, cz));

        roadpoint mid;
        mid.x = (int)cx;
        mid.y = (int)cz;
        mid.h = cy;
        int mid_idx = (int)roadpoints.size();
        roadpoints.push_back(mid); // pouze APPEND, nic starého se nepřepisuje

        roadpart r1, r2;
        r1.p1 = p1;      r1.p2 = mid_idx; r1.width = width;
        r2.p1 = mid_idx; r2.p2 = p2;      r2.width = width;
        new_parts.push_back(r1);
        new_parts.push_back(r2);
    }

    // Přepíšeme pouze segmenty v rozsahu [start,end) - VŠECHNY body (i ty
    // před/za rozsahem) zůstávají zcela netknuté, žádné indexy se neposouvají.
    roadparts.erase(roadparts.begin() + start_part_idx, roadparts.begin() + end_part_idx);
    roadparts.insert(roadparts.begin() + start_part_idx, new_parts.begin(), new_parts.end());

    roadpoints_len = (int)roadpoints.size();
    roadparts_len  = (int)roadparts.size();
}

// Omezí sklon (incline) silnice mezi po sobě jdoucími body v rozsahu
// [start_part_idx, end_part_idx) tak, aby nikde nepřekročil max_incline.
//
// max_incline: 0.0f = úplně rovně, 1.0f = 45°, obecně sklon = tan(uhel)
//              (výškový rozdíl / vodorovná vzdálenost)
//
// Křižovatky (bod sdílený >2 částmi silnice) a duplicitní body (stejná
// pozice, jiný index) se NIKDY nehýbou - fungují jako pevné kotvy, ke
// kterým se profil silnice "srovná". Volné body mezi kotvami se posouvají
// tak, aby žádný sousední pár nepřekročil povolený sklon.
// Omezí sklon (incline) mezi sousedními body v rozsahu [start_part_idx, end_part_idx).
//
// max_incline: 0.0f = rovně, 1.0f = 45° (sklon = výškový rozdíl / vodorovná vzdálenost)
//
// Nikdy nehýbe:
//   - prvním bodem rozsahu (roadparts[start_part_idx].p1)
//   - posledním bodem rozsahu (roadparts[end_part_idx-1].p2)
//   - žádnou křižovatkou (bod sdílený >2 částmi silnice)
//   - žádným duplicitním bodem (jiný index, stejná pozice)
// Všechny ostatní body mezi kotvami se opakovaně upravují, dokud
// celý úsek nesplňuje max_incline všude.
// Omezí NE absolutní sklon, ale ROZDÍL sklonu mezi dvěma po sobě jdoucími
// částmi silnice sdílejícími společný bod. Tím se odstraní ostré "zlomy"
// (náhlá změna sklonu - hrbol/prohlubeň), ale plynulé stoupání/klesání
// do kopce zůstane netknuté, protože tam je rozdíl sklonu mezi sousedními
// částmi malý.
//
// max_incline_delta: 0.0f = sklon musí být všude naprosto identický,
//                     1.0f = mezi sousedními částmi je povolená změna
//                            sklonu odpovídající 45°
//
// Nikdy nehýbe:
//   - prvním bodem rozsahu (roadparts[start_part_idx].p1)
//   - posledním bodem rozsahu (roadparts[end_part_idx-1].p2)
//   - křižovatkami (bod sdílený >2 částmi silnice)
//   - duplicitními body (jiný index, stejná pozice)
void limit_road_incline(int start_part_idx, int end_part_idx, float max_incline_delta,
                               int max_iterations = 200) {
    if (start_part_idx >= end_part_idx) return;
    if (start_part_idx < 0 || end_part_idx > roadparts_len) return;
    if (max_incline_delta < 0.0f) return;
    std::vector<int> point_degree(roadpoints.size(), 0);
    for (int i = 0; i < roadparts_len; i++) {
        point_degree[roadparts[i].p1]++;
        point_degree[roadparts[i].p2]++;
    }

    std::unordered_map<long long, int> pos_count;
    pos_count.reserve(roadpoints.size() * 2);
    for (int i = 0; i < (int)roadpoints.size(); i++) {
        long long key = ((long long)roadpoints[i].x << 32) ^ (unsigned int)roadpoints[i].y;
        pos_count[key]++;
    }

    int first_pt = roadparts[start_part_idx].p1;
    int last_pt  = roadparts[end_part_idx - 1].p2;

    auto is_locked = [&](int idx) {
        if (idx == first_pt || idx == last_pt) return true; // hranice úseku
        if (point_degree[idx] > 2) return true;              // křižovatka
        long long key = ((long long)roadpoints[idx].x << 32) ^ (unsigned int)roadpoints[idx].y;
        return pos_count[key] > 1;                            // duplicitní pozice
    };

    auto horiz_dist = [&](int a, int b) {
        float dx = (float)roadpoints[b].x - (float)roadpoints[a].x;
        float dz = (float)roadpoints[b].y - (float)roadpoints[a].y;
        return sqrtf(dx * dx + dz * dz);
    };

    // Najde v rozsahu [start,end) index části, jejíž p1 == idx (sousední
    // část navazující ZA bodem idx). Vrací -1, pokud žádná neexistuje
    // (idx je konec úseku).
    auto find_part_starting_at = [&](int idx) {
        for (int pj = start_part_idx; pj < end_part_idx; pj++) {
            if (roadparts[pj].p1 == idx) return pj;
        }
        return -1;
    };

    for (int iter = 0; iter < max_iterations; iter++) {
        bool changed = false;

        for (int pi = start_part_idx; pi < end_part_idx; pi++) {
            int mid = roadparts[pi].p2;
            if (is_locked(mid)) continue; // tenhle bod se nesmí hýbat

            int next_pi = find_part_starting_at(mid);
            if (next_pi == -1) continue; // konec řetězce v tomto rozsahu

            int prev_pt = roadparts[pi].p1;
            int next_pt = roadparts[next_pi].p2;

            float d_in  = horiz_dist(prev_pt, mid);
            float d_out = horiz_dist(mid, next_pt);
            if (d_in < 0.01f || d_out < 0.01f) continue;

            float slope_in  = (roadpoints[mid].h     - roadpoints[prev_pt].h) / d_in;
            float slope_out = (roadpoints[next_pt].h - roadpoints[mid].h)     / d_out;
            float delta = slope_out - slope_in;

            if (fabsf(delta) <= max_incline_delta + 0.0001f) continue; // v pořádku

            // Chceme nový sklon mid->next takový, aby rozdíl vůči sklonu
            // prev->mid byl přesně na povolené hranici (se stejným
            // znaménkem jako původní zlom, jen useknutý na limit).
            float sign = (delta > 0.0f) ? 1.0f : -1.0f;
            float target_delta = sign * max_incline_delta;

            // slope_out_new - slope_in = target_delta, slope_in závisí na
            // novém h[mid] taky, takže řešíme přímo pro h[mid]:
            // (next.h - h)/d_out - (h - prev.h)/d_in = target_delta
            float inv_sum = 1.0f / (1.0f / d_out + 1.0f / d_in);
            float new_h = (roadpoints[next_pt].h / d_out + roadpoints[prev_pt].h / d_in - target_delta) * inv_sum;

            roadpoints[mid].h = new_h;
            changed = true;
        }

        if (!changed) break; // celý úsek už má plynulé přechody sklonu
    }
}
#include <queue>
#include <set>
#include <cmath>
#include <vector>
#include <algorithm>

// Pomocná struktura pro A* uzel
struct RoadNode {
    Vec3 pos;
    float last_angle;
    float g_score;
    float f_score;
    int parent_idx;
    int self_idx; // <-- add this
    int connect_to_existing_idx = -1;
    bool operator>(const RoadNode& o) const { return f_score > o.f_score; }
};

// Pomocná struktura pro označení navštívených pozic v diskrétním prostoru (gridu)
struct GridPos {
    int x, z;
    bool operator<(const GridPos& other) const {
        if (x != other.x) return x < other.x;
        return z < other.z;
    }
};

void gen_road_fallback_fallback(Vec3 road_start, Vec3 road_end, float width, float roadpart_size = 100.0f) {
    major_road_segments.push_back({2,{{road_start.z,road_start.x},{road_end.z,road_end.x}}});
    std::vector<Vec3> path;
    path.push_back(road_start);

    // Rekurzivní dělení s pojistkou proti nekonečnému zacyklení (depth)
    std::function<void(Vec3, Vec3, int)> subdivide = [&](Vec3 p1, Vec3 p2, int depth) {
        // 1. POJISTKA: Počítáme vzdálenost POUZE v 2D (XZ), aby terén nemohl zvětšit vzdálenost
        float dx = p2.x - p1.x;
        float dz = p2.z - p1.z;
        float dist_2d = sqrtf(dx * dx + dz * dz);

        // 2. POJISTKA: Maximální hloubka rekurze 10 zamezí Stack Overflow
        if (dist_2d <= roadpart_size || depth >= 10) {
            return;
        }

        Vec3 mid;
        mid.x = (p1.x + p2.x) * 0.5f;
        mid.z = (p1.z + p2.z) * 0.5f;

        float terrain_y = mapgen_get_heightmap_height(mid.z, mid.x);
        float interp_y = (p1.y + p2.y) * 0.5f;
        mid.y = (terrain_y + interp_y) * 0.5f; 

        subdivide(p1, mid, depth + 1);
        path.push_back(mid);
        subdivide(mid, p2, depth + 1);
    };

    subdivide(road_start, road_end, 0);
    path.push_back(road_end);

    // Bezpečný zápis do globální sítě
    int start_part_idx = (int)roadparts.size();
    int prev_global_idx = -1;

    for (size_t i = 0; i < path.size(); i++) {
        // Bezpečné vložení bodu bez rizika špatného indexu
        roadpoints.push_back({});
        int this_global_idx = (int)roadpoints.size() - 1;
        
        roadpoints[this_global_idx].x = path[i].x;
        roadpoints[this_global_idx].y = path[i].z;
        roadpoints[this_global_idx].h = path[i].y;
        roadpoints_len = (int)roadpoints.size();

        if (i > 0 && prev_global_idx != this_global_idx) {
            roadparts.push_back({});
            int part_idx = (int)roadparts.size() - 1;
            
            roadparts[part_idx].p1 = prev_global_idx;
            roadparts[part_idx].p2 = this_global_idx;
            roadparts[part_idx].width = width;
            roadparts_len = (int)roadparts.size();
        }

        prev_global_idx = this_global_idx;
    }

    Vec2 msp = {road_start.x, road_start.z};
    Vec2 mep = {road_end.x, road_end.z};
    major_road_points.push_back(msp);
    major_road_points.push_back(mep);

    int end_part_idx = (int)roadparts.size();
    limit_road_incline(start_part_idx, end_part_idx, 0.1f);
}
void gen_road_fallback(Vec3 road_start, Vec3 road_end, float width, float roadpart_size=100.0f) {
    printf("gen road fallback\n");
    int initial_roadparts_len = roadparts_len;
    // --- NASTAVENÍ LIMITŮ ---
    const float MAX_HEIGHT_DIFF = 10.0f; // Maximální stoupání na jeden krok (roadpart_size)
    const float MAX_TUNNEL_DEPTH = 7.0f; // Max metrů pod zemí
    const float MAX_BRIDGE_HEIGHT = 80.0f; // Max výška mostu nad terénem
    const int DIRECTIONS = 16;           // Počet směrů k prozkoumání v každém kroku
    const float JUNCTION_SNAP_DIST = 100.0f; // Max vzdálenost pro napojení na existující silnici

    // Fronta pro otevřené uzly (Open Set) a pole pro všechny vygenerované uzly
    std::priority_queue<RoadNode, std::vector<RoadNode>, std::greater<RoadNode>> open_set;
    std::vector<RoadNode> all_nodes;
    std::set<GridPos> closed_set;

    // Vložení startovního bodu
    RoadNode start_node;
    start_node.pos = road_start;
    start_node.last_angle = get_angle(road_start.x, road_start.z, road_end.x, road_end.z);
    start_node.g_score = 0.0f;
    start_node.f_score = get_dist(road_start, road_end);
    start_node.parent_idx = -1;
    start_node.self_idx = 0;

    open_set.push(start_node);
    all_nodes.push_back(start_node);

    int target_node_idx = -1;
    int iterations = 0;

    // Hlavní A* smyčka (BEZE ZMĚNY oproti originálu)
    while (!open_set.empty() && iterations < 30000) {
        iterations++;
        RoadNode current = open_set.top();
        open_set.pop();

        int current_idx = current.self_idx;

        // Kontrola, zda jsme blízko cíle
        if (get_dist(current.pos, road_end) < roadpart_size * 1.5f) {
            target_node_idx = current_idx;
            break;
        }

        // Označení pozice jako zpracované (Closed Set)
        GridPos c_pos = {(int)(current.pos.x / roadpart_size), (int)(current.pos.z / roadpart_size)};
        if (closed_set.count(c_pos)) continue;
        closed_set.insert(c_pos);

        // Zkoumáme sousedy (všech 16 směrů)
        for (int i = 0; i < DIRECTIONS; i++) {
            float angle = (PI * 2.0f / (float)DIRECTIONS) * (float)i;

            // Zabránění otočení o 180 stupňů (hladkost silnice)
            if (cosf(angle - current.last_angle) < -0.3f) continue;

            float xpos = current.pos.x + sinf(angle) * roadpart_size;
            float zpos = current.pos.z + cosf(angle) * roadpart_size;

            // Kontrola hranic mapy
            if (xpos < 0 || xpos > MAP_SIZE || zpos < 0 || zpos > MAP_SIZE) continue;

            // Výška skutečného terénu v tomto bodě
            float terrain_y = mapgen_get_heightmap_height(zpos, xpos);
            float target_y = terrain_y;

            // --- LOGIKA PRO MOSTY A TUNELY ---
            float height_diff_to_terrain = target_y - current.pos.y;

            if (height_diff_to_terrain > MAX_HEIGHT_DIFF) {
                float ideal_y = current.pos.y + MAX_HEIGHT_DIFF;
                if (terrain_y - ideal_y <= MAX_TUNNEL_DEPTH) {
                    target_y = ideal_y;
                } else {
                    continue;
                }
            }
            else if (height_diff_to_terrain < -MAX_HEIGHT_DIFF) {
                float ideal_y = current.pos.y - MAX_HEIGHT_DIFF;
                if (ideal_y - terrain_y <= MAX_BRIDGE_HEIGHT) {
                    target_y = ideal_y;
                } else {
                    target_y = terrain_y + MAX_HEIGHT_DIFF;
                }
            }
            else {
                target_y = terrain_y;
            }

            if (fabs(current.pos.y - target_y) > MAX_HEIGHT_DIFF) continue;

            float step_dist = sqrtf(roadpart_size * roadpart_size + (current.pos.y - target_y) * (current.pos.y - target_y));
            float g_score = current.g_score + step_dist;

            Vec3 next_pos = {xpos, target_y, zpos};
            float h_score = get_dist(next_pos, road_end);

            if (fabs(target_y - terrain_y) > 1.0f) {
                g_score += roadpart_size * 0.5f;
            }

            RoadNode neighbor;
            neighbor.pos = next_pos;
            neighbor.last_angle = angle;
            neighbor.g_score = g_score;
            neighbor.f_score = g_score + h_score;
            neighbor.parent_idx = current_idx;
            neighbor.self_idx = (int)all_nodes.size();

            open_set.push(neighbor);
            all_nodes.push_back(neighbor);
        }
    }

    if (target_node_idx == -1) {
        gen_road_fallback_fallback(road_start, road_end, width, roadpart_size);
        return;
    }

    if (target_node_idx != -1) {

        std::vector<Vec3> final_path;
        int curr = target_node_idx;
        while (curr != -1) {
            final_path.push_back(all_nodes[curr].pos);
            curr = all_nodes[curr].parent_idx;
        }
        std::reverse(final_path.begin(), final_path.end());

        // Připojíme koncový bod POSTUPNĚ, ne jedním skokem (stejné jako předtím)
        Vec3 last_found = final_path.back();
        float height_gap = fabs(road_end.y - last_found.y);
        int extra_steps = (int)ceilf(height_gap / MAX_HEIGHT_DIFF);
        if (extra_steps < 1) extra_steps = 1;
        for (int s = 1; s <= extra_steps; s++) {
            float t = (float)s / (float)extra_steps;
            Vec3 p;
            p.x = last_found.x + (road_end.x - last_found.x) * t;
            p.z = last_found.z + (road_end.z - last_found.z) * t;
            p.y = last_found.y + (road_end.y - last_found.y) * t;
            final_path.push_back(p);
        }
        // ... hned pod cyklem for (int s = 1; s <= extra_steps; s++), který plní final_path ...

        // --- KONTROLA KVALITY A* TRASY ---
        float straight_dist = get_dist(road_start, road_end);
        float actual_path_len = 0.0f;
        for (size_t k = 1; k < final_path.size(); k++) {
            actual_path_len += get_dist(final_path[k - 1], final_path[k]);
        }

        // Pokud A* vygeneroval extrémní obchvat / klikatici, zahodíme to a voláme rekurzi
        if (actual_path_len > straight_dist * 2.2f) {
            gen_road_fallback_fallback(road_start, road_end, width, roadpart_size);
            return;
        }
        // =============================================================
        // === BUFFER + NAPOJOVÁNÍ NA EXISTUJÍCÍ SILNICE ===
        // =============================================================

        // Nejbližší bod na úsečce A-B k bodu P (jen v půdoryse XZ, výška
        // se lineárně interpoluje). out_t = 0..1 pozice na úsečce.
        auto closest_point_on_segment = [](Vec3 A, Vec3 B, Vec3 P, float& out_t) -> Vec3 {
            float abx = B.x - A.x;
            float abz = B.z - A.z;
            float apx = P.x - A.x;
            float apz = P.z - A.z;
            float ab_len_sq = abx * abx + abz * abz;
            float t = (ab_len_sq > 0.0001f) ? (apx * abx + apz * abz) / ab_len_sq : 0.0f;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;
            out_t = t;
            Vec3 result;
            result.x = A.x + abx * t;
            result.z = A.z + abz * t;
            result.y = A.y + (B.y - A.y) * t;
            return result;
        };

        struct SnapResult { bool found; Vec3 point; int part_idx; float t; };

        // Najde nejbližší bod na kterékoli existující roadpart do JUNCTION_SNAP_DIST.
        // (Prochází všechny existující roadparts - u opravdu velkých silničních sítí
        // by šlo zrychlit prostorovou mřížkou, ale tohle se volá jen jednou na
        // bod nové silnice, takže by to mělo být v pohodě.)
        auto find_nearest_existing = [&](Vec3 pos) -> SnapResult {
            SnapResult best;
            best.found = false;
            best.part_idx = -1;
            float best_dist_sq = JUNCTION_SNAP_DIST * JUNCTION_SNAP_DIST;

            // PŮVODNĚ: for (int pi = 0; pi < roadparts_len; pi++)
            for (int pi = 0; pi < initial_roadparts_len; pi++) { // <-- ZMĚNA ZDEMA
                int i1 = roadparts[pi].p1;
                int i2 = roadparts[pi].p2;

                Vec3 A, B;
                A.x = roadpoints[i1].x; A.y = roadpoints[i1].h; A.z = roadpoints[i1].y;
                B.x = roadpoints[i2].x; B.y = roadpoints[i2].h; B.z = roadpoints[i2].y;

                float t;
                Vec3 cp = closest_point_on_segment(A, B, pos, t);

                float dx = cp.x - pos.x;
                float dz = cp.z - pos.z;
                float dsq = dx * dx + dz * dz; // vzdálenost v půdoryse

                if (dsq < best_dist_sq) {
                    best_dist_sq = dsq;
                    best.found = true;
                    best.point = cp;
                    best.part_idx = pi;
                    best.t = t;
                }
            }
            return best;
        };

        // Buffer nové silnice
        std::vector<Vec3> buf_points = final_path;
        std::vector<int> buf_snap_idx(buf_points.size(), -1); // -1 = nový bod, jinak globální index v roadpoints
        // --- ZÁLOHA PRO PRACNÝ ROLLBACK V PŘÍPADĚ NEPRŮCHODNOSTI ---
        struct PartBackup { int part_idx; int old_p2; };
        std::vector<PartBackup> modified_parts;

        int backup_roadpoints_len = roadpoints_len;
        int backup_roadparts_len = roadparts_len;
        size_t backup_roadpoints_size = roadpoints.size();
        size_t backup_roadparts_size = roadparts.size();
        for (size_t i = 0; i < buf_points.size(); i++) {
            // --- PŘIDAT TYTO 2 POJISTKY ---
            if (i < 2) continue; // Ignorovat první 2 body (vymezení z vesnice)

            SnapResult r = find_nearest_existing(buf_points[i]);
            if (!r.found) continue;

            // --- NOVÁ POJISTKA PROTI SMYČKÁM VE VESNICI ---
            // Pokud je bod napojení moc blízko startu, ignorovat (ze startu už jsme vyjeli)
            if (get_dist(r.point, road_start) < 250.0f) continue; 
            // ----------------------------------------------

            if (fabs(buf_points[i].y - r.point.y) > MAX_HEIGHT_DIFF * 1.5f) continue;


            // ... zbytek tvého původního kódu
            int i1 = roadparts[r.part_idx].p1;
            int i2 = roadparts[r.part_idx].p2;
            Vec3 A, B;
            A.x = roadpoints[i1].x; A.y = roadpoints[i1].h; A.z = roadpoints[i1].y;
            B.x = roadpoints[i2].x; B.y = roadpoints[i2].h; B.z = roadpoints[i2].y;
            float seg_len = get_dist(A, B);
            const float ENDPOINT_EPS = 5.0f; // v metrech
            float t_eps = (seg_len > 0.001f) ? (ENDPOINT_EPS / seg_len) : 0.5f;

            int global_idx;
            if (r.t <= t_eps) {
                // Blízko začátku úsečky -> napoj se přímo na existující vrchol
                global_idx = i1;
            } else if (r.t >= 1.0f - t_eps) {
                // Blízko konce úsečky -> napoj se přímo na existující vrchol
                global_idx = i2;
            } else {
                // Rozděl existující díl silnice, vlož nový bod přesně v místě napojení
                roadpoints.push_back({});
                roadpoints[roadpoints_len].x = r.point.x;
                roadpoints[roadpoints_len].y = r.point.z;
                roadpoints[roadpoints_len].h = r.point.y;
                global_idx = roadpoints_len;
                roadpoints_len++;

                int old_p2 = roadparts[r.part_idx].p2;
                float old_width = roadparts[r.part_idx].width;

                // --- PŘIDAT TENTO ŘÁDEK (Záloha pro případný rollback) ---
                modified_parts.push_back({r.part_idx, old_p2});

                roadparts[r.part_idx].p2 = global_idx; // zkrátíme původní díl po nový bod


                roadparts.push_back({});
                roadparts[roadparts_len].p1 = global_idx;
                roadparts[roadparts_len].p2 = old_p2;
                roadparts[roadparts_len].width = old_width;
                roadparts_len++;
            }

            buf_snap_idx[i] = global_idx;
        }


        int start_part_idx = (int)roadparts.size();
        int prev_global_idx = -1;

        int graph_start_idx = -1;
        int graph_end_idx = -1;

        for (size_t i = 0; i < buf_points.size(); i++) {
            int this_global_idx;
            // ... tvůj původní kód pro získání/vytvoření this_global_idx ...

            // --- PŘIDAT TYTO DVA ŘÁDKY ---
            if (i == 0) graph_start_idx = this_global_idx;
            if (i == buf_points.size() - 1) graph_end_idx = this_global_idx;



            if (buf_snap_idx[i] != -1) {
                // Napojení na existující (případně čerstvě rozdělený) bod -
                // nevytváříme duplicitu, jen použijeme jeho index.
                this_global_idx = buf_snap_idx[i];
            } else {
                roadpoints.push_back({});
                roadpoints[roadpoints_len].x = buf_points[i].x;
                roadpoints[roadpoints_len].y = buf_points[i].z;
                roadpoints[roadpoints_len].h = buf_points[i].y;
                this_global_idx = roadpoints_len;
                roadpoints_len++;
            }

            if (i > 0 && prev_global_idx != this_global_idx) {
                roadparts.push_back({});
                roadparts[roadparts_len].p1 = prev_global_idx;
                roadparts[roadparts_len].p2 = this_global_idx;
                roadparts[roadparts_len].width = width;
                roadparts_len++;
            }

            prev_global_idx = this_global_idx;
        }
        // =============================================================
        // === OVERENI PRUCHODNOSTI V GRAFU (BFS) ===
        // =============================================================
        bool path_exists = false;

        // Pokud start a cíl neskončily na stejném bodě (při vzdáleném cíli)
        if (graph_start_idx != -1 && graph_end_idx != -1) {
            if (graph_start_idx == graph_end_idx && get_dist(road_start, road_end) > 10.0f) {
                path_exists = false; // Start i cíl se snapnuly do jednoho bodu = smyčka!
            } else {
                // Sestavení sousedství (Adjacency List)
                std::vector<std::vector<int>> adj(roadpoints_len);
                for (int pi = 0; pi < roadparts_len; pi++) {
                    int u = roadparts[pi].p1;
                    int v = roadparts[pi].p2;
                    if (u >= 0 && u < roadpoints_len && v >= 0 && v < roadpoints_len) {
                        adj[u].push_back(v);
                        adj[v].push_back(u);
                    }
                }

                // BFS prohledávání sítě
                std::queue<int> q;
                std::vector<bool> visited(roadpoints_len, false);
                q.push(graph_start_idx);
                visited[graph_start_idx] = true;

                while (!q.empty()) {
                    int curr = q.front();
                    q.pop();

                    if (curr == graph_end_idx) {
                        path_exists = true;
                        break; // Cesta v grafu existuje!
                    }

                    for (int neighbor : adj[curr]) {
                        if (!visited[neighbor]) {
                            visited[neighbor] = true;
                            q.push(neighbor);
                        }
                    }
                }
            }
        }

        // --- POKUD PŘES VŠECHNY SILNICE NELZE DOJED Z A DO B ---
        if (!path_exists) {
            // 1. Vrátíme zpět rozdělené úseky
            for (const auto& mod : modified_parts) {
                roadparts[mod.part_idx].p2 = mod.old_p2;
            }
            // 2. Ořízneme pole na stav před spuštěním A*
            roadpoints.resize(backup_roadpoints_size);
            roadparts.resize(backup_roadparts_size);
            roadpoints_len = backup_roadpoints_len;
            roadparts_len = backup_roadparts_len;

            // 3. Zavoláme rekurzivní jistotu
            gen_road_fallback_fallback(road_start, road_end, width, roadpart_size);
            return;
        }

    
        // Přidání hlavních bodů cesty
        Vec2 msp = {road_start.x, road_start.z};
        Vec2 mep = {road_end.x, road_end.z};
        major_road_points.push_back(msp);
        major_road_points.push_back(mep);

        // Závěrečné vyhlazení cesty (Subdivision)
        int end_part_idx = (int)roadparts.size();
        limit_road_incline(start_part_idx, end_part_idx, 0.1f); // max sklon 0.5 = 26.565°
        major_road_segments.push_back({1,{{road_start.z,road_start.x},{road_end.z,road_end.x}}});

    }
}
bool can_make_ending(Vec3 start, Vec3 end){
    return true;
}
bool is_near_roadpoint_before_idx(Vec3 pos, float dist_sq, int start_point_idx, int end_point_idx) {
    
    // Pojistka, abychom neprohledávali mimo hranice pole
    int max_check = (end_point_idx < roadpoints_len) ? end_point_idx : roadpoints_len;

    for (int k = start_point_idx; k < max_check; k++) {
        float dx = roadpoints[k].x - pos.x;
        float dz = roadpoints[k].y - pos.z; // V roadpoints je y uloženo jako Z souřadnice

        if ((dx * dx + dz * dz) < dist_sq) {
            return true; // Našli jsme bod, který je blíže než 'dist'
        }
    }

    return false; // Žádný bod v daném rozsahu není dostatečně blízko
}
// Pomocná funkce pro vyhodnocení jednoho kroku/kandidáta
bool evaluate_candidate(
    Vec3 from_pos,
    float from_angle,
    float angle,
    Vec3 road_start,
    Vec3 road_end,
    float start_point_idx,
    float roadpart_size,
    float max_height_diff,
    float min_relative_angle_score,
    float min_ang_score,
    const float weights[4],
    bool calculate_absolute_height_score,
    Vec3& out_pos,
    float& out_score
) {
    float looping_dist=roadpart_size*1.5f;
    // 0. kontrola loopingu
    if (is_near_roadpoint_before_idx({from_pos.x + sinf(angle) * roadpart_size, from_pos.y, from_pos.z + cosf(angle) * roadpart_size}, looping_dist*looping_dist, start_point_idx, roadpoints_len-1)) {
        return false; // Kandidát je příliš blízko existujícího bodu silnice
    }
    // 1. Kontrola úhlu zatáčení
    float relative_angle_score = cosf(angle - from_angle);
    if (relative_angle_score < min_relative_angle_score) return false;

    // 2. Výpočet pozice a kontrola hranic mapy
    float xpos = from_pos.x + sinf(angle) * roadpart_size;
    float zpos = from_pos.z + cosf(angle) * roadpart_size;
    if (xpos < 0 || xpos > MAP_SIZE || zpos < 0 || zpos > MAP_SIZE) return false;

    // 3. Výška a kontrola max. převýšení
    float ypos = mapgen_get_heightmap_height(zpos, xpos);
    if (fabs(from_pos.y - ypos) > max_height_diff) return false;

    // 4. Výpočet úhlu k cíli z aktuální pozice
    float target_angle = get_angle(from_pos.x, from_pos.z, road_end.x, road_end.z);
    float absolute_angle_score = cosf(angle - target_angle);
    float relative_height_score = 1.0f - fminf(1.0f, fabs(from_pos.y - ypos) / max_height_diff);

    // 5. Výpočet absolutního výškového profilu
    float absolute_height_score = 0.0f;
    if (calculate_absolute_height_score) {
        float total_len = get_dist({xpos, ypos, zpos}, road_end) + get_dist({xpos, ypos, zpos}, road_start);
        float progress = get_dist({xpos, ypos, zpos}, road_start) / total_len;
        float h_progress = (ypos - road_start.y) / (road_end.y - road_start.y);
        absolute_height_score = 1.0f - fabsf(h_progress - progress);
    }

    // 6. Vážené skóre
    float score = weights[0] * relative_height_score + 
                  weights[1] * relative_angle_score + 
                  weights[2] * absolute_angle_score;

    if (calculate_absolute_height_score) {
        score += weights[3] * absolute_height_score;
    }

    if (absolute_angle_score < min_ang_score) {
        score = absolute_angle_score;
    }

    out_pos = {xpos, ypos, zpos};
    out_score = score;
    return true;
}
bool gggen_road(Vec3 road_start, Vec3 road_end, float width, float roadpart_size, float w1, float w2, float w3, float w4, float min_relative_angle_score, float max_height_diff, float min_ang_score, int angle_attempts, int beam_width, int lookahead, bool calculate_absolute_height_score=true) {
    printf(RESET"        gggenerating road...\n");
    float weights[] = {w1, w2, w3, w4};
    bool uspech = false;

    // --- ZÁLOHA STAVU PRO PŘÍPADNÝ ROLLBACK A OŘEZÁNÍ ---
    int start_part_idx = (int)roadparts.size();
    int initial_roadpoints_idx = roadpoints_len;
    // ------------------------------------------
    printf("        creating road, start part IDX=%d\n", start_part_idx);
    Vec3 last_pos  = road_start;
    float last_angle = get_angle(road_start.x, road_start.z, road_end.x, road_end.z);
    roadpoints.push_back({});
    roadpoints[roadpoints_len].x = road_start.x;
    roadpoints[roadpoints_len].y = road_start.z;
    roadpoints[roadpoints_len].h = road_start.y;
    #ifdef ROAD_LOGGING
    printf("        added road start point: %f %f %f\n", road_start.x, road_start.y, road_start.z);
    #endif
    roadpoints_len++;
    int iterations = 0;

    while (iterations < get_dist(road_start, road_end) / roadpart_size * 10.0f) {
        if (get_dist(last_pos, road_end) < roadpart_size) {
            // 1. Přidáme finální bod přímo na pozici road_end
            roadpoints.push_back({});
            roadpoints[roadpoints_len].x = road_end.x;
            roadpoints[roadpoints_len].y = road_end.z;
            roadpoints[roadpoints_len].h = road_end.y;

            // 2. Propojíme předchozí bod s tímto cílovým bodem
            roadparts.push_back({});
            roadparts[roadparts_len].p1 = roadpoints_len - 1;
            roadparts[roadparts_len].p2 = roadpoints_len;
            roadparts[roadparts_len].width = width;

            roadpoints_len++;
            roadparts_len++;

            // 3. Označíme generování jako ÚSPĚŠNÉ
            uspech = true;
            break;
        }

        // --- 1. KROK BEAM SEARCH (PRVNÍ GENERACE) ---
        std::vector<Candidate> beam;
        for (int j = 0; j < angle_attempts; j++) {
            float angle = (PI * 2.0f / (float)angle_attempts) * (float)j;
            Vec3 new_pos;
            float score = 0.0f;

            if (!evaluate_candidate(last_pos, last_angle, angle, road_start, road_end, initial_roadpoints_idx,
                                   roadpart_size, max_height_diff, min_relative_angle_score,
                                   min_ang_score, weights, calculate_absolute_height_score,
                                   new_pos, score)) {
                continue;
            }

            Candidate c;
            c.pos = new_pos;
            c.last_angle = angle;
            c.total_score = score;
            c.path.push_back(c.pos);
            beam.push_back(c);
        }

        if (beam.empty()) { uspech = false; printf("beam is empty\n"); break; }

        // --- LOOKAHEAD SMYČKA (DALŠÍ KROKY) ---
        for (int step = 1; step < lookahead; step++) {
            std::vector<Candidate> next_beam;

            for (auto& cand : beam) {
                for (int j = 0; j < angle_attempts; j++) {
                    float angle = (PI * 2.0f / (float)angle_attempts) * (float)j;
                    Vec3 new_pos;
                    float score = 0.0f;

                    if (!evaluate_candidate(cand.pos, cand.last_angle, angle, road_start, road_end, initial_roadpoints_idx,
                                           roadpart_size, max_height_diff, min_relative_angle_score,
                                           min_ang_score, weights, calculate_absolute_height_score,
                                           new_pos, score)) {
                        continue;
                    }

                    Candidate nc;
                    nc.pos = new_pos;
                    nc.last_angle = angle;
                    nc.total_score = cand.total_score + score;
                    nc.path = cand.path;
                    nc.path.push_back(nc.pos);
                    next_beam.push_back(nc);
                }
            }

            if (next_beam.empty()) { break; }

            std::sort(next_beam.begin(), next_beam.end(), [](const Candidate& a, const Candidate& b) {
                return a.total_score > b.total_score;
            });

            if ((int)next_beam.size() > beam_width)
                next_beam.resize(beam_width);

            beam = next_beam;
        }

        if (beam.empty()) { uspech = false; printf("beam is empty\n"); break; }

        Vec3 next = beam[0].path[0];

        float dx;
        float dz;
        bool touching = false;
        int touching_index = -1;
        for (int k = 0; k < roadpoints_len - 2; k++) {
            dx = roadpoints[k].x - next.x;
            dz = roadpoints[k].y - next.z;
            if (sqrtf(dx*dx + dz*dz) < roadpart_size * 2.0f) {
                touching = true;
                touching_index = k;
                break;
            }
        }
        if (touching && !(roadpoints[touching_index].x==road_start.x && roadpoints[touching_index].y==road_start.z) && get_dist(road_start,last_pos)>get_dist(road_start,road_end)/3.0f) {
            if (touching_index>=start_part_idx){
                printf("        touching index<=start part idx\n");
                uspech=false;
                break;
            }
            printf("        connected to point %d at %d,%d after %d iterations\n", touching_index, (int)roadpoints[touching_index].x, (int)roadpoints[touching_index].y, iterations);
            int start_point_idx = roadpoints_len - 1;
            int end_point_idx = touching_index;

            roadpoints.push_back({});
            int p_mid1 = roadpoints_len++;
            roadpoints[p_mid1].x = roadpoints[start_point_idx].x + (roadpoints[end_point_idx].x - roadpoints[start_point_idx].x) * (1.0f / 3.0f);
            roadpoints[p_mid1].y = roadpoints[start_point_idx].y + (roadpoints[end_point_idx].y - roadpoints[start_point_idx].y) * (1.0f / 3.0f);
            roadpoints[p_mid1].h = ((roadpoints[start_point_idx].h + (roadpoints[end_point_idx].h - roadpoints[start_point_idx].h) * (2.0f / 3.0f)) * 0.2f)+(mapgen_get_heightmap_height(roadpoints[p_mid1].y, roadpoints[p_mid1].x)*0.8f);

            roadpoints.push_back({});
            int p_mid2 = roadpoints_len++;
            roadpoints[p_mid2].x = roadpoints[start_point_idx].x + (roadpoints[end_point_idx].x - roadpoints[start_point_idx].x) * (2.0f / 3.0f);
            roadpoints[p_mid2].y = roadpoints[start_point_idx].y + (roadpoints[end_point_idx].y - roadpoints[start_point_idx].y) * (2.0f / 3.0f);
            roadpoints[p_mid2].h = ((roadpoints[start_point_idx].h + (roadpoints[end_point_idx].h - roadpoints[start_point_idx].h) * (2.0f / 3.0f)) * 0.3f)+(roadpoints[touching_index].h*0.7f);

            roadparts.push_back({});
            roadparts[roadparts_len].p1 = start_point_idx;
            roadparts[roadparts_len].p2 = p_mid1;
            roadparts[roadparts_len].width = width;
            roadparts_len++;

            roadparts.push_back({});
            roadparts[roadparts_len].p1 = p_mid1;
            roadparts[roadparts_len].p2 = p_mid2;
            roadparts[roadparts_len].width = width;
            roadparts_len++;

            roadparts.push_back({});
            roadparts[roadparts_len].p1 = p_mid2;
            roadparts[roadparts_len].p2 = end_point_idx;
            roadparts[roadparts_len].width = width;
            roadparts_len++;

            float sign_angle = get_angle((float)roadpoints[touching_index].x,
                                        (float)roadpoints[touching_index].y,
                                        (float)roadpoints[start_point_idx].x,
                                        (float)roadpoints[start_point_idx].y) * RAD_DEG;
            
            add_stopsign((float)roadpoints[touching_index].y,
                        roadpoints[touching_index].h,
                        (float)roadpoints[touching_index].x,
                        sign_angle);
            uspech=true;

            break;
        }

        roadpoints.push_back({});
        roadparts.push_back({});
        roadpoints[roadpoints_len].x = next.x;
        roadpoints[roadpoints_len].y = next.z;
        roadpoints[roadpoints_len].h = next.y;
        roadparts[roadparts_len].p1    = roadpoints_len - 1;
        roadparts[roadparts_len].p2    = roadpoints_len;
        roadparts[roadparts_len].width = width;
        roadpoints_len++;
        roadparts_len++;
        iterations++;
        #ifdef ROAD_LOGGING
        printf(YELLOW"        added point %d at %d,%d,%d after %d iterations\n" RESET, roadpoints_len-1, (int)next.x, (int)next.y, (int)next.z, iterations);
        #endif
        last_angle = get_angle(last_pos.x, last_pos.z, next.x, next.z);
        last_pos   = next;
    }

    // =========================================================================
    // POST-PROCESSING: MULTI-SNAP
    // =========================================================================
    if (uspech && start_part_idx > 0 && false) {
        printf(GREEN "        uspech && start_part_idx > 0, roadpoints_len=%d, roadparts_len=%d\n" RESET, roadpoints_len, roadparts_len);
        const float SNAP_THRESHOLD = roadpart_size * 1.5f;
        const float ENDPOINT_EPS = 5.0f;
        const int initial_old_parts_count = start_part_idx;

        auto closest_point_on_segment = [](Vec3 A, Vec3 B, Vec3 P, float& out_t) -> Vec3 {
            float abx = B.x - A.x, abz = B.z - A.z;
            float apx = P.x - A.x, apz = P.z - A.z;
            float ab_len_sq = abx * abx + abz * abz;
            float t = (ab_len_sq > 0.0001f) ? (apx * abx + apz * abz) / ab_len_sq : 0.0f;
            t = fmaxf(0.0f, fminf(1.0f, t));
            out_t = t;
            return {A.x + abx * t, A.y + (B.y - A.y) * t, A.z + abz * t};
        };

        int new_points_start = roadparts[start_part_idx].p1;
        int current_new_points_len = roadpoints_len;

        std::vector<int> snapped_junction_idx(current_new_points_len, -1);
        std::vector<bool> is_snapped(current_new_points_len, false);

        // 1. KROK: DETEKCE A SNAPPING VŠECH BODŮ PODÉL TRASY
        for (int p_idx = new_points_start; p_idx < current_new_points_len; p_idx++) {
            Vec3 p_pos = {(float)roadpoints[p_idx].x, (float)roadpoints[p_idx].h, (float)roadpoints[p_idx].y};

            int best_old_part = -1;
            Vec3 best_cp;
            float best_t = 0.0f;
            float min_dsq = SNAP_THRESHOLD * SNAP_THRESHOLD;

            for (int old_part = 0; old_part < initial_old_parts_count; old_part++) {
                int i1 = roadparts[old_part].p1;
                int i2 = roadparts[old_part].p2;
                
                Vec3 A = {(float)roadpoints[i1].x, (float)roadpoints[i1].h, (float)roadpoints[i1].y};
                Vec3 B = {(float)roadpoints[i2].x, (float)roadpoints[i2].h, (float)roadpoints[i2].y};

                float t;
                Vec3 cp = closest_point_on_segment(A, B, p_pos, t);
                float dx = cp.x - p_pos.x;
                float dz = cp.z - p_pos.z;
                float dsq = dx * dx + dz * dz;

                if (dsq < min_dsq) {
                    min_dsq = dsq;
                    best_old_part = old_part;
                    best_cp = cp;
                    best_t = t;
                }
            }

            if (best_old_part != -1) {
                int target_p1 = roadparts[best_old_part].p1;
                int target_p2 = roadparts[best_old_part].p2;

                Vec3 A = {(float)roadpoints[target_p1].x, (float)roadpoints[target_p1].h, (float)roadpoints[target_p1].y};
                Vec3 B = {(float)roadpoints[target_p2].x, (float)roadpoints[target_p2].h, (float)roadpoints[target_p2].y};
                float seg_len = get_dist(A, B);
                float t_eps = (seg_len > 0.001f) ? (ENDPOINT_EPS / seg_len) : 0.5f;

                int junction_node_idx = -1;

                if (best_t <= t_eps) {
                    junction_node_idx = target_p1;
                } else if (best_t >= 1.0f - t_eps) {
                    junction_node_idx = target_p2;
                } else {
                    roadpoints.push_back({});
                    junction_node_idx = roadpoints_len;
                    roadpoints[junction_node_idx].x = best_cp.x;
                    roadpoints[junction_node_idx].y = best_cp.z;
                    roadpoints[junction_node_idx].h = best_cp.y;
                    #ifdef ROAD_LOGGING
                    printf(BLUE "        added junction point %d at %d,%d,%d\n" RESET, junction_node_idx, (int)best_cp.x, (int)best_cp.y, (int)best_cp.z);
                    #endif
                    roadpoints_len++;

                    float old_width = roadparts[best_old_part].width;
                    int old_p2 = roadparts[best_old_part].p2;

                    roadparts[best_old_part].p2 = junction_node_idx;

                    roadparts.push_back({});
                    roadparts[roadparts_len].p1 = junction_node_idx;
                    roadparts[roadparts_len].p2 = old_p2;
                    roadparts[roadparts_len].width = old_width;
                    roadparts_len++;
                }

                roadpoints[p_idx].x = roadpoints[junction_node_idx].x;
                roadpoints[p_idx].y = roadpoints[junction_node_idx].y;
                roadpoints[p_idx].h = roadpoints[junction_node_idx].h;

                snapped_junction_idx[p_idx] = junction_node_idx;
                is_snapped[p_idx] = true;
            }
        }

        // 2. KROK: OHRANIČENÍ ZNAČKAMI
        for (int p_idx = new_points_start; p_idx < current_new_points_len; p_idx++) {
            if (is_snapped[p_idx]) {
                bool prev_snapped = (p_idx > new_points_start) ? is_snapped[p_idx - 1] : false;
                bool next_snapped = (p_idx + 1 < current_new_points_len) ? is_snapped[p_idx + 1] : false;

                if (!prev_snapped || !next_snapped) {
                    int j_idx = snapped_junction_idx[p_idx];
                    int ref_idx = !prev_snapped ? (p_idx - 1) : (p_idx + 1);

                    if (ref_idx >= new_points_start && ref_idx < current_new_points_len && ref_idx != j_idx) {
                        float sign_angle = get_angle((float)roadpoints[j_idx].x,
                                                     (float)roadpoints[j_idx].y,
                                                     (float)roadpoints[ref_idx].x,
                                                     (float)roadpoints[ref_idx].y) * RAD_DEG;

                        add_stopsign((float)roadpoints[j_idx].y,
                                     (float)roadpoints[j_idx].h,
                                     (float)roadpoints[j_idx].x,
                                     sign_angle);
                    }
                }
            }
        }
    }

    if (uspech){
        printf(GREEN "        returning with uspech true\n");
    } else {
        printf(RED "        returning with uspech false\n");
    }
    return uspech;
}
#include <vector>
#include <queue>
#include <cmath>
bool can_reach_roadpoint(int start_idx, int target_idx) {
    // 1. Ošetření hraničních stavů
    if (start_idx < 0 || start_idx >= roadpoints_len) return false;
    if (target_idx < 0 || target_idx >= roadpoints_len) return false;
    if (start_idx == target_idx) return true; // Cíl je stejný jako start

    // 2. Sestavení grafu sousednosti (Adjacency List)
    // Pro každý roadpoint si uložíme indexy všech bodů, se kterými je přímo spojen silnicí
    std::vector<std::vector<int>> adj(roadpoints_len);
    for (int i = 0; i < roadparts_len; i++) {
        int p1 = roadparts[i].p1;
        int p2 = roadparts[i].p2;

        // Kontrola platnosti indexů v segmentu
        if (p1 >= 0 && p1 < roadpoints_len && p2 >= 0 && p2 < roadpoints_len) {
            adj[p1].push_back(p2);
            adj[p2].push_back(p1); // Silnice jsou obousměrné
        }
    }

    // 3. Prohledávání do šířky (BFS)
    std::vector<bool> visited(roadpoints_len, false);
    std::queue<int> q;

    q.push(start_idx);
    visited[start_idx] = true;

    while (!q.empty()) {
        int current = q.front();
        q.pop();

        // Pokud jsme narazili na cílový bod, cesty jsou propojené
        if (current == target_idx) {
            return true;
        }

        // Projdeme všechny sousední body spojené silnicí
        for (int neighbor : adj[current]) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                q.push(neighbor);
            }
        }
    }

    // Prošli jsme celou propojenou síť a na cíl jsme nenarazili
    return false;
}
int get_roadpoint_at(Vec3 pos, float tolerance = 1.0f) {
    float tol_sq = tolerance * tolerance;

    for (int i = 0; i < roadpoints_len; i++) {
        float dx = roadpoints[i].x - pos.x;
        float dz = roadpoints[i].y - pos.z;

        if (dx * dx + dz * dz <= tol_sq) {
            return i; // Vrátí index v poli roadpoints
        }
    }

    return -1; // Nenalezeno
}
bool ggen_road(Vec3 road_start, Vec3 road_end, float width, float roadpart_size, float w1, float w2, float w3, float w4, float min_relative_angle_score, float max_height_diff, float min_ang_score, int angle_attempts, int beam_width, int lookahead) {
    printf(WHITE BG_BLUE "ggen_road: start=(%.2f, %.2f, %.2f), end=(%.2f, %.2f, %.2f), width=%.2f, roadpart_size=%.2f" RESET "\n",
           road_start.x, road_start.y, road_start.z,
           road_end.x, road_end.y, road_end.z,
           width, roadpart_size);
    bool uspech=true;
    int orig_roadpoints_len = roadpoints_len;

    if (!gggen_road(road_start, road_end, width, roadpart_size, w1, w2, w3, w4, min_relative_angle_score, max_height_diff, min_ang_score, angle_attempts, beam_width, lookahead, true)){
        uspech=false;
        printf(RED"    ggen road failed\n");
    } else {
        int p2=get_roadpoint_at(road_end, 1.0f);
        if (p2==-1){
            printf(RED"    road_end not found in roadpoints\n");
            uspech=false;
        } else if (!can_reach_roadpoint(orig_roadpoints_len,p2)){
            printf(BLUE"    couldnt reach, trying height thing...\n");
            if (!gggen_road(road_start, road_end, width, roadpart_size, w1, w2, w3, w4, min_relative_angle_score, max_height_diff, min_ang_score, angle_attempts, beam_width, lookahead, true)){
                uspech=false;
                printf(RED"    ggen road failed\n");
            } else {
                int p22=get_roadpoint_at(road_end, 1.0f);
                if (p22==-1){
                    printf(RED"    road_end not found in roadpoints again\n");
                    uspech=false;
                } else if (!can_reach_roadpoint(orig_roadpoints_len,p22)){
                    printf(RED"    couldnt reach again\n");
                    uspech=false;
                }
            }
        }
    }
    printf(YELLOW"    ggen_road returning uspech=%d\n" RESET, uspech);
    return uspech;
}
bool make_objizdka_ending(int start_point_idx, Vec3 road_end, float width = 15.0f) {
    if (start_point_idx < 0 || start_point_idx >= roadpoints_len) return false;

    // Záloha stavu pole/vektorů pro případný rollback při neúspěchu
    int initial_points_len = roadpoints_len;
    int initial_parts_len = roadparts_len;
    size_t initial_points_size = roadpoints.size();
    size_t initial_parts_size = roadparts.size();

    // Lambda funkce pro navrácení stavu při selhání generování
    auto rollback = [&]() {
        roadpoints.resize(initial_points_size);
        roadparts.resize(initial_parts_size);
        roadpoints_len = initial_points_len;
        roadparts_len = initial_parts_len;
    };

    // Získání počáteční pozice
    Vec3 start_pos = {
        (float)roadpoints[start_point_idx].x,
        (float)roadpoints[start_point_idx].h,
        (float)roadpoints[start_point_idx].y
    };

    float dx = road_end.x - start_pos.x;
    float dz = road_end.z - start_pos.z;
    float straight_dist = sqrtf(dx * dx + dz * dz);

    if (straight_dist < 0.001f) return false;

    // Směrové vektory – dopředný a kolmý (vlevo/vpravo)
    float fx = dx / straight_dist;
    float fz = dz / straight_dist;
    float px = -fz;
    float pz =  fx;

    // Parametry pro ggen_road
    float roadpart_size = 100.0f;
    float weights[4] = {0.05f, 0.05f, 0.9f, 0.5}; // relative height, relative angle, absolute angle, absolute height
    float min_relative_angle_score = 0.0f;
    float max_height_diff = 15.0f;          // Zpřísněný výškový limit pro objížďku údolí
    float min_ang_score = 0.0f;

    int angle_attempts = 32;
    int beam_width = 12;                   // Zvýšená šířka hledání pro náročnější terén
    int lookahead = 10;                    // Zpřísněný lookahead pro přísnější kopírování profilu

    // Kandidáti pro vybočení do stran (vnásobky přímé vzdálenosti: + vlevo, - vpravo)
    float offset_factors[] = { 0.35f, -0.35f, 0.50f, -0.50f, 0.20f, -0.20f };
    int num_candidates = sizeof(offset_factors) / sizeof(offset_factors[0]);

    for (int i = 0; i < num_candidates; i++) {
        float side_offset = straight_dist * offset_factors[i];

        // Výpočet midpointu vybočeného do boku
        Vec3 midpoint;
        midpoint.x = (start_pos.x + road_end.x) * 0.5f + px * side_offset;
        midpoint.z = (start_pos.z + road_end.z) * 0.5f + pz * side_offset;
        midpoint.y = (start_pos.y + road_end.y) * 0.5f; // Výchozí střední výška

        // 1. Úsek: Start -> Midpoint
        bool seg1_ok = ggen_road(
            start_pos, midpoint, width, roadpart_size,
            weights[0], weights[1], weights[2], weights[3],
            min_relative_angle_score, max_height_diff, min_ang_score,
            angle_attempts, beam_width, lookahead
        );
        if (seg1_ok){
            int p2=get_roadpoint_at(midpoint, 1.0f);
            if (p2==-1){
                printf("road_end not found in roadpoints\n");
                seg1_ok=false;
            } else if (!can_reach_roadpoint(initial_parts_len,p2)){
                seg1_ok=false;
            }
        }
        bool override=false;
        if (seg1_ok || override) {
            // 2. Úsek: Midpoint -> Cíl
            bool seg2_ok = ggen_road(
                midpoint, road_end, width, roadpart_size,
                weights[0], weights[1], weights[2], weights[3],
                min_relative_angle_score, max_height_diff, min_ang_score,
                angle_attempts, beam_width, lookahead
            );
            if (seg1_ok){
                int p2=get_roadpoint_at(midpoint, 1.0f);
                int p3=get_roadpoint_at(road_end, 1.0f);
                if (p2==-1 || p3==-1){
                    printf("road_end not found in roadpoints\n");
                    seg2_ok=false;
                } else if (!can_reach_roadpoint(p3,p2)){
                    seg2_ok=false;
                }
            }
            if (seg2_ok || override) {
                // Obě části cesty úspěšně vygenerovány
                return true;
            }
        }

        // Pokud některý úsek selhal, vrátíme stav datových struktur zpět a zkusíme další midpoint
        rollback();
    }

    // Pokud selhaly všechny kandidátní trasy, ujistíme se, že nic nezůstalo vygenerováno, a vrátíme neúspěch
    rollback();
    return false;
}
void make_serpentine(int start_point_idx, Vec3 road_end, float width = 15.0f) {
    printf(RESET YELLOW"        make_serpentine: start_point_idx=%d, road_end=(%.2f, %.2f, %.2f), width=%.2f\n",
           start_point_idx, road_end.x, road_end.y, road_end.z, width);
    if (start_point_idx < 0 || start_point_idx >= roadpoints_len) return;
    printf(GREEN"        will continue make serpentine\n" RESET);
    // Nespokojenost s narrowing přetypováním - ošetřeno explicitním (float)
    Vec3 start_pos = {
        (float)roadpoints[start_point_idx].x,
        (float)roadpoints[start_point_idx].h,
        (float)roadpoints[start_point_idx].y
    };

    float dx = road_end.x - start_pos.x;
    float dz = road_end.z - start_pos.z;
    float dh = road_end.y - start_pos.y;

    float straight_dist = sqrtf(dx * dx + dz * dz);
    if (straight_dist < 0.001f) return;

    // Dopředný a kolmý vektor k přímému směru (pro střídání stran vlevo/vpravo)
    float fx = dx / straight_dist;
    float fz = dz / straight_dist;
    float px = -fz; // Kolmice na trasu
    float pz =  fx;

    // NASTAVENÍ SERPENTIN:
    const float MAX_INCLINE_PER_TURN = 10.0f; // Maximální převýšení na jednu zatáčku (v metrech)
    const float SERPENTINE_WIDTH = 70.0f;     // Šířka vybočení serpentiny do stran (v metrech)
    const int STEPS_PER_TURN = 6;             // Počet pod-segmentů pro vyhlazení jednoho stoupání

    // Výpočet počtu otoček podle výšky (min. 2 otočky)
    int num_turns = (int)ceilf(fabsf(dh) / MAX_INCLINE_PER_TURN);
    if (num_turns < 2) num_turns = 2;

    int total_steps = num_turns * STEPS_PER_TURN;
    int prev_point_idx = start_point_idx;

    for (int i = 1; i <= total_steps; i++) {
        float t = (float)i / (float)total_steps;

        // Lineární posun směrem k cíli a plynulé stoupání
        float base_x = start_pos.x + dx * t;
        float base_z = start_pos.z + dz * t;
        float curr_y = start_pos.y + dh * t;

        // Sinusové vybočování do stran vytváří hladké serpentinové vlny
        float side_offset = sinf(t * (float)num_turns * PI) * SERPENTINE_WIDTH;

        Vec3 new_pos;
        new_pos.x = base_x + px * side_offset;
        new_pos.z = base_z + pz * side_offset;
        new_pos.y = curr_y;

        // Poslední krok přesně napojíme na cíl
        if (i == total_steps) {
            new_pos = road_end;
        }

        // Vložení bodu do síťové struktury
        roadpoints.push_back({});
        int new_point_idx = roadpoints_len;
        roadpoints[new_point_idx].x = new_pos.x;
        roadpoints[new_point_idx].y = new_pos.z; // Pozn: .y u roadpoints je Z souřadnice
        roadpoints[new_point_idx].h = new_pos.y; // Pozn: .h je Y výška
        roadpoints_len++;

        // Vytvoření silničního dílu
        roadparts.push_back({});
        roadparts[roadparts_len].p1 = prev_point_idx;
        roadparts[roadparts_len].p2 = new_point_idx;
        roadparts[roadparts_len].width = width;
        roadparts_len++;

        prev_point_idx = new_point_idx;
    }
    printf("        made serpentines with %d turns and %d roadparts\n", num_turns, total_steps);
}
void make_ggen_ending(int start_idx, Vec3 end){
    // if (!make_objizdka_ending(start_idx,end)){
    //     make_serpentine(start_idx,end);
    // }
    make_serpentine(start_idx,end);

}
void gen_road(Vec3 road_start, Vec3 road_end, float width, float roadpart_size=100.0f) {
    float weights[] = {0.1f, 0.1f, 1.2f, 0.3f}; //relative height, relative angle, absolute angle, absolute height
    float min_relative_angle_score = 0.0f;
    float max_height_diff = 15.0f;
    float min_ang_score=0.0f;

    int angle_attempts = 32;    // directions to try per step
    int beam_width = 10;         // how many paths to keep alive
    int lookahead = 8;          // how many steps to look ahead
    if (get_dist(road_start, road_end) < roadpart_size * 2.0f) return;

    // --- ZÁLOHA STAVU PRO PŘÍPADNÝ ROLLBACK ---
    int orig_roadpoints_len = roadpoints_len;
    int orig_roadparts_len = roadparts_len;
    int orig_major_road_points_size = (int)major_road_points.size();
    int start_part_idx = (int)roadparts.size();

    if (get_dist(road_start, road_end) < roadpart_size * 2.0f) return;





    bool uspech = ggen_road(road_start, road_end, width, roadpart_size, weights[0], weights[1], weights[2], weights[3], min_relative_angle_score, max_height_diff, min_ang_score, angle_attempts, beam_width, lookahead);
    // uspech=true;
    if (!uspech){
        // =========================================================================
        // FALLBACK: OŘEZÁNÍ SLEPÉHO KONCE A DOKONČENÍ CESTY
        // =========================================================================
        if (!uspech && roadpoints_len > orig_roadpoints_len) {
            int best_idx = -1;
            float min_dist = 1e9f;

            // 1. Najdeme bod z nové trasy, který se dostal NEJBLÍŽE k cili (road_end)
            for (int p_idx = orig_roadpoints_len; p_idx < roadpoints_len; p_idx++) {
                Vec3 p_pos = {(float)roadpoints[p_idx].x, (float)roadpoints[p_idx].h, (float)roadpoints[p_idx].y};
                float d = get_dist(p_pos, road_end);
                if (d < min_dist) {
                    min_dist = d;
                    best_idx = p_idx;
                }
            }

            // 2. Pokud jsme našli nejbližší bod a lze z něj dokončit cíl
            if (best_idx != -1 && best_idx < roadpoints_len-1) {
                Vec3 best_pos = {(float)roadpoints[best_idx].x, (float)roadpoints[best_idx].h, (float)roadpoints[best_idx].y};

                if (can_make_ending(best_pos, road_end)) {
                    printf("Fallback: Orezavam slepy konec za bodem %d (%d, %d) a dokoncuji silnici.\n", best_idx, (int)roadpoints[best_idx].x, (int)roadpoints[best_idx].y);

                    // OŘEZÁNÍ: Smažeme všechny díly silnice (roadparts), které vedly ZA bod best_idx
                    int valid_parts_len = start_part_idx;
                    for (int i = start_part_idx; i < roadparts_len; i++) {
                        if (roadparts[i].p1 <= best_idx && roadparts[i].p2 <= best_idx) {
                            valid_parts_len = i + 1;
                        } else {
                            break; // Silnice je lineární řetězec, jakmile narazíme na přesahující díl, končíme
                        }
                    }
                    roadparts_len = valid_parts_len;
                    roadparts.resize(roadparts_len);

                    // OŘEZÁNÍ: Smažeme všechny body (roadpoints) ZA bodem best_idx
                    roadpoints_len = best_idx + 1;
                    roadpoints.resize(roadpoints_len+1);

                    // 3. Zavoláme tvou funkci pro dodělání konce z nejbližšího bodu
                    make_ggen_ending(best_idx, road_end);

                    // 4. Označíme generování jako úspěšné (propojení se zdařilo)
                    uspech = true;
                }
            }
        }
    }
    // Pokud se cesta zdárně dokončila, provedeme vyhlazení (Subdivision)
    if (uspech) {
        Vec2 msp={road_start.x, road_start.z};
        Vec2 mep={road_end.x, road_end.z};
        major_road_points.push_back(msp);
        major_road_points.push_back(mep);
        printf("[gen road] generated road from %d, %d to %d, %d and made %d roasparts\n", (int)road_start.x, (int)road_start.z, (int)road_end.x, (int)road_end.z, roadparts_len-orig_roadparts_len);
        int end_part_idx = (int)roadparts.size();
        limit_road_incline(start_part_idx, end_part_idx, 0.1f); // max sklon 0.5 = 26.565°

        for(int step = 0; step < ROAD_SUBDIVISIONS; step++) {
            subdivide_roads(start_part_idx+1, end_part_idx-1);
            end_part_idx = start_part_idx + (end_part_idx - start_part_idx) * 2;
        }
        major_road_segments.push_back({0,{{road_start.z,road_start.x},{road_end.z,road_end.x}}});

    } else {
        // --- CELKOVÝ FAIL: KLÍČOVÝ ROLLBACK A ZAVOLÁNÍ FALLBACKU ---
        roadpoints_len = orig_roadpoints_len;
        roadparts_len = orig_roadparts_len;
        
        roadpoints.resize(orig_roadpoints_len);
        roadparts.resize(orig_roadparts_len);
        major_road_points.resize(orig_major_road_points_size);

        // Zde voláme fallback metodu. Pokud ti kompilátor stále píše "undeclared identifier",
        // ujisti se, že funkce `gen_road_fallback` je v souboru definovaná (nebo má prototyp) NAD funkcí `gen_road`.
        gen_road_fallback(road_start, road_end, width, roadpart_size/2.0f);
    }
    int index=0;
    for (auto& p:roadparts){
        Vec3 p1={(float)roadpoints[p.p1].x,(float)roadpoints[p.p1].h,(float)roadpoints[p.p1].y};
        Vec3 p2={(float)roadpoints[p.p2].x,(float)roadpoints[p.p2].h,(float)roadpoints[p.p2].y};
        if (get_dist(p1,p2)>roadpart_size*3.0f){
            printf("roadpart too long: %d,%d to %d,%d, dist=%f, index: %d\n",(int)p1.x,(int)p1.z,(int)p2.x,(int)p2.z,get_dist(p1,p2), index);
            p.p1=0;
            p.p2=0;
        }
        index++;
    }
    index=0;
    for (auto&p:roadpoints){
        if (p.h==0.0f){
            printf("point number %d is at y=0.0f\n", index);
        }else if (p.h<5.0f){
            printf("point number %d is at y<5.0f\n", index);
        }
        #ifdef ROAD_LOGGING
        printf(YELLOW "point number %d is at x=%d, y=%f, z=%d\n" RESET, index, p.x, p.h, p.y);
        #endif
        index++;
    }
}