// void resolve_softsoft_collision(collision_box b, point& p, point& p1, point& p2, point& p3){
//     float bounce=-0.5f;
//     // 1. Vzdálenost bodu od středu boxu (relativní pozice)
//     float tx = p.x - b.cx + x_shift;
//     float ty = p.y - b.cy;
//     float tz = p.z - b.cz + z_shift;

//     // 2. Vzdálenost k jednotlivým stěnám (half-extents minus absolutní pozice)
//     float adx = b.hx - fabsf(tx);
//     float ady = b.hy - fabsf(ty);
//     float adz = b.hz - fabsf(tz);

//     float friction = (p.group == 1) ? 0.95f : 0.5f;
//     friction = clamp(friction + b.friction, 0.0f, 1.0f);
//     float bias = 0.001f;

//     // 3. RESOLVE - Zjistíme, která osa má nejmenší průnik (tam došlo ke kolizi)
//     if (ady < adx + bias && ady < adz + bias) {
//         // Kolize shora nebo zespoda (Y osa)
//         ty = (ty > 0) ? b.hy : -b.hy;
//         p.vy *= bounce;   // Odraz
//         p.vx *= friction; // Tření
//         p.vz *= friction;
//     } 
//     else if (adx < adz) {
//         // Kolize z boku (X osa)
//         tx = (tx > 0) ? b.hx : -b.hx;
//         p.vx *= bounce;
//         p.vy *= friction;
//         p.vz *= friction;
//     } 
//     else {
//         // Kolize zepředu/zezadu (Z osa)
//         tz = (tz > 0) ? b.hz : -b.hz;
//         p.vz *= bounce;
//         p.vx *= friction;
//         p.vy *= friction;
//     }

//     // 4. Update pozice (přičteme střed boxu zpět)
//     p.x = tx + b.cx;
//     p.y = ty + b.cy;
//     p.z = tz + b.cz;






//     Vec3 fm={
//         (p1.x+p2.x+p3.x)/3.0f,
//         (p1.y+p2.y+p3.y)/3.0f,
//         (p1.z+p2.z+p3.z)/3.0f,
//     };
//     Vec3 dir={
//         (fm.x-p.x),
//         (fm.y-p.y),
//         (fm.z-p.z)
//     };
//     // p1.x+=dir.x;
//     // p1.y+=dir.y;
//     // p1.z+=dir.z;
//     // p2.x+=dir.x;
//     // p2.y+=dir.y;
//     // p2.z+=dir.z;
//     // p3.x+=dir.x;
//     // p3.y+=dir.y;
//     // p3.z+=dir.z;
// }
// void calculate_softsoft_collisions(point& p, int this_car_ind){
//     for (int indian=0;indian<cars.size();indian++){
//         if (indian==this_car_ind && !voxel_debug){continue;}
//         for (int indi=0;indi<cars[indian].col_faces_count;indi++){
//             // face& f = cars[indian].col_faces[indi];
//             // point& pp1 = cars[indian].points[f.vertices[0]];
//             // point& pp2 = cars[indian].points[f.vertices[1]];
//             // point& pp3 = cars[indian].points[f.vertices[2]];
//             // point p1;
//             // point p2;
//             // point p3;
//             // // --- Krok 1: Vektory (stejný jako předtím) ---
//             // Vec3 A = {p.oldx, p.oldy, p.oldz};
//             // Vec3 B = {p.x, p.y, p.z};
//             // Vec3 N = {f.nx, f.ny, f.nz};
//             // Vec3 T1 = {p1.x, p1.y, p1.z};
//             // Vec3 T2 = {p2.x, p2.y, p2.z};
//             // Vec3 T3 = {p3.x, p3.y, p3.z};
//             // //gofyahh sort
//             // // if (pp1.y>=pp2.y && pp1.y>=pp3.y){p1=pp1;}
//             // // else if (pp2.y>=pp1.y && pp2.y>=pp3.y){p1=pp2;}
//             // // else if (pp3.y>=pp2.y && pp3.y>=pp1.y){p1=pp3;}

//             // // if (pp1.y<=pp2.y && pp1.y<=pp3.y){p3=pp1;}
//             // // else if (pp2.y<=pp1.y && pp2.y<=pp3.y){p3=pp2;}
//             // // else if (pp3.y<=pp2.y && pp3.y<=pp1.y){p3=pp3;}

//             // // if ((pp1.y>=pp2.y && pp1.y<=pp3.y) || (pp1.y<=pp2.y && pp1.y>=pp3.y)){p2=pp1;}
//             // // else if ((pp2.y>=pp1.y && pp2.y<=pp3.y) || (pp2.y<=pp1.y && pp2.y>=pp3.y)){p2=pp2;}
//             // // else if ((pp3.y>=pp2.y && pp3.y<=pp1.y) || (pp3.y<=pp2.y && pp3.y>=pp1.y)){p2=pp3;}


