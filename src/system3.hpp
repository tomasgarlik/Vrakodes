int compute_barycentric(Vec3 p, Vec3 a, Vec3 b, Vec3 c, Vec3 d, float w[4]) {
    Vec3 ab = sub(b, a);
    Vec3 ac = sub(c, a);
    Vec3 ad = sub(d, a);
    Vec3 ap = sub(p, a);

    float det = dot(ab, cross(ac, ad));
    if (fabsf(det) < 1e-12f) return 0;

    float invDet = 1.0f / det;
    w[3] = dot(ab, cross(ac, ap)) * invDet;
    w[2] = dot(ab, cross(ap, ad)) * invDet;
    w[1] = dot(ap, cross(ac, ad)) * invDet;
    w[0] = 1.0f - w[1] - w[2] - w[3];
    return 1;
}
void calculate_car_deformation_3(cardata& car) {
    printf("calculating car deformation 3\n");
    const float eps = 1e-4f;

    for (int i = 0; i < car.vertices_count; i++) {
        car_vertex* vv = &car.vertices[i];
        Vec3 pos = {vv->x, vv->y, vv->z};
        int vertex_group = vv->group; // used for triangle fallback

        int best_tet = -1;
        float best_weights[4];
        int found_inside = 0; // flag: true if we found a containing tetra

        for (int t = 0; t < car.tetrahedra_count; t++) {
            tetrahedron* tet = &car.tetrahedra[t];
            if (tet->group != vertex_group) {
                continue; 
            }
            point* p0 = &car.points[ tet->idx[0] ];
            point* p1 = &car.points[ tet->idx[1] ];
            point* p2 = &car.points[ tet->idx[2] ];
            point* p3 = &car.points[ tet->idx[3] ];

            // Optional: skip tetra if points are not of the same group? 
            // Usually tetra vertices can be of mixed groups – we'll assume it's fine.

            Vec3 A = {p0->x, p0->y, p0->z};
            Vec3 B = {p1->x, p1->y, p1->z};
            Vec3 C = {p2->x, p2->y, p2->z};
            Vec3 D = {p3->x, p3->y, p3->z};

            float w[4];
            if (!compute_barycentric(pos, A, B, C, D, w))
                continue;

            // Check if point is inside (all weights >= -eps)
            if (w[0] >= -eps && w[1] >= -eps && w[2] >= -eps && w[3] >= -eps) {
                best_tet = t;
                for (int k = 0; k < 4; k++) best_weights[k] = w[k];
                found_inside = 1;
                break;
            }
        }

        if (found_inside) {
            // Use tetra method – store tetra data
            tetrahedron* tet = &car.tetrahedra[best_tet];
            for (int k = 0; k < 4; k++) {
                vv->fyz_ref_points[k] = tet->idx[k];
                vv->weights[k] = best_weights[k];
            }
            // Mark as tetra mode (sentinel: all ref points positive)
            // No need to set a flag; we'll detect by fyz_ref_points[3] not being -1.
        } else {
            // Vertex is outside all tetra – fallback to triangle method
            // Find 3 nearest physics points of the same group
            int best_idx[3] = {-1, -1, -1};
            float best_dist[3] = {1e30f, 1e30f, 1e30f};

            for (int p = 0; p < car.points_count; p++) {
                if (car.points[p].exists && car.points[p].group == vertex_group) {
                    Vec3 pPos = {car.points[p].x, car.points[p].y, car.points[p].z};
                    float d = lengthSq(sub(pos, pPos));

                    // Insert into sorted list of three nearest
                    if (d < best_dist[0]) {
                        best_dist[2] = best_dist[1]; best_idx[2] = best_idx[1];
                        best_dist[1] = best_dist[0]; best_idx[1] = best_idx[0];
                        best_dist[0] = d; best_idx[0] = p;
                    } else if (d < best_dist[1]) {
                        best_dist[2] = best_dist[1]; best_idx[2] = best_idx[1];
                        best_dist[1] = d; best_idx[1] = p;
                    } else if (d < best_dist[2]) {
                        best_dist[2] = d; best_idx[2] = p;
                    }
                }
            }

            // Safety check – if we didn't find three points, fallback to first three of same group (or any)
            if (best_idx[0] == -1 || best_idx[1] == -1 || best_idx[2] == -1) {
                // fallback: use first three points of same group (or any)
                int count = 0;
                for (int p = 0; p < car.points_count && count < 3; p++) {
                    if (car.points[p].exists && car.points[p].group == vertex_group) {
                        best_idx[count] = p;
                        count++;
                    }
                }
                if (count < 3) {
                    // still not enough – use whatever we have (maybe duplicate?)
                    for (int k = count; k < 3; k++) best_idx[k] = best_idx[0];
                }
            }

            // Compute barycentric weights for the triangle (in 3D)
            Vec3 A = {car.points[best_idx[0]].x, car.points[best_idx[0]].y, car.points[best_idx[0]].z};
            Vec3 B = {car.points[best_idx[1]].x, car.points[best_idx[1]].y, car.points[best_idx[1]].z};
            Vec3 C = {car.points[best_idx[2]].x, car.points[best_idx[2]].y, car.points[best_idx[2]].z};

            Vec3 v0 = sub(B, A);
            Vec3 v1 = sub(C, A);
            Vec3 v2 = sub(pos, A);

            float d00 = dot(v0, v0);
            float d01 = dot(v0, v1);
            float d11 = dot(v1, v1);
            float d20 = dot(v2, v0);
            float d21 = dot(v2, v1);
            float denom = d00 * d11 - d01 * d01;

            float w[3];
            if (fabsf(denom) > 1e-12f) {
                w[1] = (d11 * d20 - d01 * d21) / denom;
                w[2] = (d00 * d21 - d01 * d20) / denom;
                w[0] = 1.0f - w[1] - w[2];
            } else {
                // Degenerate triangle – fallback to equal weights
                w[0] = w[1] = w[2] = 1.0f / 3.0f;
            }

            // Compute height offset (distance from triangle plane)
            Vec3 normal = normalize(cross(v0, v1));
            float height_offset = dot(sub(pos, A), normal);

            // Store triangle data
            vv->fyz_ref_points[0] = best_idx[0];
            vv->fyz_ref_points[1] = best_idx[1];
            vv->fyz_ref_points[2] = best_idx[2];
            vv->fyz_ref_points[3] = -1;  // sentinel: triangle mode
            vv->weights[0] = w[0];
            vv->weights[1] = w[1];
            vv->weights[2] = w[2];
            vv->height_offset = height_offset;
        }
    }
}
void update_car_mesh_3(cardata& car) {
    log("update car mesh 3");
    for (int i = 0; i < car.vertices_count; i++) {
        // printf("i=%d/%d\n", i, car.vertices_count);
        car_vertex& vv = car.vertices[i];
        if (vv.fyz_ref_points[3]==-1){
            point& p0 = car.points[ vv.fyz_ref_points[0] ];
            point& p1 = car.points[ vv.fyz_ref_points[1] ];
            point& p2 = car.points[ vv.fyz_ref_points[2] ];

            vv.x = p0.x * vv.weights[0] + p1.x * vv.weights[1] + p2.x * vv.weights[2];
            vv.y = p0.y * vv.weights[0] + p1.y * vv.weights[1] + p2.y * vv.weights[2];
            vv.z = p0.z * vv.weights[0] + p1.z * vv.weights[1] + p2.z * vv.weights[2];
        } else {
            point& p0 = car.points[ vv.fyz_ref_points[0] ];
            point& p1 = car.points[ vv.fyz_ref_points[1] ];
            point& p2 = car.points[ vv.fyz_ref_points[2] ];
            point& p3 = car.points[ vv.fyz_ref_points[3] ];

            vv.x = p0.x * vv.weights[0] + p1.x * vv.weights[1] + p2.x * vv.weights[2] + p3.x * vv.weights[3];
            vv.y = p0.y * vv.weights[0] + p1.y * vv.weights[1] + p2.y * vv.weights[2] + p3.y * vv.weights[3];
            vv.z = p0.z * vv.weights[0] + p1.z * vv.weights[1] + p2.z * vv.weights[2] + p3.z * vv.weights[3];
    
        }
    }
    // Recompute normals if needed
}