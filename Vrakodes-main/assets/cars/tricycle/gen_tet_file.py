"""
Generate tetrahedral mesh from a vehicle JSON file.
Usage: python3 gen_tet_file.py path/to/vehicle.json
The output .tet file will be saved in the same directory with the same base name.
"""

import json
import numpy as np
from scipy.spatial import Delaunay
import struct
import sys
import os

def generate_tet_file(json_path):
    # Output .tet file goes to same folder as input JSON
    base = os.path.splitext(json_path)[0]
    tet_path = base + ".tet"

    # Load JSON
    with open(json_path, 'r') as f:
        data = json.load(f)

    # Extract all physics points (must have "exists": true)
    points = []
    for p in data['points']:
        if p.get('exists', False):
            points.append([p['x'], p['y'], p['z']])

    if len(points) < 4:
        raise ValueError("Need at least 4 points to form a tetrahedral mesh.")

    points = np.array(points, dtype=np.float64)
    print(f"Loaded {len(points)} physics points from {json_path}")

    # Delaunay tetrahedralization (convex hull)
    tri = Delaunay(points)
    tetra = tri.simplices   # shape (n_tetra, 4)

    # Filter out tetrahedra with near‑zero volume (numerical junk)
    def tetra_volume(p, tet):
        a = p[tet[0]]
        b = p[tet[1]]
        c = p[tet[2]]
        d = p[tet[3]]
        return abs(np.dot(b-a, np.cross(c-a, d-a))) / 6.0

    valid_tetra = []
    for tet in tetra:
        vol = tetra_volume(points, tet)
        if vol > 1e-9:
            valid_tetra.append(tet)

    print(f"Generated {len(valid_tetra)} non‑degenerate tetrahedra.")

    # Write binary file: first 4 bytes = number of tetrahedra (int32),
    # then for each tetrahedron: 4 int32 indices.
    with open(tet_path, 'wb') as f:
        f.write(struct.pack('<i', len(valid_tetra)))
        for tet in valid_tetra:
            f.write(struct.pack('<iiii', tet[0], tet[1], tet[2], tet[3]))

    print(f"Tetrahedra saved to {tet_path}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 gen_tet_file.py <path/to/vehicle.json>")
        sys.exit(1)
    try:
        generate_tet_file(sys.argv[1])
    except Exception as e:
        print(f"Error: {e}")
        sys.exit(1)