//             // // google gemini sort
//             // point body[] = {pp1, pp2, pp3};
//             // // Seřadíme body podle y (vzestupně)
//             // std::sort(body, body + 3, [](point a, point b) {
//             //     return a.y < b.y;
//             // });
//             // // Teď už jen přiřadíš výsledek
//             // p1 = body[2]; // Největší y (nahoře)
//             // p2 = body[1]; // Prostřední
//             // p3 = body[0]; // Nejmenší y (dole)
//             // bool we_shall_resolve_this_collision=false;

//             // p1.x/=voxel_size;
//             // p1.y/=voxel_size;
//             // p1.z/=voxel_size;
//             // p2.x/=voxel_size;
//             // p2.y/=voxel_size;
//             // p2.z/=voxel_size;
//             // p3.x/=voxel_size;
//             // p3.y/=voxel_size;
//             // p3.z/=voxel_size;

//             // Vec3 fm={
//             //     (p1.x+p2.x+p3.x)/3.0f,
//             //     (p1.y+p2.y+p3.y)/3.0f,
//             //     (p1.z+p2.z+p3.z)/3.0f,
//             // };
//             // float overlap=0.3f;
//             // p1.x+=(p1.x-fm.x)*overlap;
//             // p1.y+=(p1.y-fm.y)*overlap;
//             // p1.z+=(p1.z-fm.z)*overlap;
//             // p2.x+=(p2.x-fm.x)*overlap;
//             // p2.y+=(p2.y-fm.y)*overlap;
//             // p2.z+=(p2.z-fm.z)*overlap;
//             // p3.x+=(p3.x-fm.x)*overlap;
//             // p3.y+=(p3.y-fm.y)*overlap;
//             // p3.z+=(p3.z-fm.z)*overlap;
//             // // top half of the triangle
//             // float step_multiplyer=1.2f;
//             // int ysteps=(int)(p1.y-p2.y)*step_multiplyer;
//             // int lysteps=(int)(p1.y-p3.y)*step_multiplyer;
//             // if (ysteps<1){
//             //     ysteps=1;
//             // }
//             // if (lysteps<1){
//             //     lysteps=1;
//             // }
//             // point po1=p1;
//             // point po2=p1;
//             // Vec2 po1step={(p3.x-p1.x)/(float)(lysteps), (p3.z-p1.z)/(float)(lysteps)};
//             // Vec2 po2step={(p2.x-p1.x)/(float)(ysteps), (p2.z-p1.z)/(float)(ysteps)};
//             // float ypos=p1.y;
//             // float ystep=(p1.y-p2.y)/(float)(ysteps);
//             // float xf;
//             // float yf;
//             // bool ghetout=false;
//             // for (int step=0;step<=ysteps;step++){
//             //     if (ghetout){
//             //         break;
//             //     }
//             //     ypos-=ystep;
//             //     if ((int)ypos!=(int)(p.y/voxel_size)){continue;}

//             //     po1.x+=po1step.x;
//             //     po1.z+=po1step.y;
//             //     po2.x+=po2step.x;
//             //     po2.z+=po2step.y;
//             //     xf=po1.x;
//             //     yf=po1.z;
//             //     int linsteps=0;
//             //     if (max(po2.x-po1.x)>max(po2.z-po1.z)){
//             //         linsteps=(int)(max(po2.x-po1.x)*1.1);
//             //     } else {
//             //         linsteps=(int)(max(po2.z-po1.z)*1.1);
//             //     }
//             //     for (int linstep=0;linstep<linsteps;linstep++){
//             //         Vec3 vox;
//             //         vox.x=(int)(xf);
//             //         vox.y=(int)(ypos);
//             //         vox.z=(int)(yf);
//             //         if (((int)(p.x/voxel_size)==(int)vox.x) && ((int)(p.y/voxel_size)==(int)vox.y) && ((int)(p.z/voxel_size)==(int)vox.z) && !(indian==this_car_ind)) {
//             //             collision_box b;
//             //             b.cx=xf*voxel_size;
//             //             b.cz=yf*voxel_size;
//             //             b.cy=ypos*voxel_size;
//             //             b.hx=voxel_size/2.0f;
//             //             b.hy=voxel_size/2.0f;
//             //             b.hz=voxel_size/2.0f;
//             //             b.friction=0.5f;
//             //             resolve_softsoft_collision(b,p,pp1,pp2,pp3);
//             //             if (!voxel_debug){
//             //                 ghetout=true;
//             //                 printf("not pushed back");

