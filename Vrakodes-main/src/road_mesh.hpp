#ifndef INCLUDES
#define INCLUDES
#include <SDL_opengl.h>
#include <SDL_video.h>
#include <stdio.h>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <OpenGL/gl.h>
#include "constants.h"
#include "general_functions.hpp"
#include "types.hpp"
#include "global_vars.hpp"
#endif
struct RoadVertex {
	float x, z;
	float h;       // height
	float weight;  // asphalt weight
};

struct RoadTriangle {
	RoadVertex a, b, c;
};

// Generate road mesh (triangles) from global roadpoints/roadparts and a bridge mask
static std::vector<RoadTriangle> generate_road_mesh(const std::vector<bool>& is_bridge) {
	std::vector<RoadTriangle> tris;
	for (int i = 0; i < roadparts_len; i++) {
		roadpoint& p1 = roadpoints[roadparts[i].p1];
		roadpoint& p2 = roadpoints[roadparts[i].p2];

		float x1 = p1.y; float z1 = p1.x;
		float x2 = p2.y; float z2 = p2.x;
		float h1 = p1.h; float h2 = p2.h;
		float road_w = roadparts[i].width;
		float blend_w = 8.0f;

		float dx = x2 - x1;
		float dz = z2 - z1;
		float len = sqrtf(dx * dx + dz * dz);
		if (len < 0.1f) continue;

		float ux = dx / len; 
		float uz = dz / len;
		float nx = -uz; 
		float nz = ux;

		bool segment_is_high = is_bridge[i];
		if (segment_is_high) continue; // skip bridges here

		// overlap to make segments continuous
		float length_overlap = 0.5f;
		float ox1 = x1 - ux * length_overlap; float oz1 = z1 - uz * length_overlap;
		float ox2 = x2 + ux * length_overlap; float oz2 = z2 + uz * length_overlap;

		RoadVertex p1_left_blend  = { ox1 + nx * (road_w * 0.5f + blend_w), oz1 + nz * (road_w * 0.5f + blend_w), h1, 0.0f };
		RoadVertex p1_left_road   = { ox1 + nx * (road_w * 0.5f),           oz1 + nz * (road_w * 0.5f),           h1, 1.0f };
		RoadVertex p1_right_road  = { ox1 - nx * (road_w * 0.5f),           oz1 - nz * (road_w * 0.5f),           h1, 1.0f };
		RoadVertex p1_right_blend = { ox1 - nx * (road_w * 0.5f + blend_w), oz1 - nz * (road_w * 0.5f + blend_w), h1, 0.0f };

		RoadVertex p2_left_blend  = { ox2 + nx * (road_w * 0.5f + blend_w), oz2 + nz * (road_w * 0.5f + blend_w), h2, 0.0f };
		RoadVertex p2_left_road   = { ox2 + nx * (road_w * 0.5f),           oz2 + nz * (road_w * 0.5f),           h2, 1.0f };
		RoadVertex p2_right_road  = { ox2 - nx * (road_w * 0.5f),           oz2 - nz * (road_w * 0.5f),           h2, 1.0f };
		RoadVertex p2_right_blend = { ox2 - nx * (road_w * 0.5f + blend_w), oz2 - nz * (road_w * 0.5f + blend_w), h2, 0.0f };

		// push the six standard triangles
		tris.push_back({p1_left_road, p2_left_road, p1_right_road});
		tris.push_back({p1_right_road, p2_left_road, p2_right_road});
		tris.push_back({p1_left_blend, p2_left_blend, p1_left_road});
		tris.push_back({p1_left_road, p2_left_blend, p2_left_road});
		tris.push_back({p1_right_road, p2_right_road, p1_right_blend});
		tris.push_back({p1_right_blend, p2_right_road, p2_right_blend});

		int next_idx = -1;
		if (i + 1 < roadparts_len && roadparts[i + 1].p1 == roadparts[i].p2) {
			next_idx = i + 1;
		} else {
			for (int k = 0; k < roadparts_len; k++) {
				if (roadparts[k].p1 == roadparts[i].p2) { next_idx = k; break; }
			}
		}

		if (next_idx != -1) {
			roadpoint& p3 = roadpoints[roadparts[next_idx].p2];
			float x3 = p3.y; float z3 = p3.x;
			float dx2 = x3 - x2; float dz2 = z3 - z2;
			float len2 = sqrtf(dx2 * dx2 + dz2 * dz2);

			if (len2 > 0.1f) {
				float ux2 = dx2 / len2; float uz2 = dz2 / len2;
				float nx2 = -uz2; float nz2 = ux2;

				RoadVertex next_start_left_blend  = { x2 + nx2 * (road_w * 0.5f + blend_w), z2 + nz2 * (road_w * 0.5f + blend_w), h2, 0.0f };
				RoadVertex next_start_left_road   = { x2 + nx2 * (road_w * 0.5f),           z2 + nz2 * (road_w * 0.5f),           h2, 1.0f };
				RoadVertex next_start_right_road  = { x2 - nx2 * (road_w * 0.5f),           z2 - nz2 * (road_w * 0.5f),           h2, 1.0f };
				RoadVertex next_start_right_blend = { x2 - nx2 * (road_w * 0.5f + blend_w), z2 - nz2 * (road_w * 0.5f + blend_w), h2, 0.0f };

				float cross = ux * uz2 - uz * ux2;
				RoadVertex v_center = { x2, z2, h2, 1.0f };

				if (cross > 0.0001f) {
					tris.push_back({v_center, p2_right_road, next_start_right_road});
					tris.push_back({p2_right_road, p2_right_blend, next_start_right_blend});
					tris.push_back({p2_right_road, next_start_right_blend, next_start_right_road});
				} 
				else if (cross < -0.0001f) {
					tris.push_back({v_center, next_start_left_road, p2_left_road});
					tris.push_back({p2_left_road, next_start_left_blend, p2_left_blend});
					tris.push_back({p2_left_road, next_start_left_road, next_start_left_blend});
				}
			}
		}
	}
	return tris;
}