bool is_point_in_volume(point& p, cardata& car, int group) {
    Vec3 dir = {0, 1, 0};
    int hits = 0;
    for (int j = 0; j < car.col_faces_count; j++) {
        face& f = car.col_faces[j];
        if (f.group != group) continue;
        point& v0 = car.points[f.vertices[0]];
        point& v1 = car.points[f.vertices[1]];
        point& v2 = car.points[f.vertices[2]];
        if (ray_triangle_intersection(p, v0, v1, v2, dir).hit)
            hits++;
    }
    return (hits % 2) == 1;
}

void calculate_softsoft_collisions(point& p, int this_car_ind, float dt) {
    if (!p.collide) return;

    for (int indian = 0; indian < (int)cars.size(); indian++) {
        if (indian == this_car_ind) continue;
        cardata& car = cars[indian];
        float cdx = p.x - car.pos_x;
        float cdy = p.y - car.pos_y;
        float cdz = p.z - car.pos_z;
        float dist = sqrtf(cdx*cdx + cdy*cdy + cdz*cdz);
        if (dist > car.bound * 1.1f) continue; // small margin
        for (int g = 0; g < car.volumes_count; g++) {
            if (!is_point_in_volume(p, car, g)) continue;

            // compute centroid of this group
            Vec3 centroid = {0,0,0};
            int pt_count = 0;
            for (int j = 0; j < car.col_faces_count; j++) {
                face& f = car.col_faces[j];
                if (f.group != g) continue;
                for (int v = 0; v < 3; v++) {
                    centroid.x += car.points[f.vertices[v]].x;
                    centroid.y += car.points[f.vertices[v]].y;
                    centroid.z += car.points[f.vertices[v]].z;
                    pt_count++;
                }
            }
            if (pt_count == 0) continue;
            centroid.x /= pt_count;
            centroid.y /= pt_count;
            centroid.z /= pt_count;

            // find nearest face to push out through
            float min_dist = 1e30f;
            int best_face = -1;

            for (int j = 0; j < car.col_faces_count; j++) {
                face& f = car.col_faces[j];
                if (f.group != g) continue;

                point& v0 = car.points[f.vertices[0]];
                point& v1 = car.points[f.vertices[1]];
                point& v2 = car.points[f.vertices[2]];

                float fcx = (v0.x+v1.x+v2.x)/3.0f;
                float fcy = (v0.y+v1.y+v2.y)/3.0f;
                float fcz = (v0.z+v1.z+v2.z)/3.0f;

                float dx = p.x - fcx;
                float dy = p.y - fcy;
                float dz = p.z - fcz;
                float dist = sqrtf(dx*dx+dy*dy+dz*dz);

                if (dist < min_dist) {
                    min_dist = dist;
                    best_face = j;
                }
            }

            if (best_face == -1) continue;

            face& bf = car.col_faces[best_face];
            point& v0 = car.points[bf.vertices[0]];
            point& v1 = car.points[bf.vertices[1]];
            point& v2 = car.points[bf.vertices[2]];

            // get face normal, orient away from centroid
            Vec3 n = {bf.nx, bf.ny, bf.nz};
            float nlen = sqrtf(n.x*n.x+n.y*n.y+n.z*n.z);
            if (nlen < 1e-6f) continue;
            n.x/=nlen; n.y/=nlen; n.z/=nlen;

            float fcx = (v0.x+v1.x+v2.x)/3.0f;
            float fcy = (v0.y+v1.y+v2.y)/3.0f;
            float fcz = (v0.z+v1.z+v2.z)/3.0f;

            if (n.x*(fcx-centroid.x)+n.y*(fcy-centroid.y)+n.z*(fcz-centroid.z) < 0) {
                n.x=-n.x; n.y=-n.y; n.z=-n.z;
            }

            // penetration depth along normal
            float plane_d = n.x*v0.x + n.y*v0.y + n.z*v0.z;
            float signed_dist = n.x*p.x + n.y*p.y + n.z*p.z - plane_d;
            float pen = -signed_dist;
            if (pen <= 0.0f) continue;

            // masses
            float face_mass = v0.mass + v1.mass + v2.mass;
            float total_mass = p.mass + face_mass;
            float p_ratio = face_mass / total_mass;
            float f_ratio = p.mass / total_mass;
            float stiffness = 0.5f;

            // position correction
            p.x += n.x * pen * p_ratio * stiffness;
            p.y += n.y * pen * p_ratio * stiffness;
            p.z += n.z * pen * p_ratio * stiffness;

            float f_share = (f_ratio * stiffness) / 3.0f;
            v0.x -= n.x * pen * f_share;
            v0.y -= n.y * pen * f_share;
            v0.z -= n.z * pen * f_share;
            v1.x -= n.x * pen * f_share;
            v1.y -= n.y * pen * f_share;
            v1.z -= n.z * pen * f_share;
            v2.x -= n.x * pen * f_share;
            v2.y -= n.y * pen * f_share;
            v2.z -= n.z * pen * f_share;

            // velocity response
            float fvx = (v0.vx+v1.vx+v2.vx)/3.0f;
            float fvy = (v0.vy+v1.vy+v2.vy)/3.0f;
            float fvz = (v0.vz+v1.vz+v2.vz)/3.0f;

            float rvx = p.vx - fvx;
            float rvy = p.vy - fvy;
            float rvz = p.vz - fvz;
            float relVelN = rvx*n.x + rvy*n.y + rvz*n.z;

if (relVelN < 0.0f) {
    float restitution = 0.3f;
    float impulse = -(1.0f + restitution) * relVelN / total_mass;

    p.vx += impulse * face_mass * n.x;
    p.vy += impulse * face_mass * n.y;
    p.vz += impulse * face_mass * n.z;

    float v_share = impulse * p.mass / 3.0f;
    v0.vx -= v_share * n.x;
    v0.vy -= v_share * n.y;
    v0.vz -= v_share * n.z;
    v1.vx -= v_share * n.x;
    v1.vy -= v_share * n.y;
    v1.vz -= v_share * n.z;
    v2.vx -= v_share * n.x;
    v2.vy -= v_share * n.y;
    v2.vz -= v_share * n.z;

    // --- FRICTION ---
    // relative velocity along the surface (tangent)
    float tvx = rvx - relVelN * n.x;
    float tvy = rvy - relVelN * n.y;
    float tvz = rvz - relVelN * n.z;
    float tangent_len = sqrtf(tvx*tvx + tvy*tvy + tvz*tvz);

    if (tangent_len > 1e-6f) {
        // normalize tangent
        float tx = tvx / tangent_len;
        float ty = tvy / tangent_len;
        float tz = tvz / tangent_len;

        float friction_coeff = 0.4f; // tweak this — 0 = ice, 1 = rubber
        float friction_impulse = -tangent_len / total_mass;

        // clamp friction to not exceed normal impulse (Coulomb friction)
        float max_friction = friction_coeff * fabsf(impulse);
        if (fabsf(friction_impulse) > max_friction)
            friction_impulse = -max_friction;

        p.vx += friction_impulse * face_mass * tx;
        p.vy += friction_impulse * face_mass * ty;
        p.vz += friction_impulse * face_mass * tz;

        float fv_share = friction_impulse * p.mass / 3.0f;
        v0.vx -= fv_share * tx;
        v0.vy -= fv_share * ty;
        v0.vz -= fv_share * tz;
        v1.vx -= fv_share * tx;
        v1.vy -= fv_share * ty;
        v1.vz -= fv_share * tz;
        v2.vx -= fv_share * tx;
        v2.vy -= fv_share * ty;
        v2.vz -= fv_share * tz;
    }
}
        }
    }
}