//             //                 break;
//             //             }
//             //         }
//             //         if (voxel_debug){
//             //             Vec3 mlox;
//             //             mlox.x=vox.x*voxel_size;
//             //             mlox.y=vox.y*voxel_size;
//             //             mlox.z=vox.z*voxel_size;
//             //             add_unique(voxels, mlox);
//             //         }
//             //         xf+=(po2.x-po1.x)/(float)linsteps;
//             //         yf+=(po2.z-po1.z)/(float)linsteps;
//             //     }

//             // }




//             // // now the bottom half
//             // ysteps = (int)(p2.y - p3.y)*step_multiplyer;
//             // if (ysteps<1){
//             //     ysteps=1;
//             // }
//             // // Důležité: Krátká hrana (po2) teď začíná v p2 a jde do p3
//             // po2 = p2; 
//             // // Dlouhá hrana (po1) stále začíná v p1 a jde do p3, 
//             // // ale pro spodní cyklus ji musíme posunout na úroveň p2.y
//             // float t_mid = (p1.y - p2.y) / (float)(p1.y - p3.y);
//             // po1.x = p1.x + t_mid * (p3.x - p1.x);
//             // po1.y = p2.y; // Jsme na úrovni prostředního bodu
//             // po1.z = p1.z + t_mid * (p3.z - p1.z);
//             // // Kroky pro posun po hranách
//             // // po1step: stále stejný sklon dlouhé hrany (p1 -> p3)
//             // po1step = {(p3.x - p1.x) / (float)lysteps, (p3.z - p1.z) / (float)lysteps};
//             // // po2step: nový sklon krátké hrany (p2 -> p3)
//             // po2step = {(p3.x - p2.x) / (float)ysteps, (p3.z - p2.z) / (float)ysteps};

//             // ypos = p2.y; // Začínáme na y souřadnici prostředního bodu
//             // ystep = 1.0f; // Postupujeme po 1 voxelu (v jednotkách voxel_size)

//             // for (int step = 0; step <= ysteps; step++) {
//             //     if (ghetout){break;}

//             //     // Kontrola, jestli je tento řádek ten, kde se nachází částice
//             //     if ((int)ypos == (int)(p.y / voxel_size)) {
//             //         xf = po1.x;
//             //         yf = po1.z;
                    
//             //         // Určíme počet kroků mezi hranami na tomto řádku
//             //         int linsteps = 0;
//             //         float dx = fabsf(po2.x - po1.x);
//             //         float dz = fabsf(po2.z - po1.z);
//             //         linsteps = (int)(std::max(dx, dz) * 1.1f) + 1;
//             //         for (int linstep = 0; linstep < linsteps; linstep++) {
//             //             Vec3 vox;
//             //             vox.x=(int)(xf);
//             //             vox.y=(int)(ypos);
//             //             vox.z=(int)(yf);
//             //             if (((int)(p.x/voxel_size)==(int)vox.x) && ((int)(p.y/voxel_size)==(int)vox.y) && ((int)(p.z/voxel_size)==(int)vox.z) && !(indian==this_car_ind)) {
//             //                 collision_box b;
//             //                 b.cx = xf * voxel_size;
//             //                 b.cz = yf * voxel_size;
//             //                 b.cy = ypos * voxel_size;
//             //                 b.hx = voxel_size / 2.0f;
//             //                 b.hy = voxel_size / 2.0f;
//             //                 b.hz = voxel_size / 2.0f;
//             //                 b.friction = 0.5f;
//             //                 printf("checking\n");
//             //                 resolve_softsoft_collision(b, p, pp1,pp2,pp3);
//             //                 if (!voxel_debug){
//             //                     ghetout = true;
//             //                     printf("not pushed back");
//             //                     break;
//             //                 }
//             //             }
//             //             if (voxel_debug){
//             //                 Vec3 mlox;
//             //                 mlox.x=vox.x*voxel_size;
//             //                 mlox.y=vox.y*voxel_size;
//             //                 mlox.z=vox.z*voxel_size;
//             //                 // add_unique(voxels, mlox);
//             //             }
//             //             // Posun po řádku od po1 k po2
//             //             xf += (po2.x - po1.x) / (float)linsteps;
//             //             yf += (po2.z - po1.z) / (float)linsteps;
//             //         }
//             //     }
                
//             //     // Posun na další řádek (směrem dolů k p3)
//             //     ypos -= ystep;
//             //     po1.x += po1step.x;
//             //     po1.z += po1step.y;
//             //     po2.x += po2step.x;
//             //     po2.z += po2step.y;
//             // }







//             // --- 2. Hlavní výpočetní blok ---
// face& f = cars[indian].col_faces[indi];
// point& pp1 = cars[indian].points[f.vertices[0]];
// point& pp2 = cars[indian].points[f.vertices[1]];
// point& pp3 = cars[indian].points[f.vertices[2]];

