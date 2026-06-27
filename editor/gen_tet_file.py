"""
Generate tetrahedral mesh from a vehicle JSON file, grouped by 'group' ID.
Usage: python3 gen_tet_file.py path/to/vehicle.json
"""

import json
import numpy as np
from scipy.spatial import Delaunay
import struct
import sys
import os

def generate_tet_file(json_path):
    base = os.path.splitext(json_path)[0]
    tet_path = base + ".tet"

    with open(json_path, 'r') as f:
        data = json.load(f)

    # 1. Načteme body a jejich původní indexy, abychom je mohli seskupit
    # Ukládáme si: (původní_index, [x, y, z], group_id)
    all_points_data = []
    for i, p in enumerate(data['points']):
        all_points_data.append({
            'id': i, 
            'coords': [p['x'], p['y'], p['z']], 
            'group': p.get('group', 0)
        })

    if len(all_points_data) < 4:
        raise ValueError("Potřeba alespoň 4 body celkem.")

    # 2. Rozdělíme body do skupin podle 'group'
    groups = {}
    for p in all_points_data:
        g_id = p['group']
        if g_id not in groups:
            groups[g_id] = []
        groups[g_id].append(p)

    valid_tetra = []

    # 3. Pro každou skupinu spustíme Delaunaye zvlášť
    for g_id, g_points in groups.items():
        if len(g_points) < 4:
            print(f"Skupina {g_id} má málo bodů ({len(g_points)}), přeskakuji...")
            continue
        
        # Připravíme pole souřadnic a mapu indexů pro tuhle skupinu
        coords = np.array([p['coords'] for p in g_points], dtype=np.float64)
        local_to_global = [p['id'] for p in g_points]

        try:
            tri = Delaunay(coords)
            
            # Funkce pro objem (pro filtraci smetí)
            def get_vol(p, t):
                a, b, c, d = p[t[0]], p[t[1]], p[t[2]], p[t[3]]
                return abs(np.dot(b-a, np.cross(c-a, d-a))) / 6.0

            for t in tri.simplices:
                if get_vol(coords, t) > 1e-6:
                    # Převedeme lokální indexy Delaunaye zpět na globální indexy z JSONu
                    global_tet = (
                        local_to_global[t[0]],
                        local_to_global[t[1]],
                        local_to_global[t[2]],
                        local_to_global[t[3]]
                    )
                    valid_tetra.append(global_tet)
        except Exception as e:
            print(f"Chyba při generování skupiny {g_id}: {e}")

    print(f"Celkem vygenerováno {len(valid_tetra)} tetrahedronů napříč {len(groups)} skupinami.")

    # 4. Binární zápis
    with open(tet_path, 'wb') as f:
        f.write(struct.pack('<i', len(valid_tetra)))
        for tet in valid_tetra:
            f.write(struct.pack('<iiii', *tet))

    print(f"Hotovo! Uloženo do: {tet_path}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 gen_tet_file.py <path/to/vehicle.json>")
        sys.exit(1)
    generate_tet_file(sys.argv[1])