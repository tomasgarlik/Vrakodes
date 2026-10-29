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
static bool make_frame(Vec3 A, Vec3 B, Vec3 C, Vec3& e1, Vec3& e2, Vec3& e3) {
    Vec3 AB = sub(B, A);
    Vec3 cr = cross(AB, sub(C, A));
    float l1 = dot(AB, AB), l3 = dot(cr, cr);
    if (l1 < 1e-12f || l3 < 1e-12f) return false;
    float i1 = 1.0f / sqrtf(l1), i3 = 1.0f / sqrtf(l3);
    e1 = {AB.x * i1, AB.y * i1, AB.z * i1};
    e3 = {cr.x * i3, cr.y * i3, cr.z * i3};
    e2 = cross(e3, e1);
    return true;
}

// n = 3 (trojúhelník) nebo 4 (tet); useLoc = vertex je mimo, ořízni váhy a ulož offset
static void bind_vertex(cardata& car, car_vertex& vv, const int* id, int n, float* w, Vec3 pos, bool useLoc) {
    if (useLoc) {
        for (int k = 0; k < n; k++) w[k] = fmaxf(0.0f, w[k]);
        float s = 0; for (int k = 0; k < n; k++) s += w[k];
        if (s < 1e-8f) { for (int k = 0; k < n; k++) w[k] = 0; w[0] = 1; s = 1; }
        for (int k = 0; k < n; k++) w[k] /= s;
    }

    Vec3 Q[4], P = {0, 0, 0};
    for (int k = 0; k < n; k++) {
        Q[k] = {car.points[id[k]].x, car.points[id[k]].y, car.points[id[k]].z};
        P.x += Q[k].x * w[k]; P.y += Q[k].y * w[k]; P.z += Q[k].z * w[k];
        vv.fyz_ref_points[k] = id[k];
        vv.weights[k] = w[k];
    }
    if (n == 3) vv.fyz_ref_points[3] = -1;

    vv.loc[0] = vv.loc[1] = vv.loc[2] = 0;
    vv.loc_mode = 0;
    if (!useLoc) return;

    Vec3 d = sub(pos, P);
    Vec3 e1, e2, e3;
    if (make_frame(Q[0], Q[1], Q[2], e1, e2, e3)) {
        vv.loc[0] = dot(d, e1); vv.loc[1] = dot(d, e2); vv.loc[2] = dot(d, e3);
        vv.loc_mode = 1;
    } else {
        vv.loc[0] = d.x; vv.loc[1] = d.y; vv.loc[2] = d.z;
        vv.loc_mode = 2;
    }
}
void calculate_car_deformation_3(cardata& car) {
    printf("calculating car deformation 3\n");
    const float eps = 1e-4f;

    for (int i = 0; i < car.vertices_count; i++) {
        car_vertex& vv = car.vertices[i];
        Vec3 pos = {vv.x, vv.y, vv.z};
        int g = vv.group;

        // 1. tet, který vertex obsahuje, jinak nejbližší (nejmenší "zápornost" vah)
        int   bestTet = -1;
        float bestNeg = 1e30f;
        float bestW[4] = {1, 0, 0, 0};
        bool  inside = false;

        for (int t = 0; t < car.tetrahedra_count; t++) {
            tetrahedron& tet = car.tetrahedra[t];
            if (tet.group != g) continue;

            Vec3 A = {car.points[tet.idx[0]].x, car.points[tet.idx[0]].y, car.points[tet.idx[0]].z};
            Vec3 B = {car.points[tet.idx[1]].x, car.points[tet.idx[1]].y, car.points[tet.idx[1]].z};
            Vec3 C = {car.points[tet.idx[2]].x, car.points[tet.idx[2]].y, car.points[tet.idx[2]].z};
            Vec3 D = {car.points[tet.idx[3]].x, car.points[tet.idx[3]].y, car.points[tet.idx[3]].z};

            float w[4];
            if (!compute_barycentric(pos, A, B, C, D, w)) continue;

            float neg = 0;
            for (int k = 0; k < 4; k++) neg += fmaxf(0.0f, -w[k]);

            if (neg < bestNeg) {
                bestNeg = neg; bestTet = t;
                for (int k = 0; k < 4; k++) bestW[k] = w[k];
            }
            if (neg <= eps) { inside = true; break; }
        }

        if (bestTet >= 0) {
            // uvnitř: beze změny (bez offsetu); mimo: nejbližší tet + ořez + offset
            bind_vertex(car, vv, car.tetrahedra[bestTet].idx, 4, bestW, pos, !inside);
            continue;
        }

        // 2. skupina nemá žádný použitelný tet -> trojúhelník ze 3 nejbližších bodů
        int   bi[3] = {-1, -1, -1};
        float bd[3] = {1e30f, 1e30f, 1e30f};
        for (int p = 0; p < car.points_count; p++) {
            if (!car.points[p].exists || car.points[p].group != g) continue;
            Vec3 pp = {car.points[p].x, car.points[p].y, car.points[p].z};
            float d = lengthSq(sub(pos, pp));
            if (d < bd[0])      { bd[2]=bd[1]; bi[2]=bi[1]; bd[1]=bd[0]; bi[1]=bi[0]; bd[0]=d; bi[0]=p; }
            else if (d < bd[1]) { bd[2]=bd[1]; bi[2]=bi[1]; bd[1]=d; bi[1]=p; }
            else if (d < bd[2]) { bd[2]=d; bi[2]=p; }
        }

        if (bi[0] == -1) {   // ve skupině není žádný bod -> vertex se nehýbe
            vv.fyz_ref_points[0] = vv.fyz_ref_points[1] = vv.fyz_ref_points[2] = -1;
            vv.fyz_ref_points[3] = -1;
            vv.loc_mode = 0;
            continue;
        }
        if (bi[1] == -1) bi[1] = bi[0];
        if (bi[2] == -1) bi[2] = bi[0];

        Vec3 A = {car.points[bi[0]].x, car.points[bi[0]].y, car.points[bi[0]].z};
        Vec3 B = {car.points[bi[1]].x, car.points[bi[1]].y, car.points[bi[1]].z};
        Vec3 C = {car.points[bi[2]].x, car.points[bi[2]].y, car.points[bi[2]].z};
        Vec3 v0 = sub(B, A), v1 = sub(C, A), v2 = sub(pos, A);
        float d00 = dot(v0, v0), d01 = dot(v0, v1), d11 = dot(v1, v1);
        float d20 = dot(v2, v0), d21 = dot(v2, v1);
        float denom = d00 * d11 - d01 * d01;

        float w[3] = {1, 0, 0};
        if (fabsf(denom) > 1e-12f) {
            w[1] = (d11 * d20 - d01 * d21) / denom;
            w[2] = (d00 * d21 - d01 * d20) / denom;
            w[0] = 1.0f - w[1] - w[2];
        }
        bind_vertex(car, vv, bi, 3, w, pos, true);
    }
}
void update_car_mesh_3(cardata& car) {
    for (int i = 0; i < car.vertices_count; i++) {
        car_vertex& vv = car.vertices[i];
        int n = (vv.fyz_ref_points[3] == -1) ? 3 : 4;

        bool bad = false;
        for (int k = 0; k < n; k++)
            if (vv.fyz_ref_points[k] < 0 || vv.fyz_ref_points[k] >= car.points_count) bad = true;
        if (bad) continue;

        Vec3 Q[4], P = {0, 0, 0};
        for (int k = 0; k < n; k++) {
            point& p = car.points[vv.fyz_ref_points[k]];
            Q[k] = {p.x, p.y, p.z};
            P.x += Q[k].x * vv.weights[k];
            P.y += Q[k].y * vv.weights[k];
            P.z += Q[k].z * vv.weights[k];
        }

        if (vv.loc_mode == 1) {
            Vec3 e1, e2, e3;
            if (make_frame(Q[0], Q[1], Q[2], e1, e2, e3)) {
                P.x += e1.x*vv.loc[0] + e2.x*vv.loc[1] + e3.x*vv.loc[2];
                P.y += e1.y*vv.loc[0] + e2.y*vv.loc[1] + e3.y*vv.loc[2];
                P.z += e1.z*vv.loc[0] + e2.z*vv.loc[1] + e3.z*vv.loc[2];
            }
        } else if (vv.loc_mode == 2) {
            P.x += vv.loc[0]; P.y += vv.loc[1]; P.z += vv.loc[2];
        }

        vv.x = P.x; vv.y = P.y; vv.z = P.z;
    }
}