// // Převedeme vrcholy do voxelových souřadnic (pro výpočty indexů)
// float v1x = pp1.x / voxel_size; float v1y = pp1.y / voxel_size; float v1z = pp1.z / voxel_size;
// float v2x = pp2.x / voxel_size; float v2y = pp2.y / voxel_size; float v2z = pp2.z / voxel_size;
// float v3x = pp3.x / voxel_size; float v3y = pp3.y / voxel_size; float v3z = pp3.z / voxel_size;

// // Zjistíme Bounding Box trojúhelníku (ve voxelech)
// int minX = (int)floorf(std::min({v1x, v2x, v3x}) - 0.5f);
// int maxX = (int)ceilf(std::max({v1x, v2x, v3x}) + 0.5f);
// int minY = (int)floorf(std::min({v1y, v2y, v3y}) - 0.5f);
// int maxY = (int)ceilf(std::max({v1y, v2y, v3y}) + 0.5f);
// int minZ = (int)floorf(std::min({v1z, v2z, v3z}) - 0.5f);
// int maxZ = (int)ceilf(std::max({v1z, v2z, v3z}) + 0.5f);

// // Aktuální pozice částice ve voxelech
// int partX = (int)floorf(p.x / voxel_size);
// int partY = (int)floorf(p.y / voxel_size);
// int partZ = (int)floorf(p.z / voxel_size);

// // Procházíme voxely v Bounding Boxu
// // Optimalizace: Procházíme jen Y hladinu, kde je částice
// // Procházíme voxely v Bounding Boxu (pouze X a Z)
// for (int x = minX; x <= maxX; x++) {
//     for (int z = minZ; z <= maxZ; z++) {
//         // Otestujeme, zda střed voxelu (x,z) leží v 2D průmětu trojúhelníku
//         if (isPointInTriangle((float)x, (float)z, v1x, v1z, v2x, v2z, v3x, v3z)) {
            
//             // Spočítáme world souřadnice pro tento voxel
//             float Xw = x * voxel_size;
//             float Zw = z * voxel_size;
            
//             // Normála trojúhelníku (z face)
//             float nx = f.nx, ny = f.ny, nz = f.nz;
            
//             // Pokud je ny nulové, trojúhelník je svislý – musíme řešit zvlášť
//             if (fabs(ny) > 1e-6) {
//                 // Y z rovnice roviny: ny*(Y - pp1.y) + nx*(Xw - pp1.x) + nz*(Zw - pp1.z) = 0
//                 float Yw = pp1.y - (nx*(Xw - pp1.x) + nz*(Zw - pp1.z)) / ny;
                
//                 // Voxelový index Y
//                 int yVoxel = (int)floorf(Yw / voxel_size);
                
//                 // Debug: přidáme voxel do seznamu pro vizualizaci
//                 if (voxel_debug) {
//                     Vec3 voxPos = { Xw, (float)yVoxel * voxel_size, Zw };
//                     add_unique(voxels, voxPos);
//                 }
                
//                 // Kolize s částicí: pokud se shodují X, Z a Y voxel
//                 if (x == partX && z == partZ && yVoxel == partY && !(indian == this_car_ind)) {
//                     collision_box b;
//                     b.cx = Xw;
//                     b.cy = Yw;               // Přesná Y na trojúhelníku
//                     b.cz = Zw;
//                     b.hx = b.hy = b.hz = voxel_size / 2.0f;
//                     b.friction = 0.5f;
//                     resolve_softsoft_collision(b, p, pp1, pp2, pp3);
//                 }
//             } else {
//                 // Svislý trojúhelník – Y není jednoznačné, musíme ošetřit zvlášť
//                 // Můžeme například zkontrolovat, zda Y částice leží mezi Y vrcholů
//                 float minYtri = std::min({pp1.y, pp2.y, pp3.y});
//                 float maxYtri = std::max({pp1.y, pp2.y, pp3.y});
//                 if (p.y >= minYtri && p.y <= maxYtri) {
//                     // A dále ověřit, že (Xw, Zw) je uvnitř (už jsme ověřili)
//                     if (x == partX && z == partZ && !(indian == this_car_ind)) {
//                         // Kolize – použijeme průměrnou Y nebo střed voxelu
//                         float Yw = (pp1.y + pp2.y + pp3.y) / 3.0f;
//                         collision_box b;
//                         b.cx = Xw;
//                         b.cy = Yw;
//                         b.cz = Zw;
//                         b.hx = b.hy = b.hz = voxel_size / 2.0f;
//                         b.friction = 0.5f;
//                         resolve_softsoft_collision(b, p, pp1, pp2, pp3);
//                     }
//                 }
//             }
//         }
//     }
// }



