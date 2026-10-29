import math
import sys
import pygame

# Inicializace Pygame
pygame.init()

# Nastavení okna
WIDTH, HEIGHT = 800, 600
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Interaktivní trojúhelník")

# Barvy (R, G, B)
BG_COLOR = (245, 245, 245)
LINE_COLOR = (70, 130, 180)
VERTEX_COLOR = (220, 50, 50)
GREEN_POINT_COLOR = (30, 180, 50)

# Pozice vrcholů trojúhelníku [x, y]
vertices = [
    [200, 450],
    [400, 150],
    [600, 450]
]

# Libovolný zelený bod
green_point = [380, 320]

# Velikosti bodů (poloměr)
VERTEX_RADIUS = 12
GREEN_POINT_RADIUS = 8

# Stavy pro přetahování myší
dragging_index = None

clock = pygame.time.Clock()
running = True
def sub(v1, v2):
    return [v1[0] - v2[0], v1[1] - v2[1]]

def dot(v1, v2):
    return v1[0] * v2[0] + v1[1] * v2[1]

# --- Algoritmus pro výpočet barycentrických vah ---
mult = 0
weights = [0.0, 0.0, 0.0]
def calculate_def(gregor):
    orig_A = vertices[0]
    orig_B = vertices[1]
    orig_C = vertices[2]

    # Těžiště trojúhelníku
    avg = [
        (orig_A[0] + orig_B[0] + orig_C[0]) / 3.0,
        (orig_A[1] + orig_B[1] + orig_C[1]) / 3.0
    ]
    mul=0
    wewe=[0.0, 0.0, 0.0]
    while mul < 1000:
        mul += 1

        # Zvětšení/škálování vrcholů vůči těžišti podle násobku `mult`
        A = [(orig_A[0] - avg[0]) * mul + avg[0], (orig_A[1] - avg[1]) * mul + avg[1]]
        B = [(orig_B[0] - avg[0]) * mul + avg[0], (orig_B[1] - avg[1]) * mul + avg[1]]
        C = [(orig_C[0] - avg[0]) * mul + avg[0], (orig_C[1] - avg[1]) * mul + avg[1]]

        # Vektory z vrcholů a ze zeleného bodu
        v0 = sub(B, A)
        v1 = sub(C, A)
        v2 = sub(gregor, A)

        # Skalární součiny
        d00 = dot(v0, v0)
        d01 = dot(v0, v1)
        d11 = dot(v1, v1)
        d20 = dot(v2, v0)
        d21 = dot(v2, v1)

        denom = d00 * d11 - d01 * d01

        if denom != 0:
            w1 = (d11 * d20 - d01 * d21) / denom
            w2 = (d00 * d21 - d01 * d20) / denom
            w0 = 1.0 - w1 - w2

            wewe = [w0, w1, w2]

            # Kontrola, zda leží zelený bod uvnitř škálovaného trojúhelníku
            if wewe[0] > 0.0 and wewe[1] > 0.0 and wewe[2] > 0.0:
                break
    return wewe, mul
def update_green():
    # Těžiště z aktuálních vrcholů trojúhelníku
    avg = [
        (vertices[0][0] + vertices[1][0] + vertices[2][0]) / 3.0,
        (vertices[0][1] + vertices[1][1] + vertices[2][1]) / 3.0,
    ]

    # Škálování vrcholů vůči těžišti podle uloženého `mult`
    scaled_A = [
        (vertices[0][0] - avg[0]) * mult + avg[0],
        (vertices[0][1] - avg[1]) * mult + avg[1],
    ]
    scaled_B = [
        (vertices[1][0] - avg[0]) * mult + avg[0],
        (vertices[1][1] - avg[1]) * mult + avg[1],
    ]
    scaled_C = [
        (vertices[2][0] - avg[0]) * mult + avg[0],
        (vertices[2][1] - avg[1]) * mult + avg[1],
    ]

    # Výpočet nové pozice green_point pomocí barycentrických vah
    return [
        scaled_A[0] * weights[0] + scaled_B[0] * weights[1] + scaled_C[0] * weights[2],
        scaled_A[1] * weights[0] + scaled_B[1] * weights[1] + scaled_C[1] * weights[2],
    ]
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

        elif event.type == pygame.MOUSEBUTTONDOWN:
            if event.button == 1:  # Levé tlačítko myši
                mx, my = event.pos
                # Zjištění, zda kliknutí zasáhlo některý z vrcholů
                for i, vertex in enumerate(vertices):
                    dist = math.hypot(vertex[0] - mx, vertex[1] - my)
                    if dist <= VERTEX_RADIUS:
                        dragging_index = i
                        break
            if event.button == 3:  # Pravé tlačítko myši
                green_point = list(event.pos)
                weights, mult = calculate_def(green_point)  # Přepočítání vah po změně pozice zeleného bodu

        elif event.type == pygame.MOUSEBUTTONUP:
            if event.button == 1:
                dragging_index = None

        elif event.type == pygame.MOUSEMOTION:
            if dragging_index is not None:
                # Aktualizace souřadnic taženého vrcholu
                vertices[dragging_index] = list(event.pos)

    # --- SEM MŮŽEŠ PŘIDAT SVOJI MATEMATIKU ---
    # Souřadnice vrcholů máš v seznamu `vertices` (např. vertices[0], vertices[1], vertices[2])
    # Zelený bod máš v `green_point`
    # ----------------------------------------

    # Vykreslení scény
    screen.fill(BG_COLOR)

    # 1. Trojúhelník (čáry)
    pygame.draw.polygon(screen, LINE_COLOR, vertices, width=2)

    # 2. Velké červené body na vrcholech
    for vertex in vertices:
        pygame.draw.circle(screen, VERTEX_COLOR, vertex, VERTEX_RADIUS)
    green_point = update_green()  # Aktualizace pozice zeleného bodu podle vah a škálování
    # 3. Zelený bod
    pygame.draw.circle(screen, GREEN_POINT_COLOR, green_point, GREEN_POINT_RADIUS)

    pygame.display.flip()
    clock.tick(60)

pygame.quit()
sys.exit()