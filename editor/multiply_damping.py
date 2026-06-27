import re
import sys
import os
import math

def physically_correct_damping():
    if len(sys.argv) < 3:
        print("Použití: python3 physically_correct_damping.py <soubor> <nasobic_hmotnosti>")
        return

    file_path = sys.argv[1]
    # To je to číslo (např. 3.0), kterým jsi už vynásobil mass a stiffness
    m_k_multiplier = float(sys.argv[2])

    if not os.path.exists(file_path):
        print("Soubor nenalezen.")
        return

    pattern = rf'"damping"\s*:\s*(\-?\d+\.?\d*)'

    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()

        def replace_func(match):
            old_damping = float(match.group(1))
            
            # FYZIKÁLNÍ LOGIKA:
            # Pokud m a k vzrostly n-krát, kritické tlumení vzroste odmocninou z (n*n).
            # V praxi: pokud jsi vše zvedl 3x, musíš i relativní damping 
            # upravit tak, aby odpovídal nové energii systému.
            
            # Pokud chceš, aby se auto chovalo identicky:
            new_val = old_damping * math.sqrt(m_k_multiplier)
            
            # Pokud by to přelezlo 1.0, zastavíme to na 0.99 (aby to neházelo chybu)
            new_val = max(0.0, min(0.999999, new_val))
            return f'"damping":{new_val:.6f}'

        new_content = re.sub(pattern, replace_func, content)

        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(new_content)
        
        print(f"Damping v '{file_path}' upraven pro násobič hmotnosti {m_k_multiplier}x.")

    except Exception as e:
        print(f"Chyba: {e}")

if __name__ == "__main__":
    physically_correct_damping()