//                         // Vec3 dir = B - A;
//             // p.x = A.x;
//             // p.y = A.y;
//             // p.z = A.z;
//             // p1.x+=dir.x;
//             // p1.y+=dir.y;
//             // p1.z+=dir.z;
//             // p2.x+=dir.x;
//             // p2.y+=dir.y;
//             // p2.z+=dir.z;
//             // p3.x+=dir.x;
//             // p3.y+=dir.y;
//             // p3.z+=dir.z;
//             // // 4. Koeficient odrazu (0 = lepí, 1 = dokonalý odraz)
//             // float bounce = 0.5f;
            

//             // float impulse = -(1 + bounce);
            
//             // // 6. Aplikace impulsu na bod
//             // p.vx += impulse * N.x;
//             // p.vy += impulse * N.y;
//             // p.vz += impulse * N.z;
            
//             // // 7. Aplikace impulsu na trojúhelník (rozděleno mezi vrcholy)
//             // float tri_impulse = -impulse;  // opačný směr
            
//             // p1.vx += tri_impulse * N.x;
//             // p1.vy += tri_impulse * N.y;
//             // p1.vz += tri_impulse * N.z;
//             // p2.vx += tri_impulse * N.x;
//             // p2.vy += tri_impulse * N.y;
//             // p2.vz += tri_impulse * N.z;
//             // p3.vx += tri_impulse * N.x;
//             // p3.vy += tri_impulse * N.y;
//             // p3.vz += tri_impulse * N.z;



//         }
//     }
// }

































