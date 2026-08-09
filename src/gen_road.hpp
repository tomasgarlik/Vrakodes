struct Candidate {
    Vec3 pos;
    Vec3 first_step; // Pamatujeme si jen První krok, víc lookahead nepotřebuje!
    float last_angle;
    float total_score;
};
void gen_road(Vec3 road_start, Vec3 road_end, float width, float roadpart_size = 100.0f) {
    float weights[] = {0.1f, 0.2f, 0.7f};
    float min_relative_angle_score = 0.0f;
    float max_height_diff = 10.0f;
    float min_ang_score = 0.0f;
    bool uspech = false;

    int angle_attempts = 32;    // Směry k vyzkoušení
    int beam_width = 10;        // Kolik cest držíme živých
    int lookahead = 8;          // Hloubka dohledu

    if (get_dist(road_start, road_end) < roadpart_size * 2.0f) return;

    // --- PRECOMPUTE TABULKY ÚHLŮ A SMĚRŮ (Static = počítá se jen jednou) ---
    static bool trig_inited = false;
    static float pre_angles[32];
    static float pre_sin[32];
    static float pre_cos[32];
    if (!trig_inited) {
        for (int j = 0; j < 32; j++) {
            pre_angles[j] = (PI * 2.0f / 32.0f) * (float)j;
            pre_sin[j] = sinf(pre_angles[j]);
            pre_cos[j] = cosf(pre_angles[j]);
        }
        trig_inited = true;
    }

    // --- ZÁLOHA STAVU PRO PŘÍPADNÝ ROLLBACK ---
    int orig_roadpoints_len = roadpoints_len;
    int orig_roadparts_len = roadparts_len;
    int orig_major_road_points_size = (int)major_road_points.size();
    int start_part_idx = (int)roadparts.size();
    // ------------------------------------------

    Vec3 last_pos = road_start;
    float last_angle = get_angle(road_start.x, road_start.z, road_end.x, road_end.z);
    
    roadpoints.push_back({});
    roadpoints[roadpoints_len].x = road_start.x;
    roadpoints[roadpoints_len].y = road_start.z;
    roadpoints[roadpoints_len].h = road_start.y;
    roadpoints_len++;

    // Před-alokace paměti pro beam, aby se nealokovalo dynamicky v cyklu
    std::vector<Candidate> beam;
    std::vector<Candidate> next_beam;
    beam.reserve(angle_attempts);
    next_beam.reserve(beam_width * angle_attempts);

    int iterations = 0;
    while (iterations < 200000) {
        if (get_dist(last_pos, road_end) < roadpart_size) {
            uspech = true;
            break;
        }

        float target_angle = get_angle(last_pos.x, last_pos.z, road_end.x, road_end.z);

        // --- 1. KROK BEAM SEARCHE ---
        beam.clear();
        for (int j = 0; j < angle_attempts; j++) {
            float angle = pre_angles[j];

            float relative_angle_score = cosf(angle - last_angle);
            if (relative_angle_score < min_relative_angle_score) continue;

            float xpos = last_pos.x + pre_sin[j] * roadpart_size;
            float zpos = last_pos.z + pre_cos[j] * roadpart_size;
            if (xpos < 0 || xpos > MAP_SIZE || zpos < 0 || zpos > MAP_SIZE) continue;
            
            float ypos = mapgen_get_heightmap_height(zpos, xpos);
            if (fabs(last_pos.y - ypos) > max_height_diff) continue;

            float absolute_angle_score = cosf(angle - target_angle);
            float height_score = 1.0f - fminf(1.0f, fabs(last_pos.y - ypos) / max_height_diff);
            float score = weights[0] * height_score + weights[1] * relative_angle_score + weights[2] * absolute_angle_score;
            if (absolute_angle_score < min_ang_score) {
                score = absolute_angle_score;
            }

            Candidate c;
            c.pos = {xpos, ypos, zpos};
            c.first_step = c.pos; // Zaznamenáme první krok!
            c.last_angle = angle;
            c.total_score = score;
            beam.push_back(c);
        }

        if (beam.empty()) break;

        // --- DALŠÍ KROKY LOOKAHEADU ---
        for (int step = 1; step < lookahead; step++) {
            next_beam.clear();

            for (const auto& cand : beam) {
                float step_target_angle = get_angle(cand.pos.x, cand.pos.z, road_end.x, road_end.z);

                for (int j = 0; j < angle_attempts; j++) {
                    float angle = pre_angles[j];

                    float relative_angle_score = cosf(angle - cand.last_angle);
                    if (relative_angle_score < min_relative_angle_score) continue;

                    float xpos = cand.pos.x + pre_sin[j] * roadpart_size;
                    float zpos = cand.pos.z + pre_cos[j] * roadpart_size;
                    if (xpos < 0 || xpos > MAP_SIZE || zpos < 0 || zpos > MAP_SIZE) continue;
                    
                    float ypos = mapgen_get_heightmap_height(zpos, xpos);
                    if (fabs(cand.pos.y - ypos) > max_height_diff) continue;

                    float absolute_angle_score = cosf(angle - step_target_angle);
                    float height_score = 1.0f - fminf(1.0f, fabs(cand.pos.y - ypos) / max_height_diff);
                    float score = weights[0] * height_score + weights[1] * relative_angle_score + weights[2] * absolute_angle_score;

                    Candidate nc;
                    nc.pos = {xpos, ypos, zpos};
                    nc.first_step = cand.first_step; // Předáváme dál původní 1. krok
                    nc.last_angle = angle;
                    nc.total_score = cand.total_score + score;
                    next_beam.push_back(nc);
                }
            }

            if (next_beam.empty()) break;

            // Rychlejší částečné řazení (seřadí pouze nejlepších N prvků místo celého pole)
            int keep_count = std::min((int)next_beam.size(), beam_width);
            std::partial_sort(next_beam.begin(), next_beam.begin() + keep_count, next_beam.end(),
                [](const Candidate& a, const Candidate& b) {
                    return a.total_score > b.total_score;
                });
            
            next_beam.resize(keep_count);
            beam = next_beam;
        }

        if (beam.empty()) break;

        // Nejlepší cesta určuje náš další fyzický krok
        Vec3 next = beam[0].first_step;

        // --- KONTROLA SMYČEK (Bez sqrtf, porovnáváme na druhou mocninu) ---
        bool looping = false;
        int looping_index = -1;
        float max_loop_dist_sq = (roadpart_size * 2.0f) * (roadpart_size * 2.0f);

        for (int k = 0; k < roadpoints_len - 2; k++) {
            float dx = roadpoints[k].x - next.x;
            float dz = roadpoints[k].y - next.z;
            if ((dx * dx + dz * dz) < max_loop_dist_sq) {
                looping = true;
                looping_index = k;
                break;
            }
        }

        if (looping) {
            if (looping_index > start_part_idx) {
                break;
            }
            
            int start_point_idx = roadpoints_len - 1;
            int end_point_idx = looping_index;

            roadpoints.push_back({});
            int p_mid1 = roadpoints_len++;
            roadpoints[p_mid1].x = roadpoints[start_point_idx].x + (roadpoints[end_point_idx].x - roadpoints[start_point_idx].x) * (1.0f / 3.0f);
            roadpoints[p_mid1].y = roadpoints[start_point_idx].y + (roadpoints[end_point_idx].y - roadpoints[start_point_idx].y) * (1.0f / 3.0f);
            roadpoints[p_mid1].h = ((roadpoints[start_point_idx].h + (roadpoints[end_point_idx].h - roadpoints[start_point_idx].h) * (2.0f / 3.0f)) * 0.2f) + (mapgen_get_heightmap_height(roadpoints[p_mid1].y, roadpoints[p_mid1].x) * 0.8f);

            roadpoints.push_back({});
            int p_mid2 = roadpoints_len++;
            roadpoints[p_mid2].x = roadpoints[start_point_idx].x + (roadpoints[end_point_idx].x - roadpoints[start_point_idx].x) * (2.0f / 3.0f);
            roadpoints[p_mid2].y = roadpoints[start_point_idx].y + (roadpoints[end_point_idx].y - roadpoints[start_point_idx].y) * (2.0f / 3.0f);
            roadpoints[p_mid2].h = ((roadpoints[start_point_idx].h + (roadpoints[end_point_idx].h - roadpoints[start_point_idx].h) * (2.0f / 3.0f)) * 0.3f) + (roadpoints[looping_index].h * 0.7f);

            roadparts.push_back({});
            roadparts[roadparts_len].p1 = start_point_idx;
            roadparts[roadparts_len].p2 = p_mid1;
            roadparts[roadparts_len].width = 15.0f;
            roadparts_len++;

            roadparts.push_back({});
            roadparts[roadparts_len].p1 = p_mid1;
            roadparts[roadparts_len].p2 = p_mid2;
            roadparts[roadparts_len].width = 15.0f;
            roadparts_len++;

            roadparts.push_back({});
            roadparts[roadparts_len].p1 = p_mid2;
            roadparts[roadparts_len].p2 = end_point_idx;
            roadparts[roadparts_len].width = 15.0f;
            roadparts_len++;

            float sign_angle = get_angle((float)roadpoints[looping_index].x,
                                        (float)roadpoints[looping_index].y,
                                        (float)roadpoints[start_point_idx].x,
                                        (float)roadpoints[start_point_idx].y) * RAD_DEG;
            
            add_stopsign((float)roadpoints[looping_index].y,
                        roadpoints[looping_index].h,
                        (float)roadpoints[looping_index].x,
                        sign_angle);
            uspech = true;

            break;
        }

        roadpoints.push_back({});
        roadparts.push_back({});
        roadpoints[roadpoints_len].x = next.x;
        roadpoints[roadpoints_len].y = next.z;
        roadpoints[roadpoints_len].h = next.y;
        roadparts[roadparts_len].p1 = roadpoints_len - 1;
        roadparts[roadparts_len].p2 = roadpoints_len;
        roadparts[roadparts_len].width = 15.0f;
        roadpoints_len++;
        roadparts_len++;
        iterations++;

        last_angle = get_angle(last_pos.x, last_pos.z, next.x, next.z);
        last_pos = next;
    }

    // Vyhlazení nebo Fallback
    if (uspech) {
        Vec2 msp = {road_start.x, road_start.z};
        Vec2 mep = {road_end.x, road_end.z};
        major_road_points.push_back(msp);
        major_road_points.push_back(mep);

        int end_part_idx = (int)roadparts.size();
        limit_road_incline(start_part_idx, end_part_idx, 0.1f);

        for (int step = 0; step < ROAD_SUBDIVISIONS; step++) {
            subdivide_roads(start_part_idx + 1, end_part_idx - 1);
            end_part_idx = start_part_idx + (end_part_idx - start_part_idx) * 2;
        }
    } else {
        roadpoints_len = orig_roadpoints_len;
        roadparts_len = orig_roadparts_len;
        
        roadpoints.resize(orig_roadpoints_len);
        roadparts.resize(orig_roadparts_len);
        major_road_points.resize(orig_major_road_points_size);

        gen_road_fallback(road_start, road_end, width, roadpart_size);
    }
}