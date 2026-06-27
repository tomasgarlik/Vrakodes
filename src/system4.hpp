void calculate_car_deformation_4(cardata& car) {
    for (int i = 0; i < car.vertices_count; i++) {
        car_vertex* vv = &car.vertices[i];
        Vec3 pos = {vv->x, vv->y, vv->z};
        
        int best_idx = -1; 
        int second_idx = -1;
        float dist1 = 1e30f, dist2 = 1e30f;

        for (int p = 0; p < car.points_count; p++) {
            // Only consider points that exist AND match the vertex group
            if (!car.points[p].exists || car.points[p].group != vv->group) continue;

            Vec3 pPos = {car.points[p].x, car.points[p].y, car.points[p].z};
            float d = lengthSq(sub(pos, pPos));

            if (d < dist1) {
                dist2 = dist1;
                second_idx = best_idx;
                dist1 = d;
                best_idx = p;
            } else if (d < dist2) {
                dist2 = d;
                second_idx = p;
            }
        }

        // Safety: If no matching points were found, fallback to the first point
        if (best_idx == -1) best_idx = 0;
        if (second_idx == -1) second_idx = best_idx; // Use best twice if no second match

        vv->fyz_ref_points[0] = best_idx;
        vv->fyz_ref_points[1] = second_idx;

        float d1_sqrt = sqrtf(dist1);
        float d2_sqrt = sqrtf(dist2);
        float total_dist = d1_sqrt + d2_sqrt;
        
        // Handle edge case where vertex is exactly on a point or only one point found
        float w1 = (total_dist > 0.0001f) ? (1.0f - (d1_sqrt / total_dist)) : 1.0f;
        vv->blend_factor = w1; 

        Vec3 p1 = {car.points[best_idx].x, car.points[best_idx].y, car.points[best_idx].z};
        Vec3 p2 = {car.points[second_idx].x, car.points[second_idx].y, car.points[second_idx].z};

        vv->weights[0] = pos.x - p1.x;
        vv->weights[1] = pos.y - p1.y;
        vv->weights[2] = pos.z - p1.z;
        
        vv->weights[3] = pos.x - p2.x;
        vv->weights[4] = pos.y - p2.y;
        vv->weights[5] = pos.z - p2.z;
    }
}
void update_car_mesh_4(cardata& car) {
    for (int i = 0; i < car.vertices_count; i++) {
        car_vertex& vv = car.vertices[i];
        point& p1 = car.points[vv.fyz_ref_points[0]];
        point& p2 = car.points[vv.fyz_ref_points[1]];

        float w1 = vv.blend_factor;
        float w2 = 1.0f - w1;

        // Blend the two influenced positions
        float target_x1 = p1.x + vv.weights[0];
        float target_y1 = p1.y + vv.weights[1];
        float target_z1 = p1.z + vv.weights[2];

        float target_x2 = p2.x + vv.weights[3];
        float target_y2 = p2.y + vv.weights[4];
        float target_z2 = p2.z + vv.weights[5];

        vv.x = (target_x1 * w1) + (target_x2 * w2);
        vv.y = (target_y1 * w1) + (target_y2 * w2);
        vv.z = (target_z1 * w1) + (target_z2 * w2);
    }
}