void calculate_softsoft_collisions(point& p, int this_car_ind, float dt) {
    if (!p.collide){return;}
    Vec3 dirs[] = {
        { 0.57735f,  0.57735f,  0.57735f},
        { 0.57735f, -0.57735f, -0.57735f},
        {-0.57735f,  0.57735f, -0.57735f},
        {-0.57735f, -0.57735f,  0.57735f}
    };
    for (int indian = 0; indian < cars.size(); indian++) {
        if (indian == this_car_ind){continue;}
        cardata& car = cars[indian];
        bool inside=true;
        int facegroup=-1;
        for (int ray=0;ray<4;ray++){
            bool hit=false;
            for (int j=0;j<car.col_faces_count;j++){
                face& f=car.col_faces[j];
                point& p1=car.points[f.vertices[0]];
                point& p2=car.points[f.vertices[1]];
                point& p3=car.points[f.vertices[2]];
                if (raycast(p,p1,p2,p3,dirs[ray]) && (f.group==facegroup || facegroup==-1)){
                    hit=true;
                    break;
                }
            }
            if (!hit){
                inside=false;
                break;
            }
        }
        if (inside){
            // resolve






            // // 2. Najdi trojúhelník (face) auta `car`, jehož střed je nejblíž k bodu p
            // float bestDistSq = 1e10f;  // kvadrát vzdálenosti (rychlejší)
            // int bestFaceIdx = -1;

            // for (int j = 0; j < car.col_faces_count; j++) {
            //     face& f = car.col_faces[j];
            //     point& v0 = car.points[f.vertices[0]];
            //     point& v1 = car.points[f.vertices[1]];
            //     point& v2 = car.points[f.vertices[2]];

            //     // Střed trojúhelníku (průměr vrcholů)
            //     float cx = (v0.x + v1.x + v2.x) / 3.0f;
            //     float cy = (v0.y + v1.y + v2.y) / 3.0f;
            //     float cz = (v0.z + v1.z + v2.z) / 3.0f;

            //     float dx = cx - p.x;
            //     float dy = cy - p.y;
            //     float dz = cz - p.z;
            //     float distSq = dx*dx + dy*dy + dz*dz;

            //     if (distSq < bestDistSq) {
            //         bestDistSq = distSq;
            //         bestFaceIdx = j;
            //     }
            // }

            // face& bestFace = car.col_faces[bestFaceIdx];
            // point& v0 = car.points[bestFace.vertices[0]];
            // point& v1 = car.points[bestFace.vertices[1]];
            // point& v2 = car.points[bestFace.vertices[2]];



            // float p_diffx=p.x-p.oldx;
            // float p_diffy=p.y-p.oldy;
            // float p_diffz=p.z-p.oldz;
            // float p_avgfidd=(p_diffx+p_diffy+p_diffz)/3.0f;

            // float rezerva=1.1f;
            // p.x-=(p.x-p.oldx)*rezerva;
            // p.y-=(p.y-p.oldy)*rezerva;
            // p.z-=(p.z-p.oldz)*rezerva;




            // float avg_x = (v0.x + v1.x + v2.x) / 3.0f;
            // float avg_y = (v0.y + v1.y + v2.y) / 3.0f;
            // float avg_z = (v0.z + v1.z + v2.z) / 3.0f;

            // float old_avg_x = (v0.oldx + v1.oldx + v2.oldx) / 3.0f;
            // float old_avg_y = (v0.oldy + v1.oldy + v2.oldy) / 3.0f;
            // float old_avg_z = (v0.oldz + v1.oldz + v2.oldz) / 3.0f;

            // float diff_x = avg_x - old_avg_x;
            // float diff_y = avg_y - old_avg_y;
            // float diff_z = avg_z - old_avg_z;

            // float avg_diff = (diff_x + diff_y + diff_z) / 3.0f;


            // v0.x = v0.oldx;
            // v0.y = v0.oldy;
            // v0.z = v0.oldz;


            // v1.x = v1.oldx;
            // v1.y = v1.oldy;
            // v1.z = v1.oldz;


            // v2.x = v2.oldx;
            // v2.y = v2.oldy;
            // v2.z = v2.oldz;



            // float mult=0.4f;
            // float xshift=(diff_x-p_diffx)*mult;
            // float yshift=(diff_y-p_diffy)*mult;
            // float zshift=(diff_z-p_diffz)*mult;
            // float damp=0.9f;


            // p.x+=xshift;
            // p.y+=yshift;
            // p.z+=zshift;

            // v0.x += xshift;
            // v0.y += yshift;
            // v0.z += zshift;

            // v1.x += xshift;
            // v1.y += yshift;
            // v1.z += zshift;

            // v2.x += xshift;
            // v2.y += yshift;
            // v2.z += zshift;







    // // Najdi nejbližší trojúhelník (už máš v bestFace a v0,v1,v2)

    // // Normála trojúhelníku (uložená ve face)
    // Vec3 n = {bestFace.nx, bestFace.ny, bestFace.nz};
    // float len = sqrt(n.x*n.x + n.y*n.y + n.z*n.z);
    // if (len > 1e-6) { n.x /= len; n.y /= len; n.z /= len; }
    // else continue; // degenerovaný trojúhelník

    // // Vzdálenost bodu od roviny (kladná = bod je na straně normály)
    // Vec3 v0pos = {v0.x, v0.y, v0.z};
    // float dist = (p.x - v0pos.x)*n.x + (p.y - v0pos.y)*n.y + (p.z - v0pos.z)*n.z;
    // float depth = -dist; // pokud je dist záporné, depth je kladné = hloubka průniku
    // if (depth < 0.0f) depth = 0.0f;
    // if (depth < 0.001f) continue; // moc malé, přeskoč



    // // Inverzní hmotnosti
    // float inv_p = 1.0f / p.mass;
    // float inv_face = (1.0f/v0.mass + 1.0f/v1.mass + 1.0f/v2.mass) / 3.0f;

    // // Celková korekce (lambda)
    // float lambda = depth / (inv_p + inv_face);

    // // Aplikace na bod p (kladný směr = ven)
    // p.x += lambda * inv_p * n.x;
    // p.y += lambda * inv_p * n.y;
    // p.z += lambda * inv_p * n.z;

    // // Aplikace na vrcholy trojúhelníku (záporný směr = dovnitř)
    // // Pro jednoduchost dělíme korekci rovnoměrně mezi vrcholy.
    // // Přesnější by bylo použít barycentrické souřadnice, ale pro začátek to stačí.
    // float face_correction = lambda * inv_face;  // celková korekce pro trojúhelník
    // v0.x -= face_correction * n.x;
    // v0.y -= face_correction * n.y;
    // v0.z -= face_correction * n.z;
    // v1.x -= face_correction * n.x;
    // v1.y -= face_correction * n.y;
    // v1.z -= face_correction * n.z;
    // v2.x -= face_correction * n.x;
    // v2.y -= face_correction * n.y;
    // v2.z -= face_correction * n.z;





            int cast_face=-1;
            Vec3 cor;
            for (int j = 0; j < car.col_faces_count; j++) {
                face& f = car.col_faces[j];
                if (f.group!=facegroup){continue;}
                point& v0 = car.points[f.vertices[0]];
                point& v1 = car.points[f.vertices[1]];
                point& v2 = car.points[f.vertices[2]];
                Vec3 dir={
                    p.x-car.volume_poses[j].x,
                    p.y-car.volume_poses[j].y,
                    p.z-car.volume_poses[j].z
                };
                RaycastResult res=ray_triangle_intersection(p,p1,p2,p3,dir);
                if (res.t>=0.0f && res.hit){
                    cast_face=j;
                    cor.x=dir.x*(res.t/2.0f);
                    cor.y=dir.y*(res.t/2.0f);
                    cor.z=dir.z*(res.t/2.0f);
                    break;
                }
            }
            if (cast_face==-1){continue;}
            face& f = car.col_faces[cast_face];
            point& v0 = car.points[f.vertices[0]];
            point& v1 = car.points[f.vertices[1]];
            point& v2 = car.points[f.vertices[2]];
            p.x+=cor.x;
            p.y+=cor.y;
            p.z+=cor.z;

            v0.x -= cor.x;
            v0.y -= cor.y;
            v0.z -= cor.z;

            v1.x -= cor.x;
            v1.y -= cor.y;
            v1.z -= cor.z;

            v2.x -= cor.x;
            v2.z -= cor.z;
            v2.y -= cor.y;

















            p.vx = (p.x - p.oldx) / dt;
            p.vy = (p.y - p.oldy) / dt;
            p.vz = (p.z - p.oldz) / dt;

            v0.vx = (v0.x - v0.oldx) / dt;
            v0.vy = (v0.y - v0.oldy) / dt;
            v0.vz = (v0.z - v0.oldz) / dt;
            v1.vx = (v1.x - v1.oldx) / dt;
            v1.vy = (v1.y - v1.oldy) / dt;
            v1.vz = (v1.z - v1.oldz) / dt;
            v2.vx = (v2.x - v2.oldx) / dt;
            v2.vy = (v2.y - v2.oldy) / dt;
            v2.vz = (v2.z - v2.oldz) / dt;




        }
    }
    // ballz
    for (int indian = 0; indian < cars.size(); indian++) {
        if (indian == this_car_ind) continue;
        cardata& car = cars[indian];
        for (int bi = 0; bi < car.balls_count; bi++) {
            ball& b = car.balls[bi];
            point& bp = car.points[b.p];

            float dx = p.x - bp.x;
            float dy = p.y - bp.y;
            float dz = p.z - bp.z;
            float dist = sqrtf(dx*dx + dy*dy + dz*dz);

            if (dist < b.radius && dist > 1e-6f) {
                // Push point out of ball
                float penetration = b.radius - dist;
                float nx = dx / dist;
                float ny = dy / dist;
                float nz = dz / dist;

                // Separate positions by mass ratio
                float total_mass = p.mass + bp.mass;
                float ratio_p  = bp.mass / total_mass;
                float ratio_bp = p.mass  / total_mass;

                p.x  += nx * penetration * ratio_p;
                p.y  += ny * penetration * ratio_p;
                p.z  += nz * penetration * ratio_p;
                bp.x -= nx * penetration * ratio_bp;
                bp.y -= ny * penetration * ratio_bp;
                bp.z -= nz * penetration * ratio_bp;

                // Relative velocity along normal
                float rvx = p.vx - bp.vx;
                float rvy = p.vy - bp.vy;
                float rvz = p.vz - bp.vz;
                float relVelN = rvx*nx + rvy*ny + rvz*nz;

                // Only resolve if approaching
                if (relVelN < 0.0f) {
                    float restitution = 0.8f;
                    float impulse = -(1.0f + restitution) * relVelN / total_mass;

                    p.vx  += impulse * bp.mass * nx;
                    p.vy  += impulse * bp.mass * ny;
                    p.vz  += impulse * bp.mass * nz;
                    bp.vx -= impulse * p.mass  * nx;
                    bp.vy -= impulse * p.mass  * ny;
                    bp.vz -= impulse * p.mass  * nz;
                }
            }
        }
    }
}










void calculate_softsoft_collisions(point& p, int this_car_ind, float dt) {
    if (!p.collide){return;}
    Vec3 dirs[] = {
        { 0.57735f,  0.57735f,  0.57735f},
        { 0.57735f, -0.57735f, -0.57735f},
        {-0.57735f,  0.57735f, -0.57735f},
        {-0.57735f, -0.57735f,  0.57735f}
    };
    for (int indian = 0; indian < cars.size(); indian++) {
        if (indian == this_car_ind){continue;}
        cardata& car = cars[indian];
        bool inside=true;
        int facegroup=-1;
        for (int ray=0;ray<4;ray++){
            bool hit=false;
            for (int j=0;j<car.col_faces_count;j++){
                face& f=car.col_faces[j];
                point& p1=car.points[f.vertices[0]];
                point& p2=car.points[f.vertices[1]];
                point& p3=car.points[f.vertices[2]];
                if (raycast(p,p1,p2,p3,dirs[ray]) && (f.group==facegroup || facegroup==-1)){
                    hit=true;
                    facegroup=f.group;
                    break;
                }
            }
            if (!hit){
                inside=false;
                break;
            }
        }
        if (inside){
            log("inside");
            // resolve

            int cast_face=-1;
            Vec3 cor;
            for (int j = 0; j < car.col_faces_count; j++) {
                face& f = car.col_faces[j];
                if (f.group!=facegroup){continue;}
                point& v0 = car.points[f.vertices[0]];
                point& v1 = car.points[f.vertices[1]];
                point& v2 = car.points[f.vertices[2]];

                // Vec3 dir={
                //     p.x-car.volume_poses[j].x,
                //     p.y-car.volume_poses[j].y,
                //     p.z-car.volume_poses[j].z
                // };
                Vec3 dir={
                    car.volume_poses[j].x-p.x,
                    car.volume_poses[j].y-p.y,
                    car.volume_poses[j].z-p.z
                };
                RaycastResult res=ray_triangle_intersection(p,v0,v1,v2,dir);
                if (res.t>=0.0f && res.hit){
                    cast_face=j;
                    cor.x=dir.x*res.t;
                    cor.y=dir.y*res.t;
                    cor.z=dir.z*res.t;
                    break;
                }
            }
            cars_collided_faces[indian].push_back(cast_face);
            if (cast_face==-1){continue;}
            face& f = car.col_faces[cast_face];
            point& v0 = car.points[f.vertices[0]];
            point& v1 = car.points[f.vertices[1]];
            point& v2 = car.points[f.vertices[2]];

            // 1. Spočítáme celkovou hmotnost (bod P + celý trojúhelník)
            float total_mass = p.mass + v0.mass + v1.mass + v2.mass;

            // 2. Kolik z celkové korekce schytá bod P? 
            // Čím lehčí je P, tím víc se musí pohnout (nepřímá úměra)
            float p_mul = (v0.mass + v1.mass + v2.mass) / total_mass;
            // float p_mul = ((v0.mass + v1.mass + v2.mass) / total_mass)*2.0f;

            // 3. Kolik schytá trojúhelník?
            float f_mul = (p.mass / total_mass)/3.0f;

            p.x+=cor.x*p_mul;
            p.y+=cor.y*p_mul;
            p.z+=cor.z*p_mul;

            v0.x -= cor.x*f_mul;
            v0.y -= cor.y*f_mul;
            v0.z -= cor.z*f_mul;

            v1.x -= cor.x*f_mul;
            v1.y -= cor.y*f_mul;
            v1.z -= cor.z*f_mul;

            v2.x -= cor.x*f_mul;
            v2.z -= cor.z*f_mul;
            v2.y -= cor.y*f_mul;






            // p.vx = (p.x - p.oldx) / dt;
            // p.vy = (p.y - p.oldy) / dt;
            // p.vz = (p.z - p.oldz) / dt;

            // v0.vx = (v0.x - v0.oldx) / dt;
            // v0.vy = (v0.y - v0.oldy) / dt;
            // v0.vz = (v0.z - v0.oldz) / dt;
            // v1.vx = (v1.x - v1.oldx) / dt;
            // v1.vy = (v1.y - v1.oldy) / dt;
            // v1.vz = (v1.z - v1.oldz) / dt;
            // v2.vx = (v2.x - v2.oldx) / dt;
            // v2.vy = (v2.y - v2.oldy) / dt;
            // v2.vz = (v2.z - v2.oldz) / dt;



        }
    }
    // ballz
    for (int indian = 0; indian < cars.size(); indian++) {
        if (indian == this_car_ind) continue;
        cardata& car = cars[indian];
        for (int bi = 0; bi < car.balls_count; bi++) {
            ball& b = car.balls[bi];
            point& bp = car.points[b.p];

            float dx = p.x - bp.x;
            float dy = p.y - bp.y;
            float dz = p.z - bp.z;
            float dist = sqrtf(dx*dx + dy*dy + dz*dz);

            if (dist < b.radius && dist > 1e-6f) {
                // Push point out of ball
                float penetration = b.radius - dist;
                float nx = dx / dist;
                float ny = dy / dist;
                float nz = dz / dist;

                // Separate positions by mass ratio
                float total_mass = p.mass + bp.mass;
                float ratio_p  = bp.mass / total_mass;
                float ratio_bp = p.mass  / total_mass;

                p.x  += nx * penetration * ratio_p;
                p.y  += ny * penetration * ratio_p;
                p.z  += nz * penetration * ratio_p;
                bp.x -= nx * penetration * ratio_bp;
                bp.y -= ny * penetration * ratio_bp;
                bp.z -= nz * penetration * ratio_bp;

                // Relative velocity along normal
                float rvx = p.vx - bp.vx;
                float rvy = p.vy - bp.vy;
                float rvz = p.vz - bp.vz;
                float relVelN = rvx*nx + rvy*ny + rvz*nz;

                // Only resolve if approaching
                if (relVelN < 0.0f) {
                    float restitution = 0.8f;
                    float impulse = -(1.0f + restitution) * relVelN / total_mass;

                    p.vx  += impulse * bp.mass * nx;
                    p.vy  += impulse * bp.mass * ny;
                    p.vz  += impulse * bp.mass * nz;
                    bp.vx -= impulse * p.mass  * nx;
                    bp.vy -= impulse * p.mass  * ny;
                    bp.vz -= impulse * p.mass  * nz;
                }
            }
        }